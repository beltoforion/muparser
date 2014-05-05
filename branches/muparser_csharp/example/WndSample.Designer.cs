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
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.rbMultiReturn = new System.Windows.Forms.RadioButton();
            this.rbNormal = new System.Windows.Forms.RadioButton();
            this.rbBulk = new System.Windows.Forms.RadioButton();
            this.rbMulti = new System.Windows.Forms.RadioButton();
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
            this.groupBox1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pbImage)).BeginInit();
            this.SuspendLayout();
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.tpConsole);
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabControl1.ItemSize = new System.Drawing.Size(100, 40);
            this.tabControl1.Location = new System.Drawing.Point(0, 0);
            this.tabControl1.Margin = new System.Windows.Forms.Padding(2);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(1020, 523);
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
            this.tpConsole.Location = new System.Drawing.Point(4, 44);
            this.tpConsole.Margin = new System.Windows.Forms.Padding(2);
            this.tpConsole.Name = "tpConsole";
            this.tpConsole.Padding = new System.Windows.Forms.Padding(2);
            this.tpConsole.Size = new System.Drawing.Size(1012, 475);
            this.tpConsole.TabIndex = 0;
            this.tpConsole.Text = "Console";
            this.tpConsole.UseVisualStyleBackColor = true;
            // 
            // lbVersion
            // 
            this.lbVersion.AutoSize = true;
            this.lbVersion.Location = new System.Drawing.Point(11, 28);
            this.lbVersion.Name = "lbVersion";
            this.lbVersion.Size = new System.Drawing.Size(91, 13);
            this.lbVersion.TabIndex = 24;
            this.lbVersion.Text = "(muParser VX.XX)";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 16F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(8, 2);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(491, 26);
            this.label1.TabIndex = 23;
            this.label1.Text = "C# Wrapper for the muparser math parsing library";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(712, 38);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(102, 13);
            this.label3.TabIndex = 22;
            this.label3.Text = "Argument separator:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(552, 37);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(95, 13);
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
            this.cbDec.Location = new System.Drawing.Point(649, 34);
            this.cbDec.Name = "cbDec";
            this.cbDec.Size = new System.Drawing.Size(57, 21);
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
            this.cbArg.Location = new System.Drawing.Point(825, 34);
            this.cbArg.Name = "cbArg";
            this.cbArg.Size = new System.Drawing.Size(57, 21);
            this.cbArg.TabIndex = 19;
            this.cbArg.SelectedIndexChanged += new System.EventHandler(this.cbArg_SelectedIndexChanged);
            // 
            // btnListConst
            // 
            this.btnListConst.Location = new System.Drawing.Point(557, 5);
            this.btnListConst.Name = "btnListConst";
            this.btnListConst.Size = new System.Drawing.Size(91, 23);
            this.btnListConst.TabIndex = 18;
            this.btnListConst.Text = "List constants";
            this.btnListConst.UseVisualStyleBackColor = true;
            this.btnListConst.Click += new System.EventHandler(this.btnListConst_Click);
            // 
            // btnListExprVar
            // 
            this.btnListExprVar.Location = new System.Drawing.Point(750, 5);
            this.btnListExprVar.Name = "btnListExprVar";
            this.btnListExprVar.Size = new System.Drawing.Size(131, 23);
            this.btnListExprVar.TabIndex = 17;
            this.btnListExprVar.Text = "List expression variables";
            this.btnListExprVar.UseVisualStyleBackColor = true;
            this.btnListExprVar.Click += new System.EventHandler(this.btnListExprVar_Click);
            // 
            // btnListVar
            // 
            this.btnListVar.Location = new System.Drawing.Point(655, 4);
            this.btnListVar.Name = "btnListVar";
            this.btnListVar.Size = new System.Drawing.Size(89, 23);
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
            this.meHistory.Location = new System.Drawing.Point(12, 67);
            this.meHistory.Name = "meHistory";
            this.meHistory.Size = new System.Drawing.Size(996, 374);
            this.meHistory.TabIndex = 15;
            this.meHistory.Text = "";
            // 
            // edExpr
            // 
            this.edExpr.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.edExpr.Location = new System.Drawing.Point(13, 447);
            this.edExpr.Name = "edExpr";
            this.edExpr.Size = new System.Drawing.Size(996, 20);
            this.edExpr.TabIndex = 14;
            this.edExpr.KeyDown += new System.Windows.Forms.KeyEventHandler(this.edExpr_KeyDown);
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.groupBox1);
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
            this.tabPage2.Location = new System.Drawing.Point(4, 44);
            this.tabPage2.Margin = new System.Windows.Forms.Padding(2);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(2);
            this.tabPage2.Size = new System.Drawing.Size(1079, 634);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "Image";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox1.Controls.Add(this.rbMultiReturn);
            this.groupBox1.Controls.Add(this.rbNormal);
            this.groupBox1.Controls.Add(this.rbBulk);
            this.groupBox1.Controls.Add(this.rbMulti);
            this.groupBox1.Location = new System.Drawing.Point(831, 278);
            this.groupBox1.Margin = new System.Windows.Forms.Padding(2);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Padding = new System.Windows.Forms.Padding(2);
            this.groupBox1.Size = new System.Drawing.Size(221, 67);
            this.groupBox1.TabIndex = 29;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Evaluation";
            // 
            // rbMultiReturn
            // 
            this.rbMultiReturn.AutoSize = true;
            this.rbMultiReturn.Location = new System.Drawing.Point(117, 38);
            this.rbMultiReturn.Margin = new System.Windows.Forms.Padding(2);
            this.rbMultiReturn.Name = "rbMultiReturn";
            this.rbMultiReturn.Size = new System.Drawing.Size(79, 17);
            this.rbMultiReturn.TabIndex = 29;
            this.rbMultiReturn.TabStop = true;
            this.rbMultiReturn.Text = "MultiReturn";
            this.rbMultiReturn.UseVisualStyleBackColor = true;
            // 
            // rbNormal
            // 
            this.rbNormal.AutoSize = true;
            this.rbNormal.Location = new System.Drawing.Point(13, 17);
            this.rbNormal.Margin = new System.Windows.Forms.Padding(2);
            this.rbNormal.Name = "rbNormal";
            this.rbNormal.Size = new System.Drawing.Size(58, 17);
            this.rbNormal.TabIndex = 26;
            this.rbNormal.TabStop = true;
            this.rbNormal.Text = "Normal";
            this.rbNormal.UseVisualStyleBackColor = true;
            // 
            // rbBulk
            // 
            this.rbBulk.AutoSize = true;
            this.rbBulk.Location = new System.Drawing.Point(13, 38);
            this.rbBulk.Margin = new System.Windows.Forms.Padding(2);
            this.rbBulk.Name = "rbBulk";
            this.rbBulk.Size = new System.Drawing.Size(72, 17);
            this.rbBulk.TabIndex = 28;
            this.rbBulk.TabStop = true;
            this.rbBulk.Text = "Bulkmode";
            this.rbBulk.UseVisualStyleBackColor = true;
            // 
            // rbMulti
            // 
            this.rbMulti.AutoSize = true;
            this.rbMulti.Location = new System.Drawing.Point(117, 17);
            this.rbMulti.Margin = new System.Windows.Forms.Padding(2);
            this.rbMulti.Name = "rbMulti";
            this.rbMulti.Size = new System.Drawing.Size(89, 17);
            this.rbMulti.TabIndex = 27;
            this.rbMulti.TabStop = true;
            this.rbMulti.Text = "Multithreaded";
            this.rbMulti.UseVisualStyleBackColor = true;
            // 
            // LinkLabel1
            // 
            this.LinkLabel1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.LinkLabel1.LinkArea = new System.Windows.Forms.LinkArea(34, 13);
            this.LinkLabel1.Location = new System.Drawing.Point(833, 373);
            this.LinkLabel1.Name = "LinkLabel1";
            this.LinkLabel1.Size = new System.Drawing.Size(146, 37);
            this.LinkLabel1.TabIndex = 24;
            this.LinkLabel1.TabStop = true;
            this.LinkLabel1.Text = "This demo is based on a sample by Pascal Ganaye";
            this.LinkLabel1.UseCompatibleTextRendering = true;
            this.LinkLabel1.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.LinkLabel1_LinkClicked);
            // 
            // label8
            // 
            this.label8.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(831, 179);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(40, 13);
            this.label8.TabIndex = 23;
            this.label8.Text = "Status:";
            // 
            // lbStatus
            // 
            this.lbStatus.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.lbStatus.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.lbStatus.Location = new System.Drawing.Point(833, 193);
            this.lbStatus.Name = "lbStatus";
            this.lbStatus.Size = new System.Drawing.Size(219, 54);
            this.lbStatus.TabIndex = 22;
            // 
            // btnGenerate
            // 
            this.btnGenerate.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnGenerate.Location = new System.Drawing.Point(831, 250);
            this.btnGenerate.Name = "btnGenerate";
            this.btnGenerate.Size = new System.Drawing.Size(223, 23);
            this.btnGenerate.TabIndex = 21;
            this.btnGenerate.Text = "Generate";
            this.btnGenerate.UseVisualStyleBackColor = true;
            this.btnGenerate.Click += new System.EventHandler(this.btnGenerate_Click);
            // 
            // tbBlue
            // 
            this.tbBlue.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.tbBlue.Location = new System.Drawing.Point(833, 158);
            this.tbBlue.Name = "tbBlue";
            this.tbBlue.Size = new System.Drawing.Size(220, 20);
            this.tbBlue.TabIndex = 20;
            // 
            // tbGreen
            // 
            this.tbGreen.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.tbGreen.Location = new System.Drawing.Point(833, 120);
            this.tbGreen.Name = "tbGreen";
            this.tbGreen.Size = new System.Drawing.Size(220, 20);
            this.tbGreen.TabIndex = 18;
            // 
            // tbRed
            // 
            this.tbRed.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.tbRed.Location = new System.Drawing.Point(833, 82);
            this.tbRed.Name = "tbRed";
            this.tbRed.Size = new System.Drawing.Size(220, 20);
            this.tbRed.TabIndex = 17;
            // 
            // label5
            // 
            this.label5.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(831, 141);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(31, 13);
            this.label5.TabIndex = 19;
            this.label5.Text = "Blue:";
            // 
            // label6
            // 
            this.label6.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(831, 103);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(39, 13);
            this.label6.TabIndex = 16;
            this.label6.Text = "Green:";
            // 
            // label7
            // 
            this.label7.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(831, 65);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(30, 13);
            this.label7.TabIndex = 15;
            this.label7.Text = "Red:";
            // 
            // label4
            // 
            this.label4.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(831, 13);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(42, 13);
            this.label4.TabIndex = 14;
            this.label4.Text = "Presets";
            // 
            // cbPresets
            // 
            this.cbPresets.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.cbPresets.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbPresets.FormattingEnabled = true;
            this.cbPresets.Location = new System.Drawing.Point(833, 29);
            this.cbPresets.Margin = new System.Windows.Forms.Padding(2);
            this.cbPresets.Name = "cbPresets";
            this.cbPresets.Size = new System.Drawing.Size(204, 21);
            this.cbPresets.TabIndex = 1;
            this.cbPresets.SelectedIndexChanged += new System.EventHandler(this.cbPresets_SelectedIndexChanged);
            // 
            // pbImage
            // 
            this.pbImage.Location = new System.Drawing.Point(6, 5);
            this.pbImage.Margin = new System.Windows.Forms.Padding(2);
            this.pbImage.Name = "pbImage";
            this.pbImage.Size = new System.Drawing.Size(800, 800);
            this.pbImage.TabIndex = 0;
            this.pbImage.TabStop = false;
            // 
            // WndSample
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1020, 523);
            this.Controls.Add(this.tabControl1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.MinimumSize = new System.Drawing.Size(299, 198);
            this.Name = "WndSample";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "muparser for C#";
            this.tabControl1.ResumeLayout(false);
            this.tpConsole.ResumeLayout(false);
            this.tpConsole.PerformLayout();
            this.tabPage2.ResumeLayout(false);
            this.tabPage2.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
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
        private System.Windows.Forms.RadioButton rbBulk;
        private System.Windows.Forms.RadioButton rbMulti;
        private System.Windows.Forms.RadioButton rbNormal;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.RadioButton rbMultiReturn;

    }
}

