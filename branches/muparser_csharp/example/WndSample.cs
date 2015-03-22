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


namespace sample
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
        private List<String> m_history;
        private int m_histLine;

        private byte[] m_data;

        private ParserVariable m_val1 = new ParserVariable(0);
        private ParserVariable m_val2 = new ParserVariable(0);
        private ParserVariable m_ans = new ParserVariable(0);

        private double[] m_xarray;
        private double[] m_yarray;

        public WndSample()
        {
            InitializeComponent();
            CreateInitialImage();
            PopulatePresets();

            cbDec.SelectedIndex = 0;
            cbArg.SelectedIndex = 1;
            m_data = new byte[pbImage.Width * pbImage.Height * 3];
            rbNormal.Checked = true;

            try
            {
                m_parser = new Parser(Parser.EBaseType.tpDOUBLE);
                lbVersion.Text = String.Format("muparser V{0}", m_parser.GetVersion());
                m_parser.DefineFun("fun1", new Parser.Fun1Delegate(fun1), true);
                m_parser.DefineFun("fun3", new Parser.Fun3Delegate(fun3), true);
                m_parser.DefineFun("fun4", new Parser.Fun4Delegate(fun4), true);
                m_parser.DefineFun("fun5", new Parser.Fun5Delegate(fun5), true);
                m_parser.DefineFun("fun6", new Parser.Fun6Delegate(fun6), true);
                m_parser.DefineFun("fun7", new Parser.Fun7Delegate(fun7), true);
                m_parser.DefineFun("fun8", new Parser.Fun8Delegate(fun8), true);
                m_parser.DefineFun("fun9", new Parser.Fun9Delegate(fun9), true);
                m_parser.DefineFun("fun10", new Parser.Fun10Delegate(fun10), true);

                m_parser.DefineFun("prod", new Parser.MultFunDelegate(prod), true);
                m_parser.DefineOprt("%", new Parser.Fun2Delegate(mod), 2);
                m_parser.DefineOprt("and", new Parser.Fun2Delegate(mod), 2);

                //m_parser.DefinePostfixOprt("m", new muParser.Parser.Fun1Delegate(milli));
                m_parser.DefineInfixOprt("!", new muParser.Parser.Fun1Delegate(not), muParser.Parser.EPrec.prLOGIC);

                m_parser.DefineVar("ans", m_ans);
                m_parser.DefineVar("my_var1", m_val1);
                m_parser.DefineVar("my_var2", m_val2);

                // Initialize parsers used for the image calculation
                for (int i = 0; i < 3; ++i)
                {
                    m_parserImg[i] = new Parser(Parser.EBaseType.tpDOUBLE);
                    m_parserImg[i].DefineOprt("%", new Parser.Fun2Delegate(mod), 2);
                }

                PreCalculateBulkVariables();
            }
            catch (ParserException exc)
            {
                DumpException(exc);
            }

            m_history = new List<String>();
        }

        private void PreCalculateBulkVariables()
        {
            // Precalculating the bulk variables needs to be done only once!
            m_xarray = new double[pbImage.Height * pbImage.Width];
            m_yarray = new double[pbImage.Height * pbImage.Width];

            double mult = 2 * Math.PI / 800;
            int ct = 0;
            for (int yi = 0; yi < pbImage.Height; ++yi)
            {
                for (int xi = 0; xi < pbImage.Width; ++xi)
                {
                    m_xarray[ct] = (xi - 400) * mult;
                    m_yarray[ct] = (yi - 400) * mult;
                    ++ct;
                }
            }
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

            cbPresets.SelectedIndex = 0;
        }

        public double prod(double[] a, int size)
        {
            meHistory.AppendText("demo function prod called.\n");

            double val = 1;
            for (int i = 0; i < size; ++i)
                val *= a[i];

            return val;
        }

        public double strFun1(String str, double val1)
        {
            return val1 * 2;
        }

        public double strFun2(String str, double val1, double val2)
        {
            return val1 + val2;
        }

        public double strFun3(String str, double val1, double val2, double val3)
        {
            return val1 + val2 + val3;
        }

        public double milli(double val1)
        {
            meHistory.AppendText("function \"milli\" called.\n");
            return val1 / 1000.0;
        }

        public double not(double val1)
        {
            meHistory.AppendText("function \"not\" called.\n");
            return (val1 == 0) ? 1 : 0;
        }

        public double fun1(double val1)
        {
            meHistory.AppendText("demo function fun1 called.\n");
            return val1 * 2;
        }

        public double mod(double val1, double val2)
        {
            return val1 % val2;
        }

        public double fun3(double val1, double val2, double val3)
        {
            meHistory.AppendText("demo function fun3 called.");
            return val1 + val2 + val3;
        }

        public double fun4(double val1, double val2, double val3, double val4)
        {
            meHistory.AppendText("demo function fun4 called.");
            return val1 + val2 + val3 + val4;
        }

        public double fun5(double val1, double val2, double val3, double val4, double val5)
        {
            meHistory.AppendText("demo function fun5 called.");
            return val1 + val2 + val3 + val4 + val5;
        }

        public double fun6(double val1, double val2, double val3, double val4, double val5, double val6)
        {
            meHistory.AppendText("demo function fun6 called.");
            return val1 + val2 + val3 + val4 + val5 + val6;
        }

        public double fun7(double val1, double val2, double val3, double val4, double val5, double val6, double val7)
        {
            meHistory.AppendText("demo function fun7 called.");
            return val1 + val2 + val3 + val4 + val5 + val6 + val7;
        }

        public double fun8(double val1, double val2, double val3, double val4, double val5, double val6, double val7, double val8)
        {
            meHistory.AppendText("demo function fun8 called.");
            return val1 + val2 + val3 + val4 + val5 + val6 + val7 + val8;
        }

        public double fun9(double val1, double val2, double val3, double val4, double val5, double val6, double val7, double val8, double val9)
        {
            meHistory.AppendText("demo function fun9 called.");
            return val1 + val2 + val3 + val4 + val5 + val6 + val7 + val8 + val9;
        }

        public double fun10(double val1, double val2, double val3, double val4, double val5, double val6, double val7, double val8, double val9, double val10)
        {
            meHistory.AppendText("demo function fun10 called.");
            return val1 + val2 + val3 + val4 + val5 + val6 + val7 + val8 + val9 + val10;
        }

        // Test for FilterFactory Bug report
        public double cnv0(double a00, double a01, double a02, double a10, double a11, double a12,
                           double a20, double a21, double a22, double divisor)
        {
            double val = 0;
            val = a00 + a01 + a02 + a10 + a11 + a12 + a20 + a21 + a22 + divisor;
            return val;
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
                m_ans.Value = m_parser.Eval();

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

            switch (iCode)
            {
                case Keys.Return:
                    {
                        Calc(expr);

                        if (expr.Length >= 0)
                            m_history.Add(expr);

                        edExpr.Text = "";

                        meHistory.SelectionStart = meHistory.TextLength;
                        meHistory.ScrollToCaret();
                    }
                    break;

                case Keys.Up:
                case Keys.Down:
                    {
                        m_histLine = Math.Max(0, Math.Min(m_history.Count - 1, m_histLine));

                        if (m_history.Count == 0)
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

                Dictionary<string, double> mapConst = m_parser.GetConst();

                sMsg = "Defined constants:\n";
                foreach (KeyValuePair<string, double> item in mapConst)
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
            try
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
                }

                sMsg += "\n";

                meHistory.SelectionColor = System.Drawing.Color.Blue;
                meHistory.AppendText(sMsg);
                meHistory.SelectionColor = System.Drawing.Color.Black;
                meHistory.SelectionStart = meHistory.TextLength;
                meHistory.ScrollToCaret();
            }
            catch (Exception exc)
            {
                MessageBox.Show(exc.Message);
            }
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

        private double GetColorComponent(double d)
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
            double mult = 2 * Math.PI / 800;
            int index = 0;

            // Set up parser variables
            Parser p = cs.Parser;
            ParserVariable x = new ParserVariable(0);
            ParserVariable y = new ParserVariable(0);
            p.DefineVar("x", x);
            p.DefineVar("y", y);

            // Do the actual looping for a single colorplane
            double v;
            for (int yi = 0; yi < pbImage.Height; ++yi)
            {
                y.Value = (yi - 400) * mult;
                for (int xi = 0; xi < pbImage.Width; ++xi)
                {
                    x.Value = (xi - 400) * mult;

                    v = GetColorComponent(p.Eval());
                    m_data[index + cs.Offset] = (byte)(v * 255);
                    index += 3;
                }
            }

            cs.Reset.Set();
        }

        private void CalculateBulk()
        {
            try
            {
                // Set up parser variables
                double[] rr = new double[pbImage.Height * pbImage.Width];
                double[] rg = new double[pbImage.Height * pbImage.Width];
                double[] rb = new double[pbImage.Height * pbImage.Width];

                // Note: The variable arrays for "x" and "y" were precalculated in 
                // PreCalculateBulkVariables! For each variable you need 
                Parser pr = m_parserImg[(int)EColorPlane.red];
                pr.DefineVar("x", m_xarray);
                pr.DefineVar("y", m_yarray);

                Parser pg = m_parserImg[(int)EColorPlane.green];
                pg.DefineVar("x", m_xarray);
                pg.DefineVar("y", m_yarray);

                Parser pb = m_parserImg[(int)EColorPlane.blue];
                pb.DefineVar("x", m_xarray);
                pb.DefineVar("y", m_yarray);

                // Calculate the expressions for all three colorplanes using bulk mode (Open MP)
                pr.Eval(rr, rr.Length);
                pg.Eval(rg, rg.Length);
                pb.Eval(rb, rb.Length);

                // Combine the results into a single image
                int index = 0;
                double r, g, b;
                for (int i = 0; i < rr.Length; ++i)
                {
                    r = GetColorComponent(rr[i]);
                    g = GetColorComponent(rg[i]);
                    b = GetColorComponent(rb[i]);

                    m_data[index] = (byte)(r * 255);
                    m_data[index + 1] = (byte)(g * 255);
                    m_data[index + 2] = (byte)(b * 255);
                    index += 3;
                }
            }
            catch (ParserException exc)
            {
                DumpException(exc);
            }
            catch (Exception exc)
            {
                MessageBox.Show(exc.Message);
            }
        }


        private void CalculateSingleThread()
        {
            double mult = 2 * Math.PI / 800;
            double r, g, b;
            int index = 0;

            // Assign the parser variables
            ParserVariable x = new ParserVariable(0);
            ParserVariable y = new ParserVariable(0);
            foreach (muParser.Parser p in m_parserImg)
            {
                p.DefineVar("x", x);
                p.DefineVar("y", y);
            }

            for (int yi = 0; yi < pbImage.Height; ++yi)
            {
                y.Value = (yi - 400) * mult;
                for (int xi = 0; xi < pbImage.Width; ++xi)
                {
                    x.Value = (xi - 400) * mult;

                    r = m_parserImg[(int)EColorPlane.red].Eval();
                    g = m_parserImg[(int)EColorPlane.green].Eval();
                    b = m_parserImg[(int)EColorPlane.blue].Eval();

                    r = GetColorComponent(r);
                    g = GetColorComponent(g);
                    b = GetColorComponent(b);

                    m_data[index] = (byte)(r * 255);
                    m_data[index + 1] = (byte)(g * 255);
                    m_data[index + 2] = (byte)(b * 255);
                    index += 3;
                }
            }
        }

        private void CalculateMultiReturn()
        {
            double mult = 2 * Math.PI / 800;
            double r, g, b;
            int index = 0;

            // Assign the parser variables
            ParserVariable x = new ParserVariable(0);
            ParserVariable y = new ParserVariable(0);
            muParser.Parser p = new muParser.Parser(muParser.Parser.EBaseType.tpDOUBLE);
            p.DefineOprt("%", new Parser.Fun2Delegate(mod), 2);
            p.SetExpr(m_parserImg[0].GetExpr() + "," + m_parserImg[1].GetExpr() + "," + m_parserImg[2].GetExpr());
            p.DefineVar("x", x);
            p.DefineVar("y", y);

            for (int yi = 0; yi < pbImage.Height; ++yi)
            {
                y.Value = (yi - 400) * mult;
                for (int xi = 0; xi < pbImage.Width; ++xi)
                {
                    x.Value = (xi - 400) * mult;

                    double[] ret = p.EvalMultiExpr();
                    r = GetColorComponent(ret[0]);
                    g = GetColorComponent(ret[1]);
                    b = GetColorComponent(ret[2]);

                    m_data[index] = (byte)(r * 255);
                    m_data[index + 1] = (byte)(g * 255);
                    m_data[index + 2] = (byte)(b * 255);
                    index += 3;
                }
            }
        }


        private void GenerateImage()
        {
            if (rbMulti.Checked)
            {
                List<ManualResetEvent> events = new List<ManualResetEvent>();
                int offset = 0;
                foreach (muParser.Parser p in m_parserImg)
                {
                    CalcState cs = new CalcState(new ManualResetEvent(false), p, offset);
                    ++offset;
                    events.Add(cs.Reset);
                    ThreadPool.QueueUserWorkItem(new WaitCallback(CalculateMultiThread), cs);
                }
                WaitHandle.WaitAll(events.ToArray());
            }
            else if (rbNormal.Checked)
            {
                CalculateSingleThread();
            }
            else if (rbMultiReturn.Checked)
            {
                CalculateMultiReturn();
            }
            else
            {
                CalculateBulk();
            }
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
            System.Drawing.Imaging.BitmapData data = bmp.LockBits(rect,
                                                  System.Drawing.Imaging.ImageLockMode.WriteOnly, bmp.PixelFormat);
            System.Runtime.InteropServices.Marshal.Copy(m_data, 0, data.Scan0, m_data.Length);
            bmp.UnlockBits(data);
            pbImage.Invalidate();

            // Show timing results
            double seconds = sw.ElapsedMilliseconds / 1000.0;
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
    } // class WndSample

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