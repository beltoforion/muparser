// AsmJit - Complete JIT Assembler for C++ Language.

// Copyright (c) 2008-2009, Petr Kobalicek <kobalicek.petr@gmail.com>
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

// [Dependencies]
#include "Build.h"
#include "Lock.h"
#include "MemoryManager.h"
#include "VirtualMemory.h"

#include <stdio.h>
#include <string.h>

// [Warnings-Push]
#include "WarningsPush.h"

// This file contains implementation of virtual memory management for AsmJit
// library. The initial concept is to keep this implementation simple but 
// effective. There are several goals I decided to write implementation myself.
//
// Goals:
// - We need usually to allocate blocks of 64 bytes long and more.
// - Alignment of allocated blocks is large - 32 bytes or 64 bytes.
// - Keep memory manager informations outside allocated virtual memory pages
//   (these pages allows execution of code).
// - Keep implementation small.
//
// I think that implementation is not small and probably not too much readable,
// so there is small know how.
//
// - Implementation is based on bit arrays and binary trees. Bit arrays 
//   contains informations about allocated and unused blocks of memory. Each
//   block size describes M_Node::density member. Count of blocks are
//   stored in M_Node::blocks member. For example if density is 64 and 
//   count of blocks is 20, memory node contains 64*20 bytes of memory and
//   smallest possible allocation (and also alignment) is 64 bytes. So density
//   describes also memory alignment. Binary trees are used to enable fast
//   lookup into all addresses allocated by memory manager instance. This is
//   used mainly in MemoryManagerPrivate::free().
//
//   Bit array looks like this (empty = unused, X = used) - Size of block 64
//   -------------------------------------------------------------------------
//   | |X|X| | | | | |X|X|X|X|X|X| | | | | | | | | | | | |X| | | | |X|X|X| | |
//   -------------------------------------------------------------------------
//   Bits array shows that there are 12 allocated blocks of 64 bytes, so total 
//   allocated size is 768 bytes. Maximum count of continuous blocks is 12
//   (see largest gap).

namespace AsmJit {

// ============================================================================
// [Bits Manipulation]
// ============================================================================

#define BITS_PER_ENTITY (sizeof(SysUInt) * 8)

static void _SetBit(SysUInt* buf, SysUInt index) ASMJIT_NOTHROW
{
  SysUInt i = index / BITS_PER_ENTITY; // SysUInt[]
  SysUInt j = index % BITS_PER_ENTITY; // SysUInt[][] bit index

  buf += i;
  *buf |= (SysUInt)1 << j;
}

static void _ClearBit(SysUInt* buf, SysUInt index) ASMJIT_NOTHROW
{
  SysUInt i = index / BITS_PER_ENTITY; // SysUInt[]
  SysUInt j = index % BITS_PER_ENTITY; // SysUInt[][] bit index

  buf += i;
  *buf &= ~((SysUInt)1 << j);
}

static void _SetBits(SysUInt* buf, SysUInt index, SysUInt len) ASMJIT_NOTHROW
{
  if (len == 0) return;

  SysUInt i = index / BITS_PER_ENTITY; // SysUInt[]
  SysUInt j = index % BITS_PER_ENTITY; // SysUInt[][] bit index

  // how many bytes process in first group
  SysUInt c = BITS_PER_ENTITY - j;

  // offset
  buf += i;

  if (c > len) 
  {
    *buf |= (((SysUInt)-1) >> (BITS_PER_ENTITY - len)) << j;
    return;
  }
  else
  {
    *buf++ |= (((SysUInt)-1) >> (BITS_PER_ENTITY - c)) << j;
    len -= c;
  }

  while (len >= BITS_PER_ENTITY)
  {
    *buf++ = (SysUInt)-1;
    len -= BITS_PER_ENTITY;
  }

  if (len)
  {
    *buf |= (((SysUInt)-1) >> (BITS_PER_ENTITY - len));
  }
}

static void _ClearBits(SysUInt* buf, SysUInt index, SysUInt len) ASMJIT_NOTHROW
{
  if (len == 0) return;

  SysUInt i = index / BITS_PER_ENTITY; // SysUInt[]
  SysUInt j = index % BITS_PER_ENTITY; // SysUInt[][] bit index

  // how many bytes process in first group
  SysUInt c = BITS_PER_ENTITY - j;

  // offset
  buf += i;

  if (c > len)
  {
    *buf &= ~((((SysUInt)-1) >> (BITS_PER_ENTITY - len)) << j);
    return;
  }
  else
  {
    *buf++ &= ~((((SysUInt)-1) >> (BITS_PER_ENTITY - c)) << j);
    len -= c;
  }

  while (len >= BITS_PER_ENTITY)
  {
    *buf++ = 0;
    len -= BITS_PER_ENTITY;
  }

  if (len)
  {
    *buf &= ((SysUInt)-1) << len;
  }
}

// ============================================================================
// [AsmJit::M_Node]
// ============================================================================

#define M_DIV(x, y) ((x) / (y))
#define M_MOD(x, y) ((x) % (y))

struct ASMJIT_HIDDEN M_Node
{
  // Node double-linked list
  M_Node* prev;          // Prev node in list
  M_Node* next;          // Next node in list

