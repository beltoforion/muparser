using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using muParser;
using System.Threading;


namespace muWrapper
{
    public partial class WndSample : Form
    {
      enum EColorPlane : int
      {
        red = 0,
        green = 1,
        blue = 2
      }

      private muParser.Parser m_parser;
      private muParser.Parser[] m_parserImg = new muParser.Parser[3];
//      private Parser.CompiledFunDelegate[] m_funCol = new Parser.CompiledFunDelegate[3];

      private List<String> m_history;
      private int m_histLine;

      private byte[] m_data;

      private ParserVariable m_val1 = new ParserVariable(0);
      private ParserVariable m_val2 = new ParserVariable(0);
      private ParserVariable m_ans = new ParserVariable(0);

      public WndSample()
      {
        InitializeComponent();
        CreateInitialImage();
        PopulatePresets();

        cbDec.SelectedIndex = 0;
        cbArg.SelectedIndex = 1;
        m_data = new byte[pbImage.Width * pbImage.Height * 3];
        
        try
        {
          m_parser = new Parser();
          lbVersion.Text = String.Format("muParserSSE V{0}", m_parser.GetVersion());
          m_parser.DefineFun("fun1", new Parser.Fun1Delegate(fun1), true);
          m_parser.DefineFun("fun3", new Parser.Fun3Delegate(fun3), true);
          m_parser.DefineFun("fun4", new Parser.Fun4Delegate(fun4), true);
          m_parser.DefineFun("fun5", new Parser.Fun5Delegate(fun5), true);

          //m_parser.DefinePostfixOprt("m", new muParser.Parser.Fun1Delegate(milli));
          m_parser.DefineInfixOprt("!", new muParser.Parser.Fun1Delegate(not), muParser.Parser.EPrec.prLOGIC);

          m_parser.DefineVar("ans", m_ans);
          m_parser.DefineVar("my_var1", m_val1);
          m_parser.DefineVar("my_var2", m_val2);

          // Initialize parsers used for the image calculation
          for (int i = 0; i < 3; ++i)
          {
            m_parserImg[i] = new Parser();
          }
        }
        catch (ParserException exc)
        {
          DumpException(exc);
        }
      
        m_history = new List<String>();
      }

      private void CreateInitialImage()
      {
		    Image img = new Bitmap(pbImage.Width, pbImage.Height, System.Drawing.Imaging.PixelFormat.Format24bppRgb);
		    Graphics g = Graphics.FromImage(img);

		    Rectangle imgRect = new Rectangle(0, 0, pbImage.Width, pbImage.Height);
		    g.FillRectangle(Brushes.White, imgRect);
		    g.DrawString("Select a preset or enter expressions for the red, green, and blue components.", 
                     new Font("Tahoma", 10, FontStyle.Bold), 
                     Brushes.Orange, 
                     imgRect);
		    g.Dispose();
		    pbImage.Image = img;
      }
    
      private void PopulatePresets()
      {
		    PresetInfo info;
		    ComboBox.ObjectCollection items = cbPresets.Items;

		    info = new PresetInfo("Blinds", "(rint(4*x-y*2) % 2) - x", "(abs(x+2*y) % 0.75)*10+y/5", "rint(sin(sqrt(x*x+y*y))*3/5)+x/3");
		    items.Add(info);

    		info = new PresetInfo("Bullseye", "1-rint(x/y*0.5)", "1-rint(y/x*0.4)", "rint(sin(sqrt(x*x+y*y)*10))");
    		items.Add(info);

		    info = new PresetInfo("Wave", "cos(x/2)/2", "cos(y/2)/3", "rint(sin(sqrt(x*x*x+y*y)*10))");
		    items.Add(info);

		    info = new PresetInfo("Swirls", "x*15", "cos(x*y*4900)", "y*15");
		    items.Add(info);

		    info = new PresetInfo("Mod", "(x ^2) % y", "y % x", "x % y");
		    items.Add(info);

        info = new PresetInfo("Simple", "x", "y", "x+y");
        items.Add(info);

        cbPresets.Text = "Select a preset";
    }

      public float milli(float val1)
      {
        meHistory.AppendText("function \"milli\" called.\n");
        return val1 / 1000.0f;
      }

      public float not(float val1)
      {
        meHistory.AppendText("function \"not\" called.\n");
        return (val1==0) ? 1 : 0;
      }

