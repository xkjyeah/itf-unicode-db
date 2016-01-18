namespace UnicodeDbIMESettings
{
    partial class frmSettingsBasic
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
            this.btnOK = new System.Windows.Forms.Button();
            this.btnCancel = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.txtActivationSequence = new System.Windows.Forms.TextBox();
            this.btnCapture = new System.Windows.Forms.Button();
            this.lblSearchCharacter = new System.Windows.Forms.Label();
            this.txtSearchKey = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.txtClipboard = new System.Windows.Forms.TextBox();
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.tableLayoutPanel1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.SuspendLayout();
            // 
            // btnOK
            // 
            this.btnOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.btnOK.Location = new System.Drawing.Point(331, 168);
            this.btnOK.Name = "btnOK";
            this.btnOK.Size = new System.Drawing.Size(95, 28);
            this.btnOK.TabIndex = 7;
            this.btnOK.Text = "&OK";
            this.btnOK.UseVisualStyleBackColor = true;
            this.btnOK.Click += new System.EventHandler(this.btnOK_Click);
            // 
            // btnCancel
            // 
            this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnCancel.Location = new System.Drawing.Point(432, 168);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(95, 28);
            this.btnCancel.TabIndex = 8;
            this.btnCancel.Text = "&Cancel";
            this.btnCancel.UseVisualStyleBackColor = true;
            this.btnCancel.Click += new System.EventHandler(this.btnCancel_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(3, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(172, 13);
            this.label1.TabIndex = 10;
            this.label1.Text = "Unicode input activation sequence";
            // 
            // txtActivationSequence
            // 
            this.txtActivationSequence.Enabled = false;
            this.txtActivationSequence.ImeMode = System.Windows.Forms.ImeMode.Off;
            this.txtActivationSequence.Location = new System.Drawing.Point(209, 3);
            this.txtActivationSequence.Name = "txtActivationSequence";
            this.txtActivationSequence.Size = new System.Drawing.Size(146, 20);
            this.txtActivationSequence.TabIndex = 11;
            // 
            // btnCapture
            // 
            this.btnCapture.Location = new System.Drawing.Point(375, 3);
            this.btnCapture.Name = "btnCapture";
            this.btnCapture.Size = new System.Drawing.Size(129, 31);
            this.btnCapture.TabIndex = 12;
            this.btnCapture.Text = "C&apture...";
            this.btnCapture.UseVisualStyleBackColor = true;
            // 
            // lblSearchCharacter
            // 
            this.lblSearchCharacter.AutoSize = true;
            this.lblSearchCharacter.Location = new System.Drawing.Point(3, 37);
            this.lblSearchCharacter.Name = "lblSearchCharacter";
            this.lblSearchCharacter.Size = new System.Drawing.Size(138, 13);
            this.lblSearchCharacter.TabIndex = 13;
            this.lblSearchCharacter.Text = "&Search activation character";
            // 
            // txtSearchKey
            // 
            this.txtSearchKey.Location = new System.Drawing.Point(209, 40);
            this.txtSearchKey.MaxLength = 1;
            this.txtSearchKey.Name = "txtSearchKey";
            this.txtSearchKey.Size = new System.Drawing.Size(146, 20);
            this.txtSearchKey.TabIndex = 14;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(3, 60);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(199, 13);
            this.label3.TabIndex = 15;
            this.label3.Text = "Clip&board inspection activation character";
            // 
            // txtClipboard
            // 
            this.txtClipboard.Location = new System.Drawing.Point(209, 63);
            this.txtClipboard.MaxLength = 1;
            this.txtClipboard.Name = "txtClipboard";
            this.txtClipboard.Size = new System.Drawing.Size(146, 20);
            this.txtClipboard.TabIndex = 16;
            this.txtClipboard.TextChanged += new System.EventHandler(this.txtClipboard_TextChanged);
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.ColumnCount = 3;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 55.37634F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 44.62366F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 137F));
            this.tableLayoutPanel1.Controls.Add(this.txtClipboard, 1, 2);
            this.tableLayoutPanel1.Controls.Add(this.label3, 0, 2);
            this.tableLayoutPanel1.Controls.Add(this.txtSearchKey, 1, 1);
            this.tableLayoutPanel1.Controls.Add(this.lblSearchCharacter, 0, 1);
            this.tableLayoutPanel1.Controls.Add(this.btnCapture, 2, 0);
            this.tableLayoutPanel1.Controls.Add(this.txtActivationSequence, 1, 0);
            this.tableLayoutPanel1.Controls.Add(this.label1, 0, 0);
            this.tableLayoutPanel1.Location = new System.Drawing.Point(6, 19);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 3;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 61.79775F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 38.20225F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.Size = new System.Drawing.Size(510, 87);
            this.tableLayoutPanel1.TabIndex = 18;
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.tableLayoutPanel1);
            this.groupBox2.Location = new System.Drawing.Point(11, 10);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(523, 119);
            this.groupBox2.TabIndex = 19;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Activation Keys";
            // 
            // frmSettingsAdvanced
            // 
            this.AcceptButton = this.btnOK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.btnCancel;
            this.ClientSize = new System.Drawing.Size(540, 215);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.btnOK);
            this.Name = "frmSettingsAdvanced";
            this.Text = "Unicode DB IME Settings";
            this.tableLayoutPanel1.ResumeLayout(false);
            this.tableLayoutPanel1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button btnOK;
        private System.Windows.Forms.Button btnCancel;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox txtActivationSequence;
        private System.Windows.Forms.Button btnCapture;
        private System.Windows.Forms.Label lblSearchCharacter;
        private System.Windows.Forms.TextBox txtSearchKey;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox txtClipboard;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.GroupBox groupBox2;
    }
}

