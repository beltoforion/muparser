namespace muWrapper
{
    partial class WndSample
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
          this.tabControl1 = new System.Windows.Forms.TabControl();
          this.tpConsole = new System.Windows.Forms.TabPage();
          this.lbVersion = new System.Windows.Forms.Label();
          this.label1 = new System.Windows.Forms.Label();
          this.label3 = new System.Windows.Forms.Label();
          this.label2 = new System.Windows.Forms.Label();
          this.cbDec = new System.Windows.Forms.ComboBox();
          this.cbArg = new System.Windows.Forms.ComboBox();
          this.btnListConst = new System.Windows.Forms.Button();
          this.btnListExprVar = new System.Windows.Forms.Button();
          this.btnListVar = new System.Windows.Forms.Button();
          this.meHistory = new System.Windows.Forms.RichTextBox();
          this.edExpr = new System.Windows.Forms.TextBox();
          this.tabPage2 = new System.Windows.Forms.TabPage();
          this.LinkLabel1 = new System.Windows.Forms.LinkLabel();
          this.label8 = new System.Windows.Forms.Label();
          this.lbStatus = new System.Windows.Forms.Label();
          this.btnGenerate = new System.Windows.Forms.Button();
          this.tbBlue = new System.Windows.Forms.TextBox();
          this.tbGreen = new System.Windows.Forms.TextBox();
          this.tbRed = new System.Windows.Forms.TextBox();
          this.label5 = new System.Windows.Forms.Label();
          this.label6 = new System.Windows.Forms.Label();
          this.label7 = new System.Windows.Forms.Label();
          this.label4 = new System.Windows.Forms.Label();
          this.cbPresets = new System.Windows.Forms.ComboBox();
          this.pbImage = new System.Windows.Forms.PictureBox();
          this.tabControl1.SuspendLayout();
          this.tpConsole.SuspendLayout();
          this.tabPage2.SuspendLayout();
          ((System.ComponentModel.ISupportInitialize)(this.pbImage)).BeginInit();
          this.SuspendLayout();
          // 
          // tabControl1
          // 
          this.tabControl1.Controls.Add(this.tpConsole);
          this.tabControl1.Controls.Add(this.tabPage2);
          this.tabControl1.Dock = System.Windows.Forms.DockStyle.Fill;
          this.tabControl1.Location = new System.Drawing.Point(0, 0);
          this.tabControl1.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
          this.tabControl1.Name = "tabControl1";
          this.tabControl1.SelectedIndex = 0;
          this.tabControl1.Size = new System.Drawing.Size(837, 543);
          this.tabControl1.TabIndex = 14;
          // 
          // tpConsole
          // 
          this.tpConsole.Controls.Add(this.lbVersion);
          this.tpConsole.Controls.Add(this.label1);
          this.tpConsole.Controls.Add(this.label3);
          this.tpConsole.Controls.Add(this.label2);
          this.tpConsole.Controls.Add(this.cbDec);
          this.tpConsole.Controls.Add(this.cbArg);
          this.tpConsole.Controls.Add(this.btnListConst);
          this.tpConsole.Controls.Add(this.btnListExprVar);
          this.tpConsole.Controls.Add(this.btnListVar);
          this.tpConsole.Controls.Add(this.meHistory);
          this.tpConsole.Controls.Add(this.edExpr);
          this.tpConsole.Location = new System.Drawing.Point(4, 25);
          this.tpConsole.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
          this.tpConsole.Name = "tpConsole";
          this.tpConsole.Padding = new System.Windows.Forms.Padding(3, 2, 3, 2);
          this.tpConsole.Size = new System.Drawing.Size(829, 514);
          this.tpConsole.TabIndex = 0;
          this.tpConsole.Text = "Console";
          this.tpConsole.UseVisualStyleBackColor = true;
          // 
          // lbVersion
          // 
          this.lbVersion.AutoSize = true;
          this.lbVersion.Location = new System.Drawing.Point(15, 34);
          this.lbVersion.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
          this.lbVersion.Name = "lbVersion";
          this.lbVersion.Size = new System.Drawing.Size(150, 17);
          this.lbVersion.TabIndex = 24;
          this.lbVersion.Text = "(muParserSSE VX.XX)";
          // 
          // label1
          // 
          this.label1.AutoSize = true;
          this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 16F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
          this.label1.Location = new System.Drawing.Point(11, 2);
          this.label1.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
          this.label1.Name = "label1";
          this.label1.Size = new System.Drawing.Size(322, 31);
          this.label1.TabIndex = 23;
          this.label1.Text = "C# Wrapper for muParser";
          // 
          // label3
          // 
          this.label3.AutoSize = true;
          this.label3.Location = new System.Drawing.Point(592, 53);
          this.label3.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
          this.label3.Name = "label3";
          this.label3.Size = new System.Drawing.Size(138, 17);
          this.label3.TabIndex = 22;
          this.label3.Text = "Argument separator:";
          // 
          // label2
          // 
          this.label2.AutoSize = true;
          this.label2.Location = new System.Drawing.Point(379, 52);
          this.label2.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
          this.label2.Name = "label2";
          this.label2.Size = new System.Drawing.Size(127, 17);
          this.label2.TabIndex = 21;
          this.label2.Text = "Decimal separator:";
          // 
          // cbDec
          // 
          this.cbDec.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
          this.cbDec.FormattingEnabled = true;
          this.cbDec.Items.AddRange(new object[] {
            ".",
            ","});
          this.cbDec.Location = new System.Drawing.Point(508, 48);
          this.cbDec.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
          this.cbDec.Name = "cbDec";
          this.cbDec.Size = new System.Drawing.Size(75, 24);
          this.cbDec.TabIndex = 20;
          this.cbDec.SelectedIndexChanged += new System.EventHandler(this.cbDec_SelectedIndexChanged);
          // 
          // cbArg
          // 
          this.cbArg.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
          this.cbArg.FormattingEnabled = true;
          this.cbArg.Items.AddRange(new object[] {
            ".",
            ",",
            ";"});
          this.cbArg.Location = new System.Drawing.Point(743, 48);
          this.cbArg.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
          this.cbArg.Name = "cbArg";
          this.cbArg.Size = new System.Drawing.Size(75, 24);
          this.cbArg.TabIndex = 19;
          this.cbArg.SelectedIndexChanged += new System.EventHandler(this.cbArg_SelectedIndexChanged);
          // 
          // btnListConst
          // 
          this.btnListConst.Location = new System.Drawing.Point(381, 6);
          this.btnListConst.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
          this.btnListConst.Name = "btnListConst";
          this.btnListConst.Size = new System.Drawing.Size(121, 28);
          this.btnListConst.TabIndex = 18;
          this.btnListConst.Text = "List constants";
          this.btnListConst.UseVisualStyleBackColor = true;
          this.btnListConst.Click += new System.EventHandler(this.btnListConst_Click);
          // 
          // btnListExprVar
          // 
          this.btnListExprVar.Location = new System.Drawing.Point(637, 6);
          this.btnListExprVar.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
          this.btnListExprVar.Name = "btnListExprVar";
          this.btnListExprVar.Size = new System.Drawing.Size(175, 28);
          this.btnListExprVar.TabIndex = 17;
          this.btnListExprVar.Text = "List expression variables";
          this.btnListExprVar.UseVisualStyleBackColor = true;
          this.btnListExprVar.Click += new System.EventHandler(this.btnListExprVar_Click);
          // 
          // btnListVar
          // 
          this.btnListVar.Location = new System.Drawing.Point(511, 6);
          this.btnListVar.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
          this.btnListVar.Name = "btnListVar";
          this.btnListVar.Size = new System.Drawing.Size(119, 28);
          this.btnListVar.TabIndex = 16;
          this.btnListVar.Text = "List variables";
          this.btnListVar.UseVisualStyleBackColor = true;
          this.btnListVar.Click += new System.EventHandler(this.btnListVar_Click);
          // 
          // meHistory
          // 
          this.meHistory.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                      | System.Windows.Forms.AnchorStyles.Left)
                      | System.Windows.Forms.AnchorStyles.Right)));
          this.meHistory.Location = new System.Drawing.Point(16, 82);
          this.meHistory.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
          this.meHistory.Name = "meHistory";
          this.meHistory.Size = new System.Drawing.Size(800, 379);
          this.meHistory.TabIndex = 15;
          this.meHistory.Text = "";
          // 
          // edExpr
          // 
          this.edExpr.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                      | System.Windows.Forms.AnchorStyles.Right)));
          this.edExpr.Location = new System.Drawing.Point(16, 470);
          this.edExpr.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
          this.edExpr.Name = "edExpr";
          this.edExpr.Size = new System.Drawing.Size(800, 22);
          this.edExpr.TabIndex = 14;
          this.edExpr.KeyDown += new System.Windows.Forms.KeyEventHandler(this.edExpr_KeyDown);
          // 
          // tabPage2
          // 
          this.tabPage2.Controls.Add(this.LinkLabel1);
          this.tabPage2.Controls.Add(this.label8);
          this.tabPage2.Controls.Add(this.lbStatus);
          this.tabPage2.Controls.Add(this.btnGenerate);
          this.tabPage2.Controls.Add(this.tbBlue);
          this.tabPage2.Controls.Add(this.tbGreen);
          this.tabPage2.Controls.Add(this.tbRed);
          this.tabPage2.Controls.Add(this.label5);
          this.tabPage2.Controls.Add(this.label6);
          this.tabPage2.Controls.Add(this.label7);
          this.tabPage2.Controls.Add(this.label4);
          this.tabPage2.Controls.Add(this.cbPresets);
          this.tabPage2.Controls.Add(this.pbImage);
          this.tabPage2.Location = new System.Drawing.Point(4, 25);
          this.tabPage2.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
          this.tabPage2.Name = "tabPage2";
          this.tabPage2.Padding = new System.Windows.Forms.Padding(3, 2, 3, 2);
          this.tabPage2.Size = new System.Drawing.Size(829, 514);
          this.tabPage2.TabIndex = 1;
          this.tabPage2.Text = "Image";
          this.tabPage2.UseVisualStyleBackColor = true;
          // 
          // LinkLabel1
          // 
          this.LinkLabel1.LinkArea = new System.Windows.Forms.LinkArea(34, 13);
          this.LinkLabel1.Location = new System.Drawing.Point(528, 462);
          this.LinkLabel1.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
          this.LinkLabel1.Name = "LinkLabel1";
          this.LinkLabel1.Size = new System.Drawing.Size(195, 46);
          this.LinkLabel1.TabIndex = 24;
          this.LinkLabel1.TabStop = true;
          this.LinkLabel1.Text = "This demo is based on a sample by Pascal Ganaye";
          this.LinkLabel1.UseCompatibleTextRendering = true;
          this.LinkLabel1.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.LinkLabel1_LinkClicked);
          // 
          // label8
          // 
          this.label8.AutoSize = true;
          this.label8.Location = new System.Drawing.Point(525, 223);
          this.label8.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
          this.label8.Name = "label8";
          this.label8.Size = new System.Drawing.Size(52, 17);
          this.label8.TabIndex = 23;
          this.label8.Text = "Status:";
          // 
          // lbStatus
          // 
          this.lbStatus.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
          this.lbStatus.Location = new System.Drawing.Point(528, 240);
          this.lbStatus.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
          this.lbStatus.Name = "lbStatus";
          this.lbStatus.Size = new System.Drawing.Size(292, 66);
          this.lbStatus.TabIndex = 22;
          // 
          // btnGenerate
          // 
          this.btnGenerate.Location = new System.Drawing.Point(528, 326);
          this.btnGenerate.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
          this.btnGenerate.Name = "btnGenerate";
          this.btnGenerate.Size = new System.Drawing.Size(297, 28);
          this.btnGenerate.TabIndex = 21;
          this.btnGenerate.Text = "Generate";
          this.btnGenerate.UseVisualStyleBackColor = true;
          this.btnGenerate.Click += new System.EventHandler(this.btnGenerate_Click);
          // 
          // tbBlue
          // 
          this.tbBlue.Location = new System.Drawing.Point(528, 197);
          this.tbBlue.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
          this.tbBlue.Name = "tbBlue";
          this.tbBlue.Size = new System.Drawing.Size(292, 22);
          this.tbBlue.TabIndex = 20;
          // 
          // tbGreen
          // 
          this.tbGreen.Location = new System.Drawing.Point(528, 150);
          this.tbGreen.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
          this.tbGreen.Name = "tbGreen";
          this.tbGreen.Size = new System.Drawing.Size(292, 22);
          this.tbGreen.TabIndex = 18;
          // 
          // tbRed
          // 
          this.tbRed.Location = new System.Drawing.Point(528, 103);
          this.tbRed.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
          this.tbRed.Name = "tbRed";
          this.tbRed.Size = new System.Drawing.Size(292, 22);
          this.tbRed.TabIndex = 17;
          // 
          // label5
          // 
          this.label5.AutoSize = true;
          this.label5.Location = new System.Drawing.Point(525, 176);
          this.label5.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
          this.label5.Name = "label5";
          this.label5.Size = new System.Drawing.Size(40, 17);
          this.label5.TabIndex = 19;
          this.label5.Text = "Blue:";
          // 
          // label6
          // 
          this.label6.AutoSize = true;
          this.label6.Location = new System.Drawing.Point(525, 129);
          this.label6.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
          this.label6.Name = "label6";
          this.label6.Size = new System.Drawing.Size(52, 17);
          this.label6.TabIndex = 16;
          this.label6.Text = "Green:";
          // 
          // label7
          // 
          this.label7.AutoSize = true;
          this.label7.Location = new System.Drawing.Point(525, 82);
          this.label7.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
          this.label7.Name = "label7";
          this.label7.Size = new System.Drawing.Size(38, 17);
          this.label7.TabIndex = 15;
          this.label7.Text = "Red:";
          // 
          // label4
          // 
          this.label4.AutoSize = true;
          this.label4.Location = new System.Drawing.Point(525, 18);
          this.label4.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
          this.label4.Name = "label4";
          this.label4.Size = new System.Drawing.Size(56, 17);
          this.label4.TabIndex = 14;
          this.label4.Text = "Presets";
          // 
          // cbPresets
          // 
          this.cbPresets.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
          this.cbPresets.FormattingEnabled = true;
          this.cbPresets.Location = new System.Drawing.Point(528, 38);
          this.cbPresets.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
          this.cbPresets.Name = "cbPresets";
          this.cbPresets.Size = new System.Drawing.Size(271, 24);
          this.cbPresets.TabIndex = 1;
          this.cbPresets.SelectedIndexChanged += new System.EventHandler(this.cbPresets_SelectedIndexChanged);
          // 
          // pbImage
          // 
          this.pbImage.Location = new System.Drawing.Point(8, 6);
          this.pbImage.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
          this.pbImage.Name = "pbImage";
          this.pbImage.Size = new System.Drawing.Size(500, 500);
          this.pbImage.TabIndex = 0;
          this.pbImage.TabStop = false;
          // 
          // WndSample
          // 
          this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
          this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
          this.ClientSize = new System.Drawing.Size(837, 543);
          this.Controls.Add(this.tabControl1);
          this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
          this.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
          this.MinimumSize = new System.Drawing.Size(397, 238);
          this.Name = "WndSample";
          this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
          this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
          this.Text = "muParserSSE for C#";
          this.TopMost = true;
          this.tabControl1.ResumeLayout(false);
          this.tpConsole.ResumeLayout(false);
          this.tpConsole.PerformLayout();
          this.tabPage2.ResumeLayout(false);
          this.tabPage2.PerformLayout();
          ((System.ComponentModel.ISupportInitialize)(this.pbImage)).EndInit();
          this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tpConsole;
        private System.Windows.Forms.Label lbVersion;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ComboBox cbDec;
        private System.Windows.Forms.ComboBox cbArg;
        private System.Windows.Forms.Button btnListConst;
        private System.Windows.Forms.Button btnListExprVar;
        private System.Windows.Forms.Button btnListVar;
        private System.Windows.Forms.RichTextBox meHistory;
        private System.Windows.Forms.TextBox edExpr;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.PictureBox pbImage;
        internal System.Windows.Forms.Label label4;
        private System.Windows.Forms.ComboBox cbPresets;
        internal System.Windows.Forms.Button btnGenerate;
        internal System.Windows.Forms.TextBox tbBlue;
        internal System.Windows.Forms.TextBox tbGreen;
        internal System.Windows.Forms.TextBox tbRed;
        internal System.Windows.Forms.Label label5;
        internal System.Windows.Forms.Label label6;
        internal System.Windows.Forms.Label label7;
        internal System.Windows.Forms.Label label8;
        internal System.Windows.Forms.Label lbStatus;
        internal System.Windows.Forms.LinkLabel LinkLabel1;

    }
}

