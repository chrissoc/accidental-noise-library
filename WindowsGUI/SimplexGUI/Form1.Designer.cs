namespace SimplexGUI
{
    partial class Form1
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
			this.pictureBoxSample = new System.Windows.Forms.PictureBox();
			this.txtOffsetX = new System.Windows.Forms.TextBox();
			this.label1 = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			this.btnRandomOffset = new System.Windows.Forms.Button();
			this.btnQuickSave = new System.Windows.Forms.Button();
			this.label3 = new System.Windows.Forms.Label();
			this.txtSampleHeight = new System.Windows.Forms.TextBox();
			this.txtOffsetY = new System.Windows.Forms.TextBox();
			this.txtSampleWidth = new System.Windows.Forms.TextBox();
			this.label4 = new System.Windows.Forms.Label();
			this.label5 = new System.Windows.Forms.Label();
			this.txtQuickSaveDir = new System.Windows.Forms.TextBox();
			this.btnQuickSaveBrowse = new System.Windows.Forms.Button();
			this.btnSaveAs = new System.Windows.Forms.Button();
			this.btnSample = new System.Windows.Forms.Button();
			this.label6 = new System.Windows.Forms.Label();
			this.lblElapsedTime = new System.Windows.Forms.Label();
			this.txtNoiseExpression = new System.Windows.Forms.TextBox();
			this.btnParseExpression = new System.Windows.Forms.Button();
			this.label7 = new System.Windows.Forms.Label();
			this.label9 = new System.Windows.Forms.Label();
			this.lblTotalFolds = new System.Windows.Forms.Label();
			this.lblTotalInstructions = new System.Windows.Forms.Label();
			((System.ComponentModel.ISupportInitialize)(this.pictureBoxSample)).BeginInit();
			this.SuspendLayout();
			// 
			// pictureBoxSample
			// 
			this.pictureBoxSample.Location = new System.Drawing.Point(12, 12);
			this.pictureBoxSample.Name = "pictureBoxSample";
			this.pictureBoxSample.Size = new System.Drawing.Size(600, 600);
			this.pictureBoxSample.TabIndex = 0;
			this.pictureBoxSample.TabStop = false;
			// 
			// txtOffsetX
			// 
			this.txtOffsetX.Location = new System.Drawing.Point(749, 14);
			this.txtOffsetX.Name = "txtOffsetX";
			this.txtOffsetX.Size = new System.Drawing.Size(57, 20);
			this.txtOffsetX.TabIndex = 1;
			this.txtOffsetX.Text = "0";
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(695, 17);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(48, 13);
			this.label1.TabIndex = 3;
			this.label1.Text = "Offset X:";
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(833, 17);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(17, 13);
			this.label2.TabIndex = 4;
			this.label2.Text = "Y:";
			// 
			// btnRandomOffset
			// 
			this.btnRandomOffset.Location = new System.Drawing.Point(919, 12);
			this.btnRandomOffset.Name = "btnRandomOffset";
			this.btnRandomOffset.Size = new System.Drawing.Size(75, 23);
			this.btnRandomOffset.TabIndex = 5;
			this.btnRandomOffset.Text = "Random";
			this.btnRandomOffset.UseVisualStyleBackColor = true;
			// 
			// btnQuickSave
			// 
			this.btnQuickSave.Location = new System.Drawing.Point(664, 421);
			this.btnQuickSave.Name = "btnQuickSave";
			this.btnQuickSave.Size = new System.Drawing.Size(75, 23);
			this.btnQuickSave.TabIndex = 6;
			this.btnQuickSave.Text = "Quick Save";
			this.btnQuickSave.UseVisualStyleBackColor = true;
			this.btnQuickSave.Click += new System.EventHandler(this.btnQuickSave_Click);
			// 
			// label3
			// 
			this.label3.AutoSize = true;
			this.label3.Location = new System.Drawing.Point(664, 43);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(79, 13);
			this.label3.TabIndex = 7;
			this.label3.Text = "Sample Height:";
			// 
			// txtSampleHeight
			// 
			this.txtSampleHeight.Location = new System.Drawing.Point(749, 40);
			this.txtSampleHeight.Name = "txtSampleHeight";
			this.txtSampleHeight.Size = new System.Drawing.Size(57, 20);
			this.txtSampleHeight.TabIndex = 8;
			this.txtSampleHeight.Text = "400";
			// 
			// txtOffsetY
			// 
			this.txtOffsetY.Location = new System.Drawing.Point(856, 14);
			this.txtOffsetY.Name = "txtOffsetY";
			this.txtOffsetY.Size = new System.Drawing.Size(57, 20);
			this.txtOffsetY.TabIndex = 9;
			this.txtOffsetY.Text = "0";
			// 
			// txtSampleWidth
			// 
			this.txtSampleWidth.Location = new System.Drawing.Point(856, 40);
			this.txtSampleWidth.Name = "txtSampleWidth";
			this.txtSampleWidth.Size = new System.Drawing.Size(57, 20);
			this.txtSampleWidth.TabIndex = 10;
			this.txtSampleWidth.Text = "400";
			// 
			// label4
			// 
			this.label4.AutoSize = true;
			this.label4.Location = new System.Drawing.Point(812, 43);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(38, 13);
			this.label4.TabIndex = 11;
			this.label4.Text = "Width:";
			// 
			// label5
			// 
			this.label5.AutoSize = true;
			this.label5.Location = new System.Drawing.Point(745, 426);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(52, 13);
			this.label5.TabIndex = 12;
			this.label5.Text = "Directory:";
			// 
			// txtQuickSaveDir
			// 
			this.txtQuickSaveDir.Location = new System.Drawing.Point(803, 423);
			this.txtQuickSaveDir.Name = "txtQuickSaveDir";
			this.txtQuickSaveDir.Size = new System.Drawing.Size(153, 20);
			this.txtQuickSaveDir.TabIndex = 13;
			// 
			// btnQuickSaveBrowse
			// 
			this.btnQuickSaveBrowse.Location = new System.Drawing.Point(962, 421);
			this.btnQuickSaveBrowse.Name = "btnQuickSaveBrowse";
			this.btnQuickSaveBrowse.Size = new System.Drawing.Size(28, 23);
			this.btnQuickSaveBrowse.TabIndex = 14;
			this.btnQuickSaveBrowse.Text = "...";
			this.btnQuickSaveBrowse.UseVisualStyleBackColor = true;
			// 
			// btnSaveAs
			// 
			this.btnSaveAs.Location = new System.Drawing.Point(664, 451);
			this.btnSaveAs.Name = "btnSaveAs";
			this.btnSaveAs.Size = new System.Drawing.Size(75, 23);
			this.btnSaveAs.TabIndex = 15;
			this.btnSaveAs.Text = "Save As ...";
			this.btnSaveAs.UseVisualStyleBackColor = true;
			// 
			// btnSample
			// 
			this.btnSample.Location = new System.Drawing.Point(915, 385);
			this.btnSample.Name = "btnSample";
			this.btnSample.Size = new System.Drawing.Size(75, 23);
			this.btnSample.TabIndex = 16;
			this.btnSample.Text = "Get Sample";
			this.btnSample.UseVisualStyleBackColor = true;
			this.btnSample.Click += new System.EventHandler(this.btnSample_Click);
			// 
			// label6
			// 
			this.label6.AutoSize = true;
			this.label6.Location = new System.Drawing.Point(9, 615);
			this.label6.Name = "label6";
			this.label6.Size = new System.Drawing.Size(93, 13);
			this.label6.TabIndex = 17;
			this.label6.Text = "Elapsed Time (ms)";
			// 
			// lblElapsedTime
			// 
			this.lblElapsedTime.AutoSize = true;
			this.lblElapsedTime.Location = new System.Drawing.Point(108, 615);
			this.lblElapsedTime.Name = "lblElapsedTime";
			this.lblElapsedTime.Size = new System.Drawing.Size(56, 13);
			this.lblElapsedTime.TabIndex = 18;
			this.lblElapsedTime.Text = "Time Here";
			// 
			// txtNoiseExpression
			// 
			this.txtNoiseExpression.AcceptsReturn = true;
			this.txtNoiseExpression.AcceptsTab = true;
			this.txtNoiseExpression.Location = new System.Drawing.Point(636, 66);
			this.txtNoiseExpression.Multiline = true;
			this.txtNoiseExpression.Name = "txtNoiseExpression";
			this.txtNoiseExpression.Size = new System.Drawing.Size(360, 284);
			this.txtNoiseExpression.TabIndex = 19;
			this.txtNoiseExpression.Text = "<s:2>simplexBasis(12345);";
			// 
			// btnParseExpression
			// 
			this.btnParseExpression.Location = new System.Drawing.Point(917, 356);
			this.btnParseExpression.Name = "btnParseExpression";
			this.btnParseExpression.Size = new System.Drawing.Size(75, 23);
			this.btnParseExpression.TabIndex = 20;
			this.btnParseExpression.Text = "Display";
			this.btnParseExpression.UseVisualStyleBackColor = true;
			this.btnParseExpression.Click += new System.EventHandler(this.btnParseExpression_Click);
			// 
			// label7
			// 
			this.label7.AutoSize = true;
			this.label7.Location = new System.Drawing.Point(12, 632);
			this.label7.Name = "label7";
			this.label7.Size = new System.Drawing.Size(104, 13);
			this.label7.TabIndex = 21;
			this.label7.Text = "Total Constant Folds";
			// 
			// label9
			// 
			this.label9.AutoSize = true;
			this.label9.Location = new System.Drawing.Point(12, 666);
			this.label9.Name = "label9";
			this.label9.Size = new System.Drawing.Size(88, 13);
			this.label9.TabIndex = 23;
			this.label9.Text = "Total Instructions";
			// 
			// lblTotalFolds
			// 
			this.lblTotalFolds.AutoSize = true;
			this.lblTotalFolds.Location = new System.Drawing.Point(155, 632);
			this.lblTotalFolds.Name = "lblTotalFolds";
			this.lblTotalFolds.Size = new System.Drawing.Size(41, 13);
			this.lblTotalFolds.TabIndex = 25;
			this.lblTotalFolds.Text = "label11";
			// 
			// lblTotalInstructions
			// 
			this.lblTotalInstructions.AutoSize = true;
			this.lblTotalInstructions.Location = new System.Drawing.Point(155, 666);
			this.lblTotalInstructions.Name = "lblTotalInstructions";
			this.lblTotalInstructions.Size = new System.Drawing.Size(41, 13);
			this.lblTotalInstructions.TabIndex = 26;
			this.lblTotalInstructions.Text = "label12";
			// 
			// Form1
			// 
			this.AcceptButton = this.btnParseExpression;
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(1006, 692);
			this.Controls.Add(this.lblTotalInstructions);
			this.Controls.Add(this.lblTotalFolds);
			this.Controls.Add(this.label9);
			this.Controls.Add(this.label7);
			this.Controls.Add(this.btnParseExpression);
			this.Controls.Add(this.txtNoiseExpression);
			this.Controls.Add(this.lblElapsedTime);
			this.Controls.Add(this.label6);
			this.Controls.Add(this.btnSample);
			this.Controls.Add(this.btnSaveAs);
			this.Controls.Add(this.btnQuickSaveBrowse);
			this.Controls.Add(this.txtQuickSaveDir);
			this.Controls.Add(this.label5);
			this.Controls.Add(this.label4);
			this.Controls.Add(this.txtSampleWidth);
			this.Controls.Add(this.txtOffsetY);
			this.Controls.Add(this.txtSampleHeight);
			this.Controls.Add(this.label3);
			this.Controls.Add(this.btnQuickSave);
			this.Controls.Add(this.btnRandomOffset);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.label1);
			this.Controls.Add(this.txtOffsetX);
			this.Controls.Add(this.pictureBoxSample);
			this.Name = "Form1";
			this.Text = "Simplex GUI";
			this.Load += new System.EventHandler(this.Form1_Load);
			((System.ComponentModel.ISupportInitialize)(this.pictureBoxSample)).EndInit();
			this.ResumeLayout(false);
			this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.PictureBox pictureBoxSample;
        private System.Windows.Forms.TextBox txtOffsetX;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button btnRandomOffset;
        private System.Windows.Forms.Button btnQuickSave;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox txtSampleHeight;
        private System.Windows.Forms.TextBox txtOffsetY;
        private System.Windows.Forms.TextBox txtSampleWidth;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox txtQuickSaveDir;
        private System.Windows.Forms.Button btnQuickSaveBrowse;
        private System.Windows.Forms.Button btnSaveAs;
        private System.Windows.Forms.Button btnSample;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label lblElapsedTime;
        private System.Windows.Forms.TextBox txtNoiseExpression;
        private System.Windows.Forms.Button btnParseExpression;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Label lblTotalFolds;
        private System.Windows.Forms.Label lblTotalInstructions;
    }
}