  // Node (LLRB tree, KEY is mem)
  M_Node* nlLeft;        // Left node
  M_Node* nlRight;       // Right node
  UInt32 nlColor;        // Color (RED or BLACK)

  // Chunk memory
  UInt8* mem;            // Virtual memory address

  // Chunk data
  SysUInt size;          // How many bytes contains this node
  SysUInt blocks;        // How many blocks are here.
  SysUInt density;       // Minimum count of allocated bytes in this node (also alignment).
  SysUInt used;          // How many bytes are used in this node
  SysUInt largestBlock;  // Contains largest block that can be allocated
  SysUInt* baUsed;       // Contains bits about used blocks
                         // (0 = unused, 1 = used)
  SysUInt* baCont;       // Contains bits about continuous blocks
                         // (0 = stop, 1 = continue)

  // enums
  enum NODE_COLOR
  {
    NODE_BLACK = 0,
    NODE_RED = 1
  };

  // methods
  inline SysUInt remain() const ASMJIT_NOTHROW { return size - used; }
};

// ============================================================================
// [AsmJit::M_Pernament]
// ============================================================================

//! @brief Pernament node.
struct ASMJIT_HIDDEN M_PernamentNode
{
  UInt8* mem;            // Base pointer (virtual memory address).
  SysUInt size;          // Count of bytes allocated.
  SysUInt used;          // Count of bytes used.
  M_PernamentNode* prev; // Pointer to prev chunk or NULL

  // Return available space.
  inline SysUInt available() const ASMJIT_NOTHROW { return size - used; }
};

// ============================================================================
// [AsmJit::MemoryManagerPrivate]
// ============================================================================

struct ASMJIT_HIDDEN MemoryManagerPrivate
{
  // [Construction / Destruction]

  MemoryManagerPrivate() ASMJIT_NOTHROW;
  ~MemoryManagerPrivate() ASMJIT_NOTHROW;

  // [Allocation]

  static M_Node* createNode(SysUInt size, SysUInt density) ASMJIT_NOTHROW;

  void* allocPernament(SysUInt vsize) ASMJIT_NOTHROW;
  void* allocFreeable(SysUInt vsize) ASMJIT_NOTHROW;
  bool free(void* address) ASMJIT_NOTHROW;

  // [NodeList LLRB-Tree]