      public float fun1(float val1)
      {
        meHistory.AppendText("demo function fun1 called.\n");
        return val1 * 2;
      }

      public float fun3(float val1, float val2, float val3)
      {
        meHistory.AppendText("demo function fun3 called.");
        return val1 + val2 + val3;
      }

      public float fun4(float val1, float val2, float val3, float val4)
      {
        meHistory.AppendText("demo function fun4 called.");
        return val1 + val2 + val3 + val4;
      }

      public float fun5(float val1, float val2, float val3, float val4, float val5)
      {
        meHistory.AppendText("demo function fun5 called.");
        return val1 + val2 + val3 + val4 + val5;
      }

      private void Calc(String expr)
      {
        try
        {
          m_history.Add(expr);

          meHistory.SelectionColor = System.Drawing.Color.Blue;
          meHistory.AppendText(expr);
          meHistory.AppendText("\r\n");
          meHistory.SelectionColor = System.Drawing.Color.Black;

          m_parser.SetDecSep(cbDec.Text.ToCharArray()[0]); // default: "."
          m_parser.SetArgSep(cbArg.Text.ToCharArray()[0]); // default: ","
          m_parser.SetExpr(expr);

          Parser.CompiledFunDelegate fun = m_parser.Compile();
          m_ans.Value = fun();
          //m_ans.Value = m_parser.Eval();

          string result = Convert.ToString(m_ans.Value);
          meHistory.AppendText(String.Format("{0} = ", m_parser.GetExpr()));
          meHistory.AppendText(result);
          meHistory.AppendText("\r\n");
        }
        catch (ParserException exc)
        {
          DumpException(exc);
        }
      }

      private void DumpException(ParserException exc)
      {
        string sMsg;

        sMsg = "An error occured:\n";
        sMsg += string.Format("  Expression:  \"{0}\"\n", exc.Expression);
        sMsg += string.Format("  Message:     \"{0}\"\n", exc.Message);
        sMsg += string.Format("  Token:       \"{0}\"\n", exc.Token);
        sMsg += string.Format("  Position:      {0}\n", exc.Position);

        meHistory.SelectionColor = System.Drawing.Color.Red;
        meHistory.AppendText(sMsg);
        meHistory.SelectionColor = System.Drawing.Color.Black;

        meHistory.SelectionStart = meHistory.TextLength;
        meHistory.ScrollToCaret();
      }

      private void edExpr_KeyDown(object sender, KeyEventArgs e)
      {
          Keys iCode = e.KeyCode;
          String expr = edExpr.Text;

          switch(iCode)
          {
          case Keys.Return:
               {
                  Calc(expr);
             
                  if (expr.Length>=0)
                      m_history.Add(expr);
                  else
                      edExpr.Text = "";

                  meHistory.SelectionStart = meHistory.TextLength;
                  meHistory.ScrollToCaret();
               }
               break;

          case Keys.Up:
          case Keys.Down:
               {
                   m_histLine = Math.Max(0, Math.Min(m_history.Count - 1, m_histLine));

                   if (m_history.Count==0)
                       break;

                   edExpr.Text = m_history[m_histLine];
                   edExpr.Select(edExpr.Text.Length, 0);

                   m_histLine += (iCode == Keys.Up) ? -1 : 1;
               }
               break;
          } // switch keyvode
      }

      private void btnListConst_Click(object sender, EventArgs e)
      {
        try
        {
          string sMsg = "";

          Dictionary<string, float> mapConst = m_parser.GetConst();

          sMsg = "Defined constants:\n";
          foreach (KeyValuePair<string, float> item in mapConst) 
          {
            sMsg += item.Key + " = ";
            sMsg += item.Value;
            sMsg += "\n";
          }
          sMsg += "\n";

          meHistory.SelectionColor = System.Drawing.Color.Blue;
          meHistory.AppendText(sMsg);
          meHistory.SelectionColor = System.Drawing.Color.Black;
          meHistory.SelectionStart = meHistory.TextLength;
          meHistory.ScrollToCaret();
        }
        catch (ParserException exc)
        {
          DumpException(exc);
        }
      }

