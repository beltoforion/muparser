//---------------------------------------------------------------------------
value_type Mega(value_type a_fVal) 
{ 
  return a_fVal * 1e6; 
}

//---------------------------------------------------------------------------
value_type Not(value_type v) 
{ 
  return v==0; 
}

//---------------------------------------------------------------------------
value_type Ping() 
{ 
  mu::console() << "ping\n"; 
  return 0; 
}

//---------------------------------------------------------------------------
void Calc()
{
  mu::Parser  parser;

  parser.SetArgSep(';');
  parser.SetDecSep(',');
  parser.SetThousandsSep('.');

  // Add some variables
  value_type  a = 10;
  parser.DefineVar("a", &a);
  parser.DefineStrConst("strBuf", "hello world");

  // Add user defined unary operators
  parser.DefinePostfixOprt("M", Mega);
  parser.DefineInfixOprt("!", Not);
  parser.DefineFun("ping", Ping, false);

  try
  {
    string_type sLine;
    std::getline(mu::console_in(), sLine);

    parser.SetExpr(sLine);
    mu::console() << std::setprecision(12);
    mu::console() << parser.Eval() << "\n";
  }
  catch(mu::Parser::exception_type &e)
  {
    mu::console() << "\nError:\n";
    mu::console() << "------\n";
    mu::console() << "Message:     "   << e.GetMsg()      << "\n";
    mu::console() << "Expression:  \"" << e.GetExpr()     << "\"\n";
    mu::console() << "Token:       \"" << e.GetToken()    << "\"\n";
    mu::console() << "Position:    "   << (int)e.GetPos() << "\n";
    mu::console() << "Errc:        "   << std::dec << e.GetCode() << "\n";
  }
}