  static inline bool nlIsRed(M_Node* n) ASMJIT_NOTHROW;
  static M_Node* nlRotateLeft(M_Node* n) ASMJIT_NOTHROW;
  static M_Node* nlRotateRight(M_Node* n) ASMJIT_NOTHROW;
  static inline void nlFlipColor(M_Node* n) ASMJIT_NOTHROW;
  static M_Node* nlMoveRedLeft(M_Node* h) ASMJIT_NOTHROW;
  static M_Node* nlMoveRedRight(M_Node* h) ASMJIT_NOTHROW;
  static inline M_Node* nlFixUp(M_Node* h) ASMJIT_NOTHROW;

  inline void nlInsertNode(M_Node* n) ASMJIT_NOTHROW;
  M_Node* nlInsertNode_(M_Node* h, M_Node* n) ASMJIT_NOTHROW;

  inline void nlRemoveNode(M_Node* n) ASMJIT_NOTHROW;
  M_Node* nlRemoveNode_(M_Node* h, M_Node* n) ASMJIT_NOTHROW;
  M_Node* nlRemoveMin(M_Node* h) ASMJIT_NOTHROW;

  M_Node* nlFindPtr(UInt8* mem) ASMJIT_NOTHROW;

  // [Members]

  Lock _lock;                // Lock for thread safety

  SysUInt _newChunkSize;     // Default node size
  SysUInt _newChunkDensity;  // Default node density
  SysUInt _allocated;        // How many bytes are allocated
  SysUInt _used;             // How many bytes are used

  // Memory nodes list
  M_Node* _first;
  M_Node* _last;
  M_Node* _optimal;

  // Memory nodes tree
  M_Node* _root;