      private void btnListVar_Click(object sender, EventArgs e)
      {
        try
        {
          string sMsg = "";

          Dictionary<string, ParserVariable> map = m_parser.GetVar();

          sMsg = "Defined variables:\n";
          foreach (KeyValuePair<string, ParserVariable> item in map)
          {
            sMsg += item.Key;
            sMsg += string.Format(" = {0}", item.Value.Value);
            sMsg += "\n";
          }
          sMsg += "\n";
          meHistory.SelectionColor = System.Drawing.Color.Blue;
          meHistory.AppendText(sMsg);
          meHistory.SelectionColor = System.Drawing.Color.Black;
          meHistory.SelectionStart = meHistory.TextLength;
          meHistory.ScrollToCaret();
        }
        catch (ParserException exc)
        {
          DumpException(exc);
        }
      }

      private void btnListExprVar_Click(object sender, EventArgs e)
      {
        string sMsg = "Expression variables:\n";

        try
        {

          Dictionary<string, IntPtr> map = m_parser.GetExprVar();

          if (map.Count > 0)
          {
            foreach (KeyValuePair<string, IntPtr> item in map)
            {
              sMsg += item.Key;
              sMsg += string.Format(" (memory location: {0:X})", item.Value.ToInt64());
              sMsg += "\n";
            }
          }
          else
            sMsg += "none";

        }
        catch (ParserException /*exc*/)
        {
          sMsg += "none";
//          DumpException(exc);
        }

        sMsg += "\n";

        meHistory.SelectionColor = System.Drawing.Color.Blue;
        meHistory.AppendText(sMsg);
        meHistory.SelectionColor = System.Drawing.Color.Black;
        meHistory.SelectionStart = meHistory.TextLength;
        meHistory.ScrollToCaret();
      }

      private void cbDec_SelectedIndexChanged(object sender, EventArgs e)
      {
        meHistory.SelectionColor = System.Drawing.Color.Blue;
        meHistory.AppendText(String.Format("Decimal separator changed to \"{0}\"\n", cbDec.Text));
        meHistory.SelectionColor = System.Drawing.Color.Black;
      }

      private void cbArg_SelectedIndexChanged(object sender, EventArgs e)
      {
        meHistory.SelectionColor = System.Drawing.Color.Blue;
        meHistory.AppendText(String.Format("Argument separator changed to \"{0}\"\n", cbArg.Text));
        meHistory.SelectionColor = System.Drawing.Color.Black;
      }

      private void cbPresets_SelectedIndexChanged(object sender, EventArgs e)
      {
		    PresetInfo preset = (PresetInfo)cbPresets.SelectedItem;
    		
		    this.tbRed.Text = preset.MyRed;
    		this.tbGreen.Text = preset.MyGreen;
    		this.tbBlue.Text = preset.MyBlue;
		    this.btnGenerate.Enabled = true;
    		this.tbRed.BackColor = Color.Empty;
    		this.tbGreen.BackColor = Color.Empty;
        this.tbBlue.BackColor = Color.Empty;
      }

      private float GetColorComponent(float d)
      {
        if (d < 0)
          return 0;
        else if (d > 1)
          return 1;
        else if (Double.IsNaN(d))
          return 0;
        else
          return d;
      }

      class CalcState
      {
        public CalcState(ManualResetEvent reset, muParser.Parser parser, int offset)
        {
            Reset = reset;
            Parser = parser;
            Offset = offset;
        }

        public ManualResetEvent Reset { get; private set; }
        public muParser.Parser Parser { get; set; }
        public int Offset { get; set; }
      }
      
      private void CalculateMultiThread(object s)
      {
        CalcState cs = s as CalcState;
        float mult = 2 * (float)Math.PI / (float)256.0;
        int index = 0;

        // Set up parser variables
        Parser p = cs.Parser;
        ParserVariable x = new ParserVariable(0);
        ParserVariable y = new ParserVariable(0);
        p.DefineVar("x", x);
        p.DefineVar("y", y);

        Parser.CompiledFunDelegate fun = p.Compile();

        // Do the actual looping for a single colorplane
        float v;
        for (int yi = 0; yi < pbImage.Height; ++yi)
        {
          y.Value = (yi - 128) * mult;
          for (int xi = 0; xi < pbImage.Width; ++xi)
          {
            x.Value = (xi - 128) * mult;

            //v = GetColorComponent(p.Eval());
            v = GetColorComponent(fun());
            m_data[index + cs.Offset] = (byte)(v * 255);
            index += 3;
          }
        }

        cs.Reset.Set();
      }

