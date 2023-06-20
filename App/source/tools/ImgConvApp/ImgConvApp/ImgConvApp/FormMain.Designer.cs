using System;
using System.Windows.Forms;
using System.Drawing;
namespace ImgConvApp
{
    partial class FormMain
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
            this.panelCommand = new System.Windows.Forms.Panel();
            this.comboBoxFormat = new System.Windows.Forms.ComboBox();
            this.label1 = new System.Windows.Forms.Label();
            this.buttonExport = new System.Windows.Forms.Button();
            this.buttonLoad = new System.Windows.Forms.Button();
            this.checkBoxBatch = new System.Windows.Forms.CheckBox();
            this.pictureBoxImage = new System.Windows.Forms.PictureBox();
            this.lblOpenImage = new System.Windows.Forms.Label();
            this.buttonFit = new System.Windows.Forms.Button();
            this.panelCommand.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxImage)).BeginInit();
            this.SuspendLayout();
            // 
            // panelCommand
            // 
            this.panelCommand.AutoSize = true;
            this.panelCommand.Controls.Add(this.buttonFit);
            this.panelCommand.Controls.Add(this.comboBoxFormat);
            this.panelCommand.Controls.Add(this.label1);
            this.panelCommand.Controls.Add(this.buttonExport);
            this.panelCommand.Controls.Add(this.buttonLoad);
            this.panelCommand.Controls.Add(this.checkBoxBatch);
            this.panelCommand.Location = new System.Drawing.Point(1, 1);
            this.panelCommand.Name = "panelCommand";
            this.panelCommand.Size = new System.Drawing.Size(661, 53);
            this.panelCommand.TabIndex = 0;
            // 
            // comboBoxFormat
            // 
            this.comboBoxFormat.FormattingEnabled = true;
            this.comboBoxFormat.Items.AddRange(new object[] {
            "6-Bit Color",
            "SPI 1-Bit B&W",
            "SPI 3-Bit Color",
            "1bpp Grayscale",
            "2bpp Grayscale",
            "4bpp Grayscale",
            "8bpp Grayscale",
            "1-Bit Bitmap",
            "2-Bit Bitmap"});
            this.comboBoxFormat.Location = new System.Drawing.Point(376, 18);
            this.comboBoxFormat.Name = "comboBoxFormat";
            this.comboBoxFormat.Size = new System.Drawing.Size(156, 20);
            this.comboBoxFormat.TabIndex = 3;
            this.comboBoxFormat.Text = "6-Bit Color";
            this.comboBoxFormat.SelectedIndexChanged += new System.EventHandler(this.comboBoxFormat_SelectedIndexChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(341, 22);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(29, 12);
            this.label1.TabIndex = 2;
            this.label1.Text = "格式";
            // 
            // buttonExport
            // 
            this.buttonExport.Location = new System.Drawing.Point(185, 4);
            this.buttonExport.Name = "buttonExport";
            this.buttonExport.Size = new System.Drawing.Size(121, 46);
            this.buttonExport.TabIndex = 1;
            this.buttonExport.Text = "导出";
            this.buttonExport.UseVisualStyleBackColor = true;
            this.buttonExport.Click += new System.EventHandler(this.buttonExport_Click);
            // 
            // buttonLoad
            // 
            this.buttonLoad.Location = new System.Drawing.Point(58, 3);
            this.buttonLoad.Name = "buttonLoad";
            this.buttonLoad.Size = new System.Drawing.Size(121, 46);
            this.buttonLoad.TabIndex = 1;
            this.buttonLoad.Text = "加载";
            this.buttonLoad.UseVisualStyleBackColor = true;
            this.buttonLoad.Click += new System.EventHandler(this.buttonLoad_Click);
            // 
            // checkBoxBatch
            // 
            this.checkBoxBatch.AutoSize = true;
            this.checkBoxBatch.Location = new System.Drawing.Point(4, 19);
            this.checkBoxBatch.Name = "checkBoxBatch";
            this.checkBoxBatch.Size = new System.Drawing.Size(48, 16);
            this.checkBoxBatch.TabIndex = 0;
            this.checkBoxBatch.Text = "批量";
            this.checkBoxBatch.UseVisualStyleBackColor = true;
            // 
            // pictureBoxImage
            // 
            this.pictureBoxImage.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.pictureBoxImage.Location = new System.Drawing.Point(1, 54);
            this.pictureBoxImage.Name = "pictureBoxImage";
            this.pictureBoxImage.Size = new System.Drawing.Size(661, 357);
            this.pictureBoxImage.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
            this.pictureBoxImage.TabIndex = 1;
            this.pictureBoxImage.TabStop = false;
            // 
            // lblOpenImage
            // 
            this.lblOpenImage.Anchor = (AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right);
            this.lblOpenImage.Location = new System.Drawing.Point(12, 425);
            this.lblOpenImage.Name = "lblOpenImage";
            this.lblOpenImage.Size = new System.Drawing.Size(0, 12);
            this.lblOpenImage.TabIndex = 2;
            // 
            // buttonFit
            // 
            this.buttonFit.Location = new System.Drawing.Point(554, 17);
            this.buttonFit.Name = "buttonFit";
            this.buttonFit.Size = new System.Drawing.Size(75, 23);
            this.buttonFit.TabIndex = 4;
            this.buttonFit.Text = "Fit";
            this.buttonFit.UseVisualStyleBackColor = true;
            this.buttonFit.Click += new System.EventHandler(this.buttonFit_Click);
            // 
            // FormMain
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(664, 448);
            this.Controls.Add(this.lblOpenImage);
            this.Controls.Add(this.pictureBoxImage);
            this.Controls.Add(this.panelCommand);
            this.Name = "FormMain";
            this.Text = "Image Convert Application";
            this.panelCommand.ResumeLayout(false);
            this.panelCommand.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxImage)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Panel panelCommand;
        private System.Windows.Forms.CheckBox checkBoxBatch;
        private System.Windows.Forms.Button buttonLoad;
        private System.Windows.Forms.Button buttonExport;
        private System.Windows.Forms.ComboBox comboBoxFormat;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.PictureBox pictureBoxImage;
        private System.Windows.Forms.Label lblOpenImage;
        private Button buttonFit;
    }
}