  // Pernament memory
  M_PernamentNode* _pernament;
};

// ============================================================================
// [AsmJit::MemoryManagerPrivate - Construction / Destruction]
// ============================================================================

MemoryManagerPrivate::MemoryManagerPrivate() ASMJIT_NOTHROW :
  _newChunkSize(65536),
  _newChunkDensity(64),
  _allocated(0),
  _used(0),
  _root(NULL),
  _first(NULL),
  _last(NULL),
  _optimal(NULL),
  _pernament(NULL)
{
}

MemoryManagerPrivate::~MemoryManagerPrivate() ASMJIT_NOTHROW
{
}

// ============================================================================
// [AsmJit::MemoryManagerPrivate - Allocation]
// ============================================================================

// allocates virtual memory node and M_Node structure.
//
// returns M_Node* if success, otherwise NULL
M_Node* MemoryManagerPrivate::createNode(SysUInt size, SysUInt density) ASMJIT_NOTHROW
{
  SysUInt vsize;
  UInt8* vmem = (UInt8*)VirtualMemory::alloc(size, &vsize, true);

  // Out of memory
  if (vmem == NULL) return NULL;

  SysUInt blocks = (vsize / density);
  SysUInt basize = (((blocks + 7) >> 3) + sizeof(SysUInt) - 1) & ~(SysUInt)(sizeof(SysUInt)-1);
  SysUInt memSize = sizeof(M_Node) + (basize * 2);

  M_Node* node = (M_Node*)ASMJIT_MALLOC(memSize);

  // Out of memory
  if (node == NULL)
  {
    VirtualMemory::free(vmem, vsize);
    return NULL;
  }

  memset(node, 0, memSize);

  node->nlColor = M_Node::NODE_RED;
  node->mem = vmem;

  node->size = vsize;
  node->blocks = blocks;
  node->density = density;
  node->largestBlock = vsize;
  node->baUsed = (SysUInt*)( (UInt8*)node + sizeof(M_Node) );
  node->baCont = (SysUInt*)( (UInt8*)node->baUsed + basize );

  return node;
}

void* MemoryManagerPrivate::allocPernament(SysUInt vsize) ASMJIT_NOTHROW
{
  static const SysUInt pernamentAlignment = 32;
  static const SysUInt pernamentNodeSize  = 32768;

  SysUInt over = vsize % pernamentAlignment;
  if (over) over = pernamentAlignment - over;
  SysUInt alignedSize = vsize + over;

  AutoLock locked(_lock);

  M_PernamentNode* node = _pernament;

  // Try to find space in allocated chunks
  while (node && alignedSize > node->available()) node = node->prev;

  // Or allocate new node
  if (!node)
  {
    SysUInt nodeSize = pernamentNodeSize;
    if (vsize > nodeSize) nodeSize = vsize;

    node = (M_PernamentNode*)ASMJIT_MALLOC(sizeof(M_PernamentNode));
    // Out of memory
    if (node == NULL) return NULL;

    node->mem = (UInt8*)VirtualMemory::alloc(nodeSize, &node->size, true);
    // Out of memory
    if (node->mem == NULL) 
    {
      ASMJIT_FREE(node);
      return NULL;
    }

    node->used = 0;
    node->prev = _pernament;
    _pernament = node;
  }

  // Finally, copy function code to our space we reserved for.
  UInt8* result = node->mem + node->used;

  // Update Statistics
  node->used += alignedSize;
  _used += alignedSize;

  // Code can be null to only reserve space for code.
  return (void*)result;
}

void* MemoryManagerPrivate::allocFreeable(SysUInt vsize) ASMJIT_NOTHROW
{
  SysUInt i;               // current index
  SysUInt need;            // how many we need to be freed
  SysUInt minVSize;

  // align to 32 bytes (our default alignment)
  vsize = (vsize + 31) & ~(SysUInt)31;
  if (vsize == 0) return NULL;

  AutoLock locked(_lock);
  M_Node* node = _optimal;

  minVSize = _newChunkSize;

  // try to find memory block in existing nodes
  while (node)
  {
    // Skip this node ?
    if ((node->remain() < vsize) || 
        (node->largestBlock < vsize && node->largestBlock != 0))
    {
      M_Node* next = node->next;
      if (node->remain() < minVSize && node == _optimal && next) _optimal = next;
      node = next;
      continue;
    }

    SysUInt* up = node->baUsed;    // current ubits address
    SysUInt ubits;                 // current ubits[0] value
    SysUInt bit;                   // current bit mask
    SysUInt blocks = node->blocks; // count of blocks in node
    SysUInt cont = 0;              // how many bits are currently freed in find loop
    SysUInt maxCont = 0;           // largest continuous block (bits count)
    SysUInt j;

    need = M_DIV((vsize + node->density - 1), node->density);
    i = 0;

    // try to find node that is large enough
    while (i < blocks)
    {
      ubits = *up++;

      // Fast skip used blocks
      if (ubits == (SysUInt)-1) 
      { 
        if (cont > maxCont) maxCont = cont;
        cont = 0;

        i += BITS_PER_ENTITY;
        continue;
      }

      SysUInt max = BITS_PER_ENTITY;
      if (i + max > blocks) max = blocks - i;

      for (j = 0, bit = 1; j < max; bit <<= 1)
      {
        j++;
        if ((ubits & bit) == 0)
        {
          if (++cont == need) { i += j; i -= cont; goto found; }
          continue;
        }

        if (cont > maxCont) maxCont = cont;
        cont = 0;
      }

      i += BITS_PER_ENTITY;
    }

    // because we traversed entire node, we can set largest node size that 
    // will be used to cache next traversing.
    node->largestBlock = maxCont * node->density;

    node = node->next;
  }

  // if we are here, we failed to find existing memory block and we must 
  // allocate new.
  {
    SysUInt chunkSize = _newChunkSize;
    if (chunkSize < vsize) chunkSize = vsize;

    node = createNode(chunkSize, _newChunkDensity);
    if (node == NULL) return NULL;

    // link with others
    node->prev = _last;

    if (_first == NULL)
    {
      _first = node;
      _last = node;
      _optimal = node;
    }
    else
    {
      node->prev = _last;
      _last->next = node;
      _last = node;
    }

    // Update binary tree
    nlInsertNode(node);

    // Alloc first node at start
    i = 0;
    need = (vsize + node->density - 1) / node->density;

    // Update statistics
    _allocated += node->size;
  }

found:
  // Update bits
  _SetBits(node->baUsed, i, need);
  _SetBits(node->baCont, i, need-1);

  // Update statistics
  {
    SysUInt u = need * node->density;
    node->used += u;
    node->largestBlock = 0;
    _used += u;
  }

  // and return pointer
  UInt8* result = node->mem + i * node->density;
  ASMJIT_ASSERT(result >= node->mem && result < node->mem + node->size);
  return result;
}

bool MemoryManagerPrivate::free(void* address) ASMJIT_NOTHROW
{
  if (address == NULL) return true;

  AutoLock locked(_lock);

  M_Node* node = nlFindPtr((UInt8*)address);
  if (node == NULL) return false;

  SysUInt offset = (SysUInt)((UInt8*)address - (UInt8*)node->mem);
  SysUInt bitpos = M_DIV(offset, node->density);
  SysUInt i = (bitpos / BITS_PER_ENTITY);
  SysUInt j = (bitpos % BITS_PER_ENTITY);

  SysUInt* up = node->baUsed + i;// current ubits address
  SysUInt* cp = node->baCont + i;// current cbits address
  SysUInt ubits = *up;           // current ubits[0] value
  SysUInt cbits = *cp;           // current cbits[0] value
  SysUInt bit = (SysUInt)1 << j; // current bit mask

  SysUInt cont = 0;

  bool stop;

  for (;;)
  {
    stop = (cbits & bit) == 0;
    ubits &= ~bit;
    cbits &= ~bit;

    j++;
    bit <<= 1;
    cont++;

    if (stop || j == BITS_PER_ENTITY)
    {
      *up = ubits;
      *cp = cbits;

      if (stop) break;

      ubits = *++up;
      cbits = *++cp;

      j = 0;
      bit = 1;
    }
  }

  // if we freed block is fully allocated node, need to update optimal
  // pointer in memory manager.
  if (node->used == node->size)
  {
    M_Node* cur = _optimal;

    do {
      cur = cur->prev;
      if (cur == node) { _optimal = node; break; }
    } while (cur);
  }

  // statistics
  cont *= node->density;
  if (node->largestBlock < cont) node->largestBlock = cont;
  node->used -= cont;
  _used -= cont;

  // if page is empty, we can free it
  if (node->used == 0)
  {
    _allocated -= node->size;
    nlRemoveNode(node);
    VirtualMemory::free(node->mem, node->size);

    M_Node* next = node->next;
    M_Node* prev = node->prev;

    if (prev) { prev->next = next; } else { _first = next; }
    if (next) { next->prev = prev; } else { _last  = prev; }
    if (_optimal == node) { _optimal = prev ? prev : next; }

    ASMJIT_FREE(node);
  }

  return true;
}

// ============================================================================
// [AsmJit::MemoryManagerPrivate - NodeList LLRB-Tree]
// ============================================================================

inline bool MemoryManagerPrivate::nlIsRed(M_Node* n) ASMJIT_NOTHROW
{
  return n && n->nlColor == M_Node::NODE_RED;
}

inline M_Node* MemoryManagerPrivate::nlRotateLeft(M_Node* n) ASMJIT_NOTHROW
{
  M_Node* x = n->nlRight;
  n->nlRight = x->nlLeft;
  x->nlLeft = n;
  x->nlColor = x->nlLeft->nlColor;
  x->nlLeft->nlColor = M_Node::NODE_RED;
  return x;
}

inline M_Node* MemoryManagerPrivate::nlRotateRight(M_Node* n) ASMJIT_NOTHROW
{
  M_Node* x = n->nlLeft;
  n->nlLeft = x->nlRight;
  x->nlRight = n;
  x->nlColor = x->nlRight->nlColor;
  x->nlRight->nlColor = M_Node::NODE_RED;
  return x;
}

inline void MemoryManagerPrivate::nlFlipColor(M_Node* n) ASMJIT_NOTHROW
{
  n->nlColor = !n->nlColor;
  n->nlLeft->nlColor = !(n->nlLeft->nlColor);
  n->nlRight->nlColor = !(n->nlRight->nlColor);
}

M_Node* MemoryManagerPrivate::nlMoveRedLeft(M_Node* h) ASMJIT_NOTHROW
{
  nlFlipColor(h);
  if (nlIsRed(h->nlRight->nlLeft))
  {
    h->nlRight = nlRotateRight(h->nlRight);
    h = nlRotateLeft(h);
    nlFlipColor(h);
  }
  return h;
}

M_Node* MemoryManagerPrivate::nlMoveRedRight(M_Node* h) ASMJIT_NOTHROW
{
  nlFlipColor(h);
  if (nlIsRed(h->nlLeft->nlLeft))
  {
    h = nlRotateRight(h);
    nlFlipColor(h);
  }
  return h;
}

inline M_Node* MemoryManagerPrivate::nlFixUp(M_Node* h) ASMJIT_NOTHROW
{
  if (nlIsRed(h->nlRight))
    h = nlRotateLeft(h);
  if (nlIsRed(h->nlLeft) && nlIsRed(h->nlLeft->nlLeft))
    h = nlRotateRight(h);
  if (nlIsRed(h->nlLeft) && nlIsRed(h->nlRight))
    nlFlipColor(h);

  return h;
}

inline void MemoryManagerPrivate::nlInsertNode(M_Node* n) ASMJIT_NOTHROW
{
  _root = nlInsertNode_(_root, n);
}

M_Node* MemoryManagerPrivate::nlInsertNode_(M_Node* h, M_Node* n) ASMJIT_NOTHROW
{
  if (h == NULL) return n;

  if (nlIsRed(h->nlLeft) && nlIsRed(h->nlRight)) nlFlipColor(h);

  if (n->mem < h->mem)
    h->nlLeft = nlInsertNode_(h->nlLeft, n);
  else
    h->nlRight = nlInsertNode_(h->nlRight, n);

  if (nlIsRed(h->nlRight) && !nlIsRed(h->nlLeft)) h = nlRotateLeft(h);
  if (nlIsRed(h->nlLeft) && nlIsRed(h->nlLeft->nlLeft)) h = nlRotateRight(h);

  return h;
}

void MemoryManagerPrivate::nlRemoveNode(M_Node* n) ASMJIT_NOTHROW
{
  _root = nlRemoveNode_(_root, n);
  if (_root) _root->nlColor = M_Node::NODE_BLACK;

  ASMJIT_ASSERT(nlFindPtr(n->mem) == NULL);
}

static M_Node* findParent(M_Node* root, M_Node* n) ASMJIT_NOTHROW
{
  M_Node* parent = NULL;
  M_Node* cur = root;
  UInt8* mem = n->mem;
  UInt8* curMem;
  UInt8* curEnd;

  while (cur)
  {
    curMem = cur->mem;
    if (mem < curMem)
    {
      parent = cur;
      cur = cur->nlLeft;
      continue;
    }
    else
    {
      curEnd = curMem + cur->size;
      if (mem >= curEnd)
      {
        parent = cur;
        cur = cur->nlRight;
        continue;
      }
      return parent;
    }
  }

  return NULL;
}

M_Node* MemoryManagerPrivate::nlRemoveNode_(M_Node* h, M_Node* n) ASMJIT_NOTHROW
{
  if (n->mem < h->mem)
  {
    if (!nlIsRed(h->nlLeft) && !nlIsRed(h->nlLeft->nlLeft))
      h = nlMoveRedLeft(h);
    h->nlLeft = nlRemoveNode_(h->nlLeft, n);
  }
  else
  {
    if (nlIsRed(h->nlLeft))
      h = nlRotateRight(h);
    if (h == n && (h->nlRight == NULL))
      return NULL;
    if (!nlIsRed(h->nlRight) && !nlIsRed(h->nlRight->nlLeft))
      h = nlMoveRedRight(h);
    if (h == n)
    {
      // Get minimum node
      h = n->nlRight;
      while (h->nlLeft) h = h->nlLeft;

      M_Node* _l = n->nlLeft;
      M_Node* _r = nlRemoveMin(n->nlRight);

      h->nlLeft = _l;
      h->nlRight = _r;
      h->nlColor = n->nlColor;
    }
    else
      h->nlRight = nlRemoveNode_(h->nlRight, n);
  }

  return nlFixUp(h);
}

M_Node* MemoryManagerPrivate::nlRemoveMin(M_Node* h) ASMJIT_NOTHROW
{
  if (h->nlLeft == NULL) return NULL;
  if (!nlIsRed(h->nlLeft) && !nlIsRed(h->nlLeft->nlLeft))
    h = nlMoveRedLeft(h);
  h->nlLeft = nlRemoveMin(h->nlLeft);
  return nlFixUp(h);
}

M_Node* MemoryManagerPrivate::nlFindPtr(UInt8* mem) ASMJIT_NOTHROW
{
  M_Node* cur = _root;
  UInt8* curMem;
  UInt8* curEnd;

  while (cur)
  {
    curMem = cur->mem;
    if (mem < curMem)
    {
      cur = cur->nlLeft;
      continue;
    }
    else
    {
      curEnd = curMem + cur->size;
      if (mem >= curEnd)
      {
        cur = cur->nlRight;
        continue;
      }
      return cur;
    }
  }

  return NULL;
}

// ============================================================================
// [AsmJit::MemoryManager]
// ============================================================================

MemoryManager::MemoryManager() ASMJIT_NOTHROW
{
}

MemoryManager::~MemoryManager() ASMJIT_NOTHROW
{
}

MemoryManager* MemoryManager::global() ASMJIT_NOTHROW
{
  static DefaultMemoryManager memmgr;
  return &memmgr;
}

// ============================================================================
// [AsmJit::DefaultMemoryManager]
// ============================================================================

DefaultMemoryManager::DefaultMemoryManager() ASMJIT_NOTHROW
{
  MemoryManagerPrivate* d = new MemoryManagerPrivate();
  _d = (void*)d;
}

DefaultMemoryManager::~DefaultMemoryManager() ASMJIT_NOTHROW
{
  MemoryManagerPrivate* d = reinterpret_cast<MemoryManagerPrivate*>(_d);
  delete d;
}

void* DefaultMemoryManager::alloc(SysUInt size, UInt32 type) ASMJIT_NOTHROW
{
  MemoryManagerPrivate* d = reinterpret_cast<MemoryManagerPrivate*>(_d);

  if (type == MEMORY_ALLOC_PERNAMENT) 
    return d->allocPernament(size);
  else
    return d->allocFreeable(size);
}

bool DefaultMemoryManager::free(void* address) ASMJIT_NOTHROW
{
  MemoryManagerPrivate* d = reinterpret_cast<MemoryManagerPrivate*>(_d);
  return d->free(address);
}

SysUInt DefaultMemoryManager::used() ASMJIT_NOTHROW
{
  MemoryManagerPrivate* d = reinterpret_cast<MemoryManagerPrivate*>(_d);
  return d->_used;
}

SysUInt DefaultMemoryManager::allocated() ASMJIT_NOTHROW
{
  MemoryManagerPrivate* d = reinterpret_cast<MemoryManagerPrivate*>(_d);
  return d->_allocated;
}

} // AsmJit namespace

// [Warnings-Pop]
#include "WarningsPop.h"