      private void CalculateSingleThread()
      {
        float mult = 2 * (float)Math.PI / (float)256.0;
        float r, g, b;
        int index = 0;

        //// Assign the parser variables
        ParserVariable x = new ParserVariable(0);
        ParserVariable y = new ParserVariable(0);
        foreach (muParser.Parser p in m_parserImg)
        {
          p.DefineVar("x", x);
          p.DefineVar("y", y);
        }
                                                        
        //Parser.CompiledFunDelegate funRed = m_parserImg[(int)EColorPlane.red].Compile();
        //Parser.CompiledFunDelegate funGreen = m_parserImg[(int)EColorPlane.green].Compile();
        //Parser.CompiledFunDelegate funBlue = m_parserImg[(int)EColorPlane.blue].Compile();
        int h = pbImage.Height;
        int w = pbImage.Width;
        for (int yi = 0; yi < h; ++yi)
        {
          y.Value = (yi - 128) * mult;
          for (int xi = 0; xi < w; ++xi)
          {
            x.Value = (xi - 128) * mult;

            r = GetColorComponent(m_parserImg[(int)EColorPlane.red].Eval());
            g = GetColorComponent(m_parserImg[(int)EColorPlane.green].Eval());
            b = GetColorComponent(m_parserImg[(int)EColorPlane.blue].Eval());

            m_data[index] = (byte)(r * 255);
            m_data[index + 1] = (byte)(g * 255);
            m_data[index + 2] = (byte)(b * 255);
            index += 3;
          }
        }
      }

      private void GenerateImage()
      {
        CalculateSingleThread();
      }

      private void btnGenerate_Click(object sender, EventArgs e)
      {
        Stopwatch sw = new Stopwatch();

        // assign expressions
        m_parserImg[(int)EColorPlane.blue].SetExpr(tbBlue.Text);
        m_parserImg[(int)EColorPlane.green].SetExpr(tbGreen.Text);
        m_parserImg[(int)EColorPlane.red].SetExpr(tbRed.Text);

        try
        {
          sw.Start();
          GenerateImage();
          sw.Stop();
        }
        catch (ParserException exc)
        {
          string sMsg;
          sMsg = "An error occured:\n";
          sMsg += string.Format("  Expression:  \"{0}\"\n", exc.Expression);
          sMsg += string.Format("  Message:     \"{0}\"\n", exc.Message);
          sMsg += string.Format("  Token:       \"{0}\"\n", exc.Token);
          sMsg += string.Format("  Position:      {0}\n", exc.Position);
          MessageBox.Show(sMsg, "Error");
          return;
        }

         // Fast transfer of all the raw values to the image
		    Rectangle rect = new Rectangle(0, 0, pbImage.Width, pbImage.Height);
        Bitmap bmp = (Bitmap)pbImage.Image;
		    System.Drawing.Imaging.BitmapData data  = bmp.LockBits(rect, 
                                                  System.Drawing.Imaging.ImageLockMode.WriteOnly, bmp.PixelFormat);
		    System.Runtime.InteropServices.Marshal.Copy(m_data, 0, data.Scan0, m_data.Length);
		    bmp.UnlockBits(data);
		    pbImage.Invalidate();

		    // Show timing results
        float seconds = sw.ElapsedMilliseconds / 1000.0f;
		    int NUM_EVALS = pbImage.Width * pbImage.Height * 3;
        lbStatus.Text = String.Format("Evaluations: {1:n0}{0}Time: {2:n2} seconds{0}Speed: {3:n0} evaluations/sec", 
                                      System.Environment.NewLine, 
                                      NUM_EVALS, 
                                      seconds, 
                                      NUM_EVALS / seconds);
      }

      private void LinkLabel1_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
      {
        System.Diagnostics.Process.Start("http://www.codeproject.com/dotnet/eval3.asp");
      }
    } // class Form1

    class PresetInfo
    {
      public string MyName, MyRed, MyGreen, MyBlue;
      public PresetInfo(string name, string red, string green, string blue)
      {
        MyName = name;
        MyRed = red;
        MyGreen = green;
        MyBlue = blue;
      }

      public override string ToString()
      {
        return MyName;
      }
    }
} // namespace muWrapper