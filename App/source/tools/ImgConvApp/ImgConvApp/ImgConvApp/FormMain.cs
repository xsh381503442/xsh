using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.IO;
using System.Windows.Forms;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;


namespace ImgConvApp
{
    public partial class FormMain : Form
    {
        private OpenFileDialog oDlg;
        private SaveFileDialog sDlg;

        private Bitmap LoadedImage;
        private Bitmap FilteredImage;

        public FormMain()
        {
            InitializeComponent();

            oDlg = new OpenFileDialog(); // Open Dialog Initialization
            oDlg.Title = "Open an Image File";
            oDlg.RestoreDirectory = true;
            oDlg.InitialDirectory = "C:\\";
            oDlg.FilterIndex = 1;
          //  oDlg.Filter = "All Files |*.*|jpg Files (*.jpg)|*.jpg|gif Files (*.gif)|*.gif|png Files (*.png)|*.png |bmp Files (*.bmp)|*.bmp";
            oDlg.Filter = "All Image Files (*.png,*.gif,*.tif,*.ico,*.bmp ,*.jpg)|*.png;*.gif;*.tif;*.ico;*.bmp;*.jpg|Windows Bitmap (*.bmp)|*.bmp|Joint Photographic Experts Group (*.jpg)|*.jpg|Windows Icon (*.ico)|*.ico|Tagged Image File Format (*.tif)|*.tif|CompuServe Graphics Interchange (*.gif)|*.gif|Portable Network Graphics (*.png)|*.png|All Files (*.*)|*.*";

            /*************************/
            sDlg = new SaveFileDialog(); // Save Dialog Initialization
            sDlg.Title = "Export Image";
            sDlg.RestoreDirectory = true;
            sDlg.InitialDirectory = "C:\\";
            sDlg.FilterIndex = 1;
            sDlg.Filter = "C Header file|*.h|HEX file|*.hex|MDCIMG file|*.mdcimg";
        }

        private void buttonLoad_Click(object sender, EventArgs e)
        {
            this.oDlg.ShowHelp = true;
            if (this.oDlg.ShowDialog() == DialogResult.OK)
            {
                IDisposable loadedImage = this.LoadedImage;
                if (loadedImage != null)
                {
                    loadedImage.Dispose();
                }

                //if (!checkBoxBatch.Checked)
                {
                    try
                    {
                        LoadedImage = new Bitmap(oDlg.FileName);
                        lblOpenImage.Text = oDlg.FileName;
                        DisplayFilter();
                    }
                    catch (Exception)
                    {
                        this.lblOpenImage.Text = "Can't read " + oDlg.FileName;
                    }
                }
            }
        }

        private void comboBoxFormat_SelectedIndexChanged(object sender, EventArgs e)
        {
            DisplayFilter();
        }

        private void DisplayFilter()
        {
            if (this.LoadedImage != null)
            {
                IDisposable filteredImage = this.FilteredImage;
                if (filteredImage != null)
                {
                    filteredImage.Dispose();
                }
                try
                {
                    this.FilteredImage = new Bitmap(this.LoadedImage.Width, this.LoadedImage.Height, PixelFormat.Format32bppArgb);
                    if (this.comboBoxFormat.Text == "6-Bit Color")
                    {
                        for (int i = 0; i < this.LoadedImage.Height; i++)
                        {
                            for (int j = 0; j < this.LoadedImage.Width; j++)
                            {
                                Color pixel = this.LoadedImage.GetPixel(j, i);
                                uint num = (uint)(pixel.ToArgb() & 12632256);
                                uint num2 = (pixel.A <= 191) ? 127u : 255u;
                                num2 = ((pixel.A <= 127) ? 63u : num2);
                                num2 = ((pixel.A <= 63) ? 0u : num2);
                                num |= num2 << 24;
                                Color color = Color.FromArgb((int)num);
                                this.FilteredImage.SetPixel(j, i, color);
                            }
                        }
                    }
                    else if (this.comboBoxFormat.Text == "SPI 3-Bit Color")
                    {
                        for (int k = 0; k < this.LoadedImage.Height; k++)
                        {
                            for (int l = 0; l < this.LoadedImage.Width; l++)
                            {
                                Color pixel2 = this.LoadedImage.GetPixel(l, k);
                                uint num3 = (uint)(pixel2.ToArgb() & 8421504);
                                uint num4 = (pixel2.A <= 127) ? 0u : 255u;
                                num3 |= num4 << 24;
                                Color color2 = Color.FromArgb((int)num3);
                                this.FilteredImage.SetPixel(l, k, color2);
                            }
                        }
                    }
                    else if (this.comboBoxFormat.Text == "SPI 1-Bit B&W")
                    {
                        for (int m = 0; m < this.LoadedImage.Height; m++)
                        {
                            for (int n = 0; n < this.LoadedImage.Width; n++)
                            {
                                Color pixel3 = this.LoadedImage.GetPixel(n, m);
                                float value = (float)(0.299 * (double)((float)pixel3.R) + 0.587 * (double)((float)pixel3.G) + 0.114 * (double)((float)pixel3.B));
                                byte b = (byte)(Convert.ToByte(value) & 0x80);
                                byte b2 = (byte)((pixel3.A <= 127) ? 0 : 0xFF);
                                int arg_2B6_0 = (int)b2;
                                byte expr_2B3 = b;
                                Color color3 = Color.FromArgb(arg_2B6_0, (int)expr_2B3, (int)expr_2B3, (int)b);
                                this.FilteredImage.SetPixel(n, m, color3);
                            }
                        }
                    }
                    else if (this.comboBoxFormat.Text == "1bpp Grayscale")
                    {
                        for (int num5 = 0; num5 < this.LoadedImage.Height; num5++)
                        {
                            for (int num6 = 0; num6 < this.LoadedImage.Width; num6++)
                            {
                                Color pixel4 = this.LoadedImage.GetPixel(num6, num5);
                                float value2 = (float)(0.299 * (double)((float)pixel4.R) + 0.587 * (double)((float)pixel4.G) + 0.114 * (double)((float)pixel4.B));
                                byte b3 = (byte)(Convert.ToByte(value2) & 0x80);
                                int arg_399_0 = 255;
                                byte expr_396 = b3;
                                Color color4 = Color.FromArgb(arg_399_0, (int)expr_396, (int)expr_396, (int)b3);
                                this.FilteredImage.SetPixel(num6, num5, color4);
                            }
                        }
                    }
                    else if (this.comboBoxFormat.Text == "2bpp Grayscale")
                    {
                        for (int num7 = 0; num7 < this.LoadedImage.Height; num7++)
                        {
                            for (int num8 = 0; num8 < this.LoadedImage.Width; num8++)
                            {
                                Color pixel5 = this.LoadedImage.GetPixel(num8, num7);
                                float value3 = (float)(0.299 * (double)((float)pixel5.R) + 0.587 * (double)((float)pixel5.G) + 0.114 * (double)((float)pixel5.B));
                                byte b4 = (byte)(Convert.ToByte(value3) & 0xC0);
                                int arg_47C_0 = 255;
                                byte expr_479 = b4;
                                Color color5 = Color.FromArgb(arg_47C_0, (int)expr_479, (int)expr_479, (int)b4);
                                this.FilteredImage.SetPixel(num8, num7, color5);
                            }
                        }
                    }
                    else if (this.comboBoxFormat.Text == "4bpp Grayscale")
                    {
                        for (int num9 = 0; num9 < this.LoadedImage.Height; num9++)
                        {
                            for (int num10 = 0; num10 < this.LoadedImage.Width; num10++)
                            {
                                Color pixel6 = this.LoadedImage.GetPixel(num10, num9);
                                float value4 = (float)(0.299 * (double)((float)pixel6.R) + 0.587 * (double)((float)pixel6.G) + 0.114 * (double)((float)pixel6.B));
                                byte b5 = (byte)(Convert.ToByte(value4) & 0xF0);
                                int arg_55F_0 = 255;
                                byte expr_55C = b5;
                                Color color6 = Color.FromArgb(arg_55F_0, (int)expr_55C, (int)expr_55C, (int)b5);
                                this.FilteredImage.SetPixel(num10, num9, color6);
                            }
                        }
                    }
                    else if (this.comboBoxFormat.Text == "8bpp Grayscale")
                    {
                        for (int num11 = 0; num11 < this.LoadedImage.Height; num11++)
                        {
                            for (int num12 = 0; num12 < this.LoadedImage.Width; num12++)
                            {
                                Color pixel7 = this.LoadedImage.GetPixel(num12, num11);
                                float value5 = (float)(0.299 * (double)((float)pixel7.R) + 0.587 * (double)((float)pixel7.G) + 0.114 * (double)((float)pixel7.B));
                                byte b6 = Convert.ToByte(value5);
                                int arg_63C_0 = 255;
                                byte expr_639 = b6;
                                Color color7 = Color.FromArgb(arg_63C_0, (int)expr_639, (int)expr_639, (int)b6);
                                this.FilteredImage.SetPixel(num12, num11, color7);
                            }
                        }
                    }
                    else if (this.comboBoxFormat.Text == "1-Bit Bitmap")
                    {
                        for (int num13 = 0; num13 < this.LoadedImage.Height; num13++)
                        {
                            for (int num14 = 0; num14 < this.LoadedImage.Width; num14++)
                            {
                                Color pixel8 = this.LoadedImage.GetPixel(num14, num13);
                                float value6 = (float)(0.299 * (double)((float)pixel8.R) + 0.587 * (double)((float)pixel8.G) + 0.114 * (double)((float)pixel8.B));
                                byte b7 = (byte)(Convert.ToByte(value6) & 0x80);
                                int arg_715_0 = 255;
                                byte expr_712 = b7;
                                Color color8 = Color.FromArgb(arg_715_0, (int)expr_712, (int)expr_712, (int)b7);
                                this.FilteredImage.SetPixel(num14, num13, color8);
                            }
                        }
                    }
                    else if (this.comboBoxFormat.Text == "2-Bit Bitmap")
                    {
                        for (int num15 = 0; num15 < this.LoadedImage.Height; num15++)
                        {
                            for (int num16 = 0; num16 < this.LoadedImage.Width; num16++)
                            {
                                Color pixel9 = this.LoadedImage.GetPixel(num16, num15);
                                float value7 = (float)(0.299 * (double)((float)pixel9.R) + 0.587 * (double)((float)pixel9.G) + 0.114 * (double)((float)pixel9.B));
                                byte b8 = (byte)(Convert.ToByte(value7) & 0xC0);
                                int arg_7EC_0 = 255;
                                byte expr_7E9 = b8;
                                Color color9 = Color.FromArgb(arg_7EC_0, (int)expr_7E9, (int)expr_7E9, (int)b8);
                                this.FilteredImage.SetPixel(num16, num15, color9);
                            }
                        }
                    }
                    this.pictureBoxImage.Image = this.FilteredImage;
                }
                catch (Exception)
                {
                    IDisposable filteredImage2 = this.FilteredImage;
                    if (filteredImage2 != null)
                    {
                        filteredImage2.Dispose();
                    }
                    this.FilteredImage = new Bitmap(oDlg.FileName);
                    this.pictureBoxImage.Image = this.FilteredImage;
                    this.lblOpenImage.Text = "DISPLAYED IMAGE IS NOT FILTERED " + oDlg.FileName;
                }
            }
        }

        private void buttonFit_Click(object sender, EventArgs e)
        {
            pictureBoxImage.SizeMode = PictureBoxSizeMode.Zoom;
        }

        private void buttonExport_Click(object sender, EventArgs e)
        {
            if (LoadedImage != null)
            {
                sDlg.ShowDialog();
            }
            if (this.sDlg.FileName != "")
            {
                int filterIndex = this.sDlg.FilterIndex;
                if (filterIndex != 1)
                {
                    if (filterIndex != 2)
                    {
                        if (filterIndex == 3)
                        {
                            this.saveBINFile();
                        }
                    }
                    else
                    {
                        this.saveHEXFile();
                    }
                }
                else
                {
                    this.saveHFile();
                }
            }
        }

        private void saveHEXFile()
        {
            StreamWriter streamWriter = new StreamWriter(this.sDlg.OpenFile());
            if (this.comboBoxFormat.Text == "6-Bit Color")
            {
                char[] array = new char[]
				{
					'0',
					'x',
					'0',
					'0'
				};
                byte value = (byte)((ushort)this.LoadedImage.Width & 255);
                streamWriter.Write(Convert.ToString(value, 16).PadLeft(2, '0'));
                value = (byte)((ushort)this.LoadedImage.Width >> 8 & 255);
                streamWriter.Write(Convert.ToString(value, 16).PadLeft(2, '0'));
                value = (byte)((ushort)this.LoadedImage.Height & 255);
                streamWriter.Write(Convert.ToString(value, 16).PadLeft(2, '0'));
                value = (byte)((ushort)this.LoadedImage.Height >> 8 & 255);
                streamWriter.Write(Convert.ToString(value, 16).PadLeft(2, '0'));
                value = (byte)((ushort)this.LoadedImage.Width & 255);
                streamWriter.Write(Convert.ToString(value, 16).PadLeft(2, '0'));
                value = (byte)((ushort)this.LoadedImage.Width >> 8 & 255);
                streamWriter.Write(Convert.ToString(value, 16).PadLeft(2, '0'));
                streamWriter.Write(Convert.ToString(8).PadLeft(2, '0'));
                streamWriter.Write(Convert.ToString(0).PadLeft(2, '0'));
                streamWriter.Write("0C000000");
                for (int i = 0; i < this.LoadedImage.Height; i++)
                {
                    for (int j = 0; j < this.LoadedImage.Width; j++)
                    {
                        Color pixel = this.LoadedImage.GetPixel(j, i);
                        this.FormatJDI(pixel.A, pixel.R, pixel.G, pixel.B, array);
                        streamWriter.Write(array, 2, 2);
                    }
                }
            }
            else if (this.comboBoxFormat.Text == "SPI 3-Bit Color")
            {
                bool flag = false;
                char[] array2 = new char[]
				{
					'0'
				};
                char[] array3 = new char[]
				{
					'0'
				};
                char[] array4 = new char[]
				{
					'0',
					'x',
					'0',
					'0'
				};
                byte value2 = (byte)((ushort)this.LoadedImage.Width & 255);
                streamWriter.Write(Convert.ToString(value2, 16).PadLeft(2, '0'));
                value2 = (byte)((ushort)this.LoadedImage.Width >> 8 & 255);
                streamWriter.Write(Convert.ToString(value2, 16).PadLeft(2, '0'));
                value2 = (byte)((ushort)this.LoadedImage.Height & 255);
                streamWriter.Write(Convert.ToString(value2, 16).PadLeft(2, '0'));
                value2 = (byte)((ushort)this.LoadedImage.Height >> 8 & 255);
                streamWriter.Write(Convert.ToString(value2, 16).PadLeft(2, '0'));
                value2 = (byte)((ushort)this.LoadedImage.Width & 255);
                streamWriter.Write(Convert.ToString(value2, 16).PadLeft(2, '0'));
                value2 = (byte)((ushort)this.LoadedImage.Width >> 8 & 255);
                streamWriter.Write(Convert.ToString(value2, 16).PadLeft(2, '0'));
                streamWriter.Write(Convert.ToString(7).PadLeft(2, '0'));
                streamWriter.Write(Convert.ToString(0).PadLeft(2, '0'));
                streamWriter.Write(Convert.ToString(12, 32).PadLeft(4, '0'));
                for (int k = 0; k < this.LoadedImage.Height; k++)
                {
                    for (int l = 0; l < this.LoadedImage.Width; l++)
                    {
                        Color pixel2 = this.LoadedImage.GetPixel(l, k);
                        if (flag)
                        {
                            this.FormatSharpColor(pixel2.A, pixel2.R, pixel2.G, pixel2.B, array2);
                            array4[2] = array2[0];
                            array4[3] = array3[0];
                            streamWriter.Write(array4, 2, 2);
                            flag = false;
                        }
                        else
                        {
                            this.FormatSharpColor(pixel2.A, pixel2.R, pixel2.G, pixel2.B, array3);
                            flag = true;
                            if (l == this.LoadedImage.Width - 1)
                            {
                                array4[2] = '0';
                                array4[3] = array3[0];
                                streamWriter.Write(array4, 2, 2);
                                flag = false;
                            }
                        }
                    }
                }
            }
            else if (this.comboBoxFormat.Text == "SPI 1-Bit B&W")
            {
                char[] array5 = new char[]
				{
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0'
				};
                char[] array6 = new char[]
				{
					'0',
					'x',
					'0',
					'0'
				};
                uint num = 0u;
                byte value3 = (byte)((ushort)this.LoadedImage.Width & 255);
                streamWriter.Write(Convert.ToString(value3, 16).PadLeft(2, '0'));
                value3 = (byte)((ushort)this.LoadedImage.Width >> 8 & 255);
                streamWriter.Write(Convert.ToString(value3, 16).PadLeft(2, '0'));
                value3 = (byte)((ushort)this.LoadedImage.Height & 255);
                streamWriter.Write(Convert.ToString(value3, 16).PadLeft(2, '0'));
                value3 = (byte)((ushort)this.LoadedImage.Height >> 8 & 255);
                streamWriter.Write(Convert.ToString(value3, 16).PadLeft(2, '0'));
                value3 = (byte)((ushort)this.LoadedImage.Width & 255);
                streamWriter.Write(Convert.ToString(value3, 16).PadLeft(2, '0'));
                value3 = (byte)((ushort)this.LoadedImage.Width >> 8 & 255);
                streamWriter.Write(Convert.ToString(value3, 16).PadLeft(2, '0'));
                streamWriter.Write(Convert.ToString(6).PadLeft(2, '0'));
                streamWriter.Write(Convert.ToString(0).PadLeft(2, '0'));
                streamWriter.Write("0C000000");
                for (int m = 0; m < this.LoadedImage.Height; m++)
                {
                    for (int n = 0; n < this.LoadedImage.Width; n++)
                    {
                        Color pixel3 = this.LoadedImage.GetPixel(n, m);
                        float num2 = (float)(0.299 * (double)((float)pixel3.R) + 0.587 * (double)((float)pixel3.G) + 0.114 * (double)((float)pixel3.B));
                        if ((double)num2 >= 128.0)
                        {
                            array5[(int)(num * 2u)] = '\u0001';
                        }
                        else
                        {
                            array5[(int)(num * 2u)] = '\0';
                        }
                        if (pixel3.A >= 128)
                        {
                            array5[(int)(num * 2u + 1u)] = '\u0001';
                        }
                        else
                        {
                            array5[(int)(num * 2u + 1u)] = '\0';
                        }
                        if (num < 3u)
                        {
                            num += 1u;
                        }
                        else
                        {
                            this.FormatSharpBW(array5, array6);
                            streamWriter.Write(array6, 2, 2);
                            for (int num3 = 0; num3 < 8; num3++)
                            {
                                array5[num3] = '\0';
                            }
                            num = 0u;
                        }
                        if (n == this.LoadedImage.Width - 1 && num != 0u)
                        {
                            this.FormatSharpBW(array5, array6);
                            streamWriter.Write(array6, 2, 2);
                            for (int num4 = 0; num4 < 8; num4++)
                            {
                                array5[num4] = '\0';
                            }
                            num = 0u;
                        }
                    }
                }
            }
            else if (this.comboBoxFormat.Text == "1bpp Grayscale")
            {
                char[] array7 = new char[]
				{
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0'
				};
                char[] array8 = new char[]
				{
					'0',
					'x',
					'0',
					'0'
				};
                uint num5 = 0u;
                byte value4 = (byte)((ushort)this.LoadedImage.Width & 255);
                streamWriter.Write(Convert.ToString(value4, 16).PadLeft(2, '0'));
                value4 = (byte)((ushort)this.LoadedImage.Width >> 8 & 255);
                streamWriter.Write(Convert.ToString(value4, 16).PadLeft(2, '0'));
                value4 = (byte)((ushort)this.LoadedImage.Height & 255);
                streamWriter.Write(Convert.ToString(value4, 16).PadLeft(2, '0'));
                value4 = (byte)((ushort)this.LoadedImage.Height >> 8 & 255);
                streamWriter.Write(Convert.ToString(value4, 16).PadLeft(2, '0'));
                value4 = (byte)((ushort)this.LoadedImage.Width & 255);
                streamWriter.Write(Convert.ToString(value4, 16).PadLeft(2, '0'));
                value4 = (byte)((ushort)this.LoadedImage.Width >> 8 & 255);
                streamWriter.Write(Convert.ToString(value4, 16).PadLeft(2, '0'));
                streamWriter.Write(Convert.ToString(2).PadLeft(2, '0'));
                streamWriter.Write(Convert.ToString(0).PadLeft(2, '0'));
                streamWriter.Write("0C000000");
                for (int num6 = 0; num6 < this.LoadedImage.Height; num6++)
                {
                    for (int num7 = 0; num7 < this.LoadedImage.Width; num7++)
                    {
                        Color pixel4 = this.LoadedImage.GetPixel(num7, num6);
                        float num8 = (float)(0.299 * (double)((float)pixel4.R) + 0.587 * (double)((float)pixel4.G) + 0.114 * (double)((float)pixel4.B));
                        if ((double)num8 >= 128.0)
                        {
                            array7[(int)num5] = '\u0001';
                        }
                        else
                        {
                            array7[(int)num5] = '\0';
                        }
                        if (num5 < 7u)
                        {
                            num5 += 1u;
                        }
                        else
                        {
                            this.FormatSharpBW(array7, array8);
                            streamWriter.Write(array8, 2, 2);
                            for (int num9 = 0; num9 < 8; num9++)
                            {
                                array7[num9] = '\0';
                            }
                            num5 = 0u;
                        }
                        if (num7 == this.LoadedImage.Width - 1 && num5 != 0u)
                        {
                            this.FormatSharpBW(array7, array8);
                            streamWriter.Write(array8, 2, 2);
                            for (int num10 = 0; num10 < 8; num10++)
                            {
                                array7[num10] = '\0';
                            }
                            num5 = 0u;
                        }
                    }
                }
            }
            else if (this.comboBoxFormat.Text == "2bpp Grayscale")
            {
                char[] array9 = new char[]
				{
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0'
				};
                char[] array10 = new char[]
				{
					'0',
					'x',
					'0',
					'0'
				};
                uint num11 = 0u;
                byte value5 = (byte)((ushort)this.LoadedImage.Width & 255);
                streamWriter.Write(Convert.ToString(value5, 16).PadLeft(2, '0'));
                value5 = (byte)((ushort)this.LoadedImage.Width >> 8 & 255);
                streamWriter.Write(Convert.ToString(value5, 16).PadLeft(2, '0'));
                value5 = (byte)((ushort)this.LoadedImage.Height & 255);
                streamWriter.Write(Convert.ToString(value5, 16).PadLeft(2, '0'));
                value5 = (byte)((ushort)this.LoadedImage.Height >> 8 & 255);
                streamWriter.Write(Convert.ToString(value5, 16).PadLeft(2, '0'));
                value5 = (byte)((ushort)this.LoadedImage.Width & 255);
                streamWriter.Write(Convert.ToString(value5, 16).PadLeft(2, '0'));
                value5 = (byte)((ushort)this.LoadedImage.Width >> 8 & 255);
                streamWriter.Write(Convert.ToString(value5, 16).PadLeft(2, '0'));
                streamWriter.Write(Convert.ToString(3).PadLeft(2, '0'));
                streamWriter.Write(Convert.ToString(0).PadLeft(2, '0'));
                streamWriter.Write("0C000000");
                for (int num12 = 0; num12 < this.LoadedImage.Height; num12++)
                {
                    for (int num13 = 0; num13 < this.LoadedImage.Width; num13++)
                    {
                        Color pixel5 = this.LoadedImage.GetPixel(num13, num12);
                        float value6 = (float)(0.299 * (double)((float)pixel5.R) + 0.587 * (double)((float)pixel5.G) + 0.114 * (double)((float)pixel5.B));
                        uint num14 = Convert.ToUInt32(value6);
                        if (num14 >= 128u)
                        {
                            array9[(int)(num11 * 2u + 1u)] = '\u0001';
                        }
                        else
                        {
                            array9[(int)(num11 * 2u + 1u)] = '\0';
                        }
                        num14 &= 127u;
                        if (num14 >= 64u)
                        {
                            array9[(int)(num11 * 2u)] = '\u0001';
                        }
                        else
                        {
                            array9[(int)(num11 * 2u)] = '\0';
                        }
                        if (num11 < 3u)
                        {
                            num11 += 1u;
                        }
                        else
                        {
                            this.FormatSharpBW(array9, array10);
                            streamWriter.Write(array10, 2, 2);
                            for (int num15 = 0; num15 < 8; num15++)
                            {
                                array9[num15] = '\0';
                            }
                            num11 = 0u;
                        }
                        if (num13 == this.LoadedImage.Width - 1 && num11 != 0u)
                        {
                            this.FormatSharpBW(array9, array10);
                            streamWriter.Write(array10, 2, 2);
                            for (int num16 = 0; num16 < 8; num16++)
                            {
                                array9[num16] = '\0';
                            }
                            num11 = 0u;
                        }
                    }
                }
            }
            else if (this.comboBoxFormat.Text == "4bpp Grayscale")
            {
                char[] array11 = new char[]
				{
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0'
				};
                char[] array12 = new char[]
				{
					'0',
					'x',
					'0',
					'0'
				};
                uint num17 = 0u;
                byte value7 = (byte)((ushort)this.LoadedImage.Width & 255);
                streamWriter.Write(Convert.ToString(value7, 16).PadLeft(2, '0'));
                value7 = (byte)((ushort)this.LoadedImage.Width >> 8 & 255);
                streamWriter.Write(Convert.ToString(value7, 16).PadLeft(2, '0'));
                value7 = (byte)((ushort)this.LoadedImage.Height & 255);
                streamWriter.Write(Convert.ToString(value7, 16).PadLeft(2, '0'));
                value7 = (byte)((ushort)this.LoadedImage.Height >> 8 & 255);
                streamWriter.Write(Convert.ToString(value7, 16).PadLeft(2, '0'));
                value7 = (byte)((ushort)this.LoadedImage.Width & 255);
                streamWriter.Write(Convert.ToString(value7, 16).PadLeft(2, '0'));
                value7 = (byte)((ushort)this.LoadedImage.Width >> 8 & 255);
                streamWriter.Write(Convert.ToString(value7, 16).PadLeft(2, '0'));
                streamWriter.Write(Convert.ToString(4).PadLeft(2, '0'));
                streamWriter.Write(Convert.ToString(0).PadLeft(2, '0'));
                streamWriter.Write("0C000000");
                for (int num18 = 0; num18 < this.LoadedImage.Height; num18++)
                {
                    for (int num19 = 0; num19 < this.LoadedImage.Width; num19++)
                    {
                        Color pixel6 = this.LoadedImage.GetPixel(num19, num18);
                        float value8 = (float)(0.299 * (double)((float)pixel6.R) + 0.587 * (double)((float)pixel6.G) + 0.114 * (double)((float)pixel6.B));
                        uint num20 = Convert.ToUInt32(value8);
                        if (num20 >= 128u)
                        {
                            array11[(int)(num17 * 4u + 3u)] = '\u0001';
                        }
                        else
                        {
                            array11[(int)(num17 * 4u + 3u)] = '\0';
                        }
                        num20 &= 127u;
                        if (num20 >= 64u)
                        {
                            array11[(int)(num17 * 4u + 2u)] = '\u0001';
                        }
                        else
                        {
                            array11[(int)(num17 * 4u + 2u)] = '\0';
                        }
                        num20 &= 63u;
                        if (num20 >= 32u)
                        {
                            array11[(int)(num17 * 4u + 1u)] = '\u0001';
                        }
                        else
                        {
                            array11[(int)(num17 * 4u + 1u)] = '\0';
                        }
                        num20 &= 31u;
                        if (num20 >= 16u)
                        {
                            array11[(int)(num17 * 4u)] = '\u0001';
                        }
                        else
                        {
                            array11[(int)(num17 * 4u)] = '\0';
                        }
                        if (num17 < 1u)
                        {
                            num17 += 1u;
                        }
                        else
                        {
                            this.FormatSharpBW(array11, array12);
                            streamWriter.Write(array12, 2, 2);
                            for (int num21 = 0; num21 < 8; num21++)
                            {
                                array11[num21] = '\0';
                            }
                            num17 = 0u;
                        }
                        if (num19 == this.LoadedImage.Width - 1 && num17 != 0u)
                        {
                            this.FormatSharpBW(array11, array12);
                            streamWriter.Write(array12, 2, 2);
                            for (int num22 = 0; num22 < 8; num22++)
                            {
                                array11[num22] = '\0';
                            }
                            num17 = 0u;
                        }
                    }
                }
            }
            else if (this.comboBoxFormat.Text == "8bpp Grayscale")
            {
                char[] array13 = new char[]
				{
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0'
				};
                char[] array14 = new char[]
				{
					'0',
					'x',
					'0',
					'0'
				};
                byte value9 = (byte)((ushort)this.LoadedImage.Width & 255);
                streamWriter.Write(Convert.ToString(value9, 16).PadLeft(2, '0'));
                value9 = (byte)((ushort)this.LoadedImage.Width >> 8 & 255);
                streamWriter.Write(Convert.ToString(value9, 16).PadLeft(2, '0'));
                value9 = (byte)((ushort)this.LoadedImage.Height & 255);
                streamWriter.Write(Convert.ToString(value9, 16).PadLeft(2, '0'));
                value9 = (byte)((ushort)this.LoadedImage.Height >> 8 & 255);
                streamWriter.Write(Convert.ToString(value9, 16).PadLeft(2, '0'));
                value9 = (byte)((ushort)this.LoadedImage.Width & 255);
                streamWriter.Write(Convert.ToString(value9, 16).PadLeft(2, '0'));
                value9 = (byte)((ushort)this.LoadedImage.Width >> 8 & 255);
                streamWriter.Write(Convert.ToString(value9, 16).PadLeft(2, '0'));
                streamWriter.Write(Convert.ToString(5).PadLeft(2, '0'));
                streamWriter.Write(Convert.ToString(0).PadLeft(2, '0'));
                streamWriter.Write("0C000000");
                for (int num23 = 0; num23 < this.LoadedImage.Height; num23++)
                {
                    for (int num24 = 0; num24 < this.LoadedImage.Width; num24++)
                    {
                        Color pixel7 = this.LoadedImage.GetPixel(num24, num23);
                        float value10 = (float)(0.299 * (double)((float)pixel7.R) + 0.587 * (double)((float)pixel7.G) + 0.114 * (double)((float)pixel7.B));
                        uint num25 = Convert.ToUInt32(value10);
                        if (num25 >= 128u)
                        {
                            array13[7] = '\u0001';
                        }
                        else
                        {
                            array13[7] = '\0';
                        }
                        num25 &= 127u;
                        if (num25 >= 64u)
                        {
                            array13[6] = '\u0001';
                        }
                        else
                        {
                            array13[6] = '\0';
                        }
                        num25 &= 63u;
                        if (num25 >= 32u)
                        {
                            array13[5] = '\u0001';
                        }
                        else
                        {
                            array13[5] = '\0';
                        }
                        num25 &= 31u;
                        if (num25 >= 16u)
                        {
                            array13[4] = '\u0001';
                        }
                        else
                        {
                            array13[4] = '\0';
                        }
                        num25 &= 15u;
                        if (num25 >= 8u)
                        {
                            array13[3] = '\u0001';
                        }
                        else
                        {
                            array13[3] = '\0';
                        }
                        num25 &= 7u;
                        if (num25 >= 4u)
                        {
                            array13[2] = '\u0001';
                        }
                        else
                        {
                            array13[2] = '\0';
                        }
                        num25 &= 3u;
                        if (num25 >= 2u)
                        {
                            array13[1] = '\u0001';
                        }
                        else
                        {
                            array13[1] = '\0';
                        }
                        num25 &= 1u;
                        if (num25 != 0u)
                        {
                            array13[0] = '\u0001';
                        }
                        else
                        {
                            array13[0] = '\0';
                        }
                        this.FormatSharpBW(array13, array14);
                        streamWriter.Write(array14, 2, 2);
                    }
                }
            }
            else if (this.comboBoxFormat.Text == "1-Bit Bitmap")
            {
                char[] array15 = new char[]
				{
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0'
				};
                char[] array16 = new char[]
				{
					'0',
					'x',
					'0',
					'0'
				};
                uint num26 = 0u;
                byte value11 = (byte)((ushort)this.LoadedImage.Width & 255);
                streamWriter.Write(Convert.ToString(value11, 16).PadLeft(2, '0'));
                value11 = (byte)((ushort)this.LoadedImage.Width >> 8 & 255);
                streamWriter.Write(Convert.ToString(value11, 16).PadLeft(2, '0'));
                value11 = (byte)((ushort)this.LoadedImage.Height & 255);
                streamWriter.Write(Convert.ToString(value11, 16).PadLeft(2, '0'));
                value11 = (byte)((ushort)this.LoadedImage.Height >> 8 & 255);
                streamWriter.Write(Convert.ToString(value11, 16).PadLeft(2, '0'));
                value11 = (byte)((ushort)this.LoadedImage.Width & 255);
                streamWriter.Write(Convert.ToString(value11, 16).PadLeft(2, '0'));
                value11 = (byte)((ushort)this.LoadedImage.Width >> 8 & 255);
                streamWriter.Write(Convert.ToString(value11, 16).PadLeft(2, '0'));
                streamWriter.Write(Convert.ToString(0).PadLeft(2, '0'));
                streamWriter.Write(Convert.ToString(0).PadLeft(2, '0'));
                streamWriter.Write("0C000000");
                for (int num27 = 0; num27 < this.LoadedImage.Height; num27++)
                {
                    for (int num28 = 0; num28 < this.LoadedImage.Width; num28++)
                    {
                        Color pixel8 = this.LoadedImage.GetPixel(num28, num27);
                        float num29 = (float)(0.299 * (double)((float)pixel8.R) + 0.587 * (double)((float)pixel8.G) + 0.114 * (double)((float)pixel8.B));
                        if ((double)num29 >= 128.0)
                        {
                            array15[(int)num26] = '\u0001';
                        }
                        else
                        {
                            array15[(int)num26] = '\0';
                        }
                        if (num26 < 7u)
                        {
                            num26 += 1u;
                        }
                        else
                        {
                            this.FormatSharpBW(array15, array16);
                            streamWriter.Write(array16, 2, 2);
                            for (int num30 = 0; num30 < 8; num30++)
                            {
                                array15[num30] = '\0';
                            }
                            num26 = 0u;
                        }
                        if (num28 == this.LoadedImage.Width - 1 && num26 != 0u)
                        {
                            this.FormatSharpBW(array15, array16);
                            streamWriter.Write(array16, 2, 2);
                            for (int num31 = 0; num31 < 8; num31++)
                            {
                                array15[num31] = '\0';
                            }
                            num26 = 0u;
                        }
                    }
                }
            }
            else if (this.comboBoxFormat.Text == "2-Bit Bitmap")
            {
                char[] array17 = new char[]
				{
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0'
				};
                char[] array18 = new char[]
				{
					'0',
					'x',
					'0',
					'0'
				};
                uint num32 = 0u;
                byte value12 = (byte)((ushort)this.LoadedImage.Width & 255);
                streamWriter.Write(Convert.ToString(value12, 16).PadLeft(2, '0'));
                value12 = (byte)((ushort)this.LoadedImage.Width >> 8 & 255);
                streamWriter.Write(Convert.ToString(value12, 16).PadLeft(2, '0'));
                value12 = (byte)((ushort)this.LoadedImage.Height & 255);
                streamWriter.Write(Convert.ToString(value12, 16).PadLeft(2, '0'));
                value12 = (byte)((ushort)this.LoadedImage.Height >> 8 & 255);
                streamWriter.Write(Convert.ToString(value12, 16).PadLeft(2, '0'));
                value12 = (byte)((ushort)this.LoadedImage.Width & 255);
                streamWriter.Write(Convert.ToString(value12, 16).PadLeft(2, '0'));
                value12 = (byte)((ushort)this.LoadedImage.Width >> 8 & 255);
                streamWriter.Write(Convert.ToString(value12, 16).PadLeft(2, '0'));
                streamWriter.Write(Convert.ToString(1).PadLeft(2, '0'));
                streamWriter.Write(Convert.ToString(0).PadLeft(2, '0'));
                streamWriter.Write("0C000000");
                for (int num33 = 0; num33 < this.LoadedImage.Height; num33++)
                {
                    for (int num34 = 0; num34 < this.LoadedImage.Width; num34++)
                    {
                        Color pixel9 = this.LoadedImage.GetPixel(num34, num33);
                        float value13 = (float)(0.299 * (double)((float)pixel9.R) + 0.587 * (double)((float)pixel9.G) + 0.114 * (double)((float)pixel9.B));
                        uint num35 = Convert.ToUInt32(value13);
                        if (num35 >= 128u)
                        {
                            array17[(int)(num32 * 2u + 1u)] = '\u0001';
                        }
                        else
                        {
                            array17[(int)(num32 * 2u + 1u)] = '\0';
                        }
                        num35 &= 127u;
                        if (num35 >= 64u)
                        {
                            array17[(int)(num32 * 2u)] = '\u0001';
                        }
                        else
                        {
                            array17[(int)(num32 * 2u)] = '\0';
                        }
                        if (num32 < 3u)
                        {
                            num32 += 1u;
                        }
                        else
                        {
                            this.FormatSharpBW(array17, array18);
                            streamWriter.Write(array18, 2, 2);
                            for (int num36 = 0; num36 < 8; num36++)
                            {
                                array17[num36] = '\0';
                            }
                            num32 = 0u;
                        }
                        if (num34 == this.LoadedImage.Width - 1 && num32 != 0u)
                        {
                            this.FormatSharpBW(array17, array18);
                            streamWriter.Write(array18, 2, 2);
                            for (int num37 = 0; num37 < 8; num37++)
                            {
                                array17[num37] = '\0';
                            }
                            num32 = 0u;
                        }
                    }
                }
            }
            streamWriter.Close();
            IDisposable disposable = streamWriter;
            if (disposable != null)
            {
                disposable.Dispose();
            }
        }

        private void saveBINFile()
        {
            BinaryWriter binaryWriter = new BinaryWriter(this.sDlg.OpenFile());
            if (this.comboBoxFormat.Text == "6-Bit Color")
            {
                binaryWriter.Write((ushort)this.LoadedImage.Width);
                binaryWriter.Write((ushort)this.LoadedImage.Height);
                binaryWriter.Write((ushort)this.LoadedImage.Width);
                binaryWriter.Write(8);
                binaryWriter.Write(0);
                binaryWriter.Write(12u);
                for (int i = 0; i < this.LoadedImage.Height; i++)
                {
                    for (int j = 0; j < this.LoadedImage.Width; j++)
                    {
                        Color pixel = this.LoadedImage.GetPixel(j, i);
                        byte b = 0;
                        b |= (byte)(pixel.A & 192);
                        b = (byte)((int)b | (pixel.R & 192) >> 2);
                        b = (byte)((int)b | (pixel.G & 192) >> 4);
                        b = (byte)((int)b | (pixel.B & 192) >> 6);
                        binaryWriter.Write(b);
                    }
                }
            }
            else if (this.comboBoxFormat.Text == "SPI 3-Bit Color")
            {
                byte b2 = 0;
                bool flag = false;
                binaryWriter.Write((ushort)this.LoadedImage.Width);
                binaryWriter.Write((ushort)this.LoadedImage.Height);
                binaryWriter.Write((ushort)this.LoadedImage.Width);
                binaryWriter.Write(7);
                binaryWriter.Write(0);
                binaryWriter.Write(12u);
                for (int k = 0; k < this.LoadedImage.Height; k++)
                {
                    for (int l = 0; l < this.LoadedImage.Width; l++)
                    {
                        Color pixel2 = this.LoadedImage.GetPixel(l, k);
                        if (flag)
                        {
                            b2 |= (byte)(pixel2.A & 128);
                            b2 = (byte)((int)b2 | (pixel2.R & 128) >> 1);
                            b2 = (byte)((int)b2 | (pixel2.G & 128) >> 2);
                            b2 = (byte)((int)b2 | (pixel2.B & 128) >> 3);
                            binaryWriter.Write(b2);
                            b2 = 0;
                            flag = false;
                        }
                        else
                        {
                            b2 = (byte)((int)b2 | (pixel2.A & 128) >> 4);
                            b2 = (byte)((int)b2 | (pixel2.R & 128) >> 5);
                            b2 = (byte)((int)b2 | (pixel2.G & 128) >> 6);
                            b2 = (byte)((int)b2 | (pixel2.B & 128) >> 7);
                            flag = true;
                            if (l == this.LoadedImage.Width - 1)
                            {
                                binaryWriter.Write(b2);
                                b2 = 0;
                                flag = false;
                            }
                        }
                    }
                }
            }
            else if (this.comboBoxFormat.Text == "SPI 1-Bit B&W")
            {
                byte b3 = 0;
                uint num = 0u;
                binaryWriter.Write((ushort)this.LoadedImage.Width);
                binaryWriter.Write((ushort)this.LoadedImage.Height);
                binaryWriter.Write((ushort)this.LoadedImage.Width);
                binaryWriter.Write(6);
                binaryWriter.Write(0);
                binaryWriter.Write(12u);
                for (int m = 0; m < this.LoadedImage.Height; m++)
                {
                    for (int n = 0; n < this.LoadedImage.Width; n++)
                    {
                        Color pixel3 = this.LoadedImage.GetPixel(n, m);
                        float num2 = (float)(0.299 * (double)((float)pixel3.R) + 0.587 * (double)((float)pixel3.G) + 0.114 * (double)((float)pixel3.B));
                        if ((double)num2 >= 128.0)
                        {
                            b3 = (byte)((int)b3 | 1 << (int)(num * 2u));
                        }
                        else
                        {
                            b3 = (byte)((int)b3 & ~(1 << (int)(num * 2u)));
                        }
                        if (pixel3.A >= 128)
                        {
                            b3 = (byte)((int)b3 | 1 << (int)(num * 2u + 1u));
                        }
                        else
                        {
                            b3 = (byte)((int)b3 & ~(1 << (int)(num * 2u + 1u)));
                        }
                        if (num < 3u)
                        {
                            num += 1u;
                        }
                        else
                        {
                            binaryWriter.Write(b3);
                            b3 = 0;
                            num = 0u;
                        }
                        if (n == this.LoadedImage.Width - 1 && num != 0u)
                        {
                            binaryWriter.Write(b3);
                            b3 = 0;
                            num = 0u;
                        }
                    }
                }
            }
            else if (this.comboBoxFormat.Text == "1bpp Grayscale")
            {
                byte b4 = 0;
                uint num3 = 0u;
                binaryWriter.Write((ushort)this.LoadedImage.Width);
                binaryWriter.Write((ushort)this.LoadedImage.Height);
                binaryWriter.Write((ushort)this.LoadedImage.Width);
                binaryWriter.Write(2);
                binaryWriter.Write(0);
                binaryWriter.Write(12u);
                for (int num4 = 0; num4 < this.LoadedImage.Height; num4++)
                {
                    for (int num5 = 0; num5 < this.LoadedImage.Width; num5++)
                    {
                        Color pixel4 = this.LoadedImage.GetPixel(num5, num4);
                        float num6 = (float)(0.299 * (double)((float)pixel4.R) + 0.587 * (double)((float)pixel4.G) + 0.114 * (double)((float)pixel4.B));
                        if ((double)num6 >= 128.0)
                        {
                            b4 = (byte)((int)b4 | 1 << (int)num3);
                        }
                        else
                        {
                            b4 = (byte)((int)b4 & ~(1 << (int)num3));
                        }
                        if (num3 < 7u)
                        {
                            num3 += 1u;
                        }
                        else
                        {
                            binaryWriter.Write(b4);
                            b4 = 0;
                            num3 = 0u;
                        }
                        if (num5 == this.LoadedImage.Width - 1 && num3 != 0u)
                        {
                            binaryWriter.Write(b4);
                            b4 = 0;
                            num3 = 0u;
                        }
                    }
                }
            }
            else if (this.comboBoxFormat.Text == "2bpp Grayscale")
            {
                byte b5 = 0;
                uint num7 = 0u;
                binaryWriter.Write((ushort)this.LoadedImage.Width);
                binaryWriter.Write((ushort)this.LoadedImage.Height);
                binaryWriter.Write((ushort)this.LoadedImage.Width);
                binaryWriter.Write(3);
                binaryWriter.Write(0);
                binaryWriter.Write(12u);
                for (int num8 = 0; num8 < this.LoadedImage.Height; num8++)
                {
                    for (int num9 = 0; num9 < this.LoadedImage.Width; num9++)
                    {
                        Color pixel5 = this.LoadedImage.GetPixel(num9, num8);
                        float value = (float)(0.299 * (double)((float)pixel5.R) + 0.587 * (double)((float)pixel5.G) + 0.114 * (double)((float)pixel5.B));
                        uint num10 = Convert.ToUInt32(value);
                        if (num10 >= 128u)
                        {
                            b5 = (byte)((int)b5 | 1 << (int)(num7 * 2u + 1u));
                        }
                        else
                        {
                            b5 = (byte)((int)b5 & ~(1 << (int)(num7 * 2u + 1u)));
                        }
                        num10 &= 127u;
                        if (num10 >= 64u)
                        {
                            b5 = (byte)((int)b5 | 1 << (int)(num7 * 2u));
                        }
                        else
                        {
                            b5 = (byte)((int)b5 & ~(1 << (int)(num7 * 2u)));
                        }
                        if (num7 < 3u)
                        {
                            num7 += 1u;
                        }
                        else
                        {
                            binaryWriter.Write(b5);
                            b5 = 0;
                            num7 = 0u;
                        }
                        if (num9 == this.LoadedImage.Width - 1 && num7 != 0u)
                        {
                            binaryWriter.Write(b5);
                            b5 = 0;
                            num7 = 0u;
                        }
                    }
                }
            }
            else if (this.comboBoxFormat.Text == "4bpp Grayscale")
            {
                byte b6 = 0;
                uint num11 = 0u;
                binaryWriter.Write((ushort)this.LoadedImage.Width);
                binaryWriter.Write((ushort)this.LoadedImage.Height);
                binaryWriter.Write((ushort)this.LoadedImage.Width);
                binaryWriter.Write(4);
                binaryWriter.Write(0);
                binaryWriter.Write(12u);
                for (int num12 = 0; num12 < this.LoadedImage.Height; num12++)
                {
                    for (int num13 = 0; num13 < this.LoadedImage.Width; num13++)
                    {
                        Color pixel6 = this.LoadedImage.GetPixel(num13, num12);
                        float value2 = (float)(0.299 * (double)((float)pixel6.R) + 0.587 * (double)((float)pixel6.G) + 0.114 * (double)((float)pixel6.B));
                        uint num14 = Convert.ToUInt32(value2);
                        if (num14 >= 128u)
                        {
                            b6 = (byte)((int)b6 | 1 << (int)(num11 * 4u + 3u));
                        }
                        else
                        {
                            b6 = (byte)((int)b6 & ~(1 << (int)(num11 * 4u + 3u)));
                        }
                        num14 &= 127u;
                        if (num14 >= 64u)
                        {
                            b6 = (byte)((int)b6 | 1 << (int)(num11 * 4u + 2u));
                        }
                        else
                        {
                            b6 = (byte)((int)b6 & ~(1 << (int)(num11 * 4u + 2u)));
                        }
                        num14 &= 63u;
                        if (num14 >= 32u)
                        {
                            b6 = (byte)((int)b6 | 1 << (int)(num11 * 4u + 1u));
                        }
                        else
                        {
                            b6 = (byte)((int)b6 & ~(1 << (int)(num11 * 4u + 1u)));
                        }
                        num14 &= 31u;
                        if (num14 >= 16u)
                        {
                            b6 = (byte)((int)b6 | 1 << (int)(num11 * 4u));
                        }
                        else
                        {
                            b6 = (byte)((int)b6 & ~(1 << (int)(num11 * 4u)));
                        }
                        if (num11 < 1u)
                        {
                            num11 += 1u;
                        }
                        else
                        {
                            binaryWriter.Write(b6);
                            b6 = 0;
                            num11 = 0u;
                        }
                        if (num13 == this.LoadedImage.Width - 1 && num11 != 0u)
                        {
                            binaryWriter.Write(b6);
                            b6 = 0;
                            num11 = 0u;
                        }
                    }
                }
            }
            else if (this.comboBoxFormat.Text == "8bpp Grayscale")
            {
                byte b7 = 0;
                binaryWriter.Write((ushort)this.LoadedImage.Width);
                binaryWriter.Write((ushort)this.LoadedImage.Height);
                binaryWriter.Write((ushort)this.LoadedImage.Width);
                binaryWriter.Write(5);
                binaryWriter.Write(0);
                binaryWriter.Write(12u);
                for (int num15 = 0; num15 < this.LoadedImage.Height; num15++)
                {
                    for (int num16 = 0; num16 < this.LoadedImage.Width; num16++)
                    {
                        Color pixel7 = this.LoadedImage.GetPixel(num16, num15);
                        float value3 = (float)(0.299 * (double)((float)pixel7.R) + 0.587 * (double)((float)pixel7.G) + 0.114 * (double)((float)pixel7.B));
                        uint num17 = Convert.ToUInt32(value3);
                        if (num17 >= 128u)
                        {
                            b7 |= 128;
                        }
                        num17 &= 127u;
                        if (num17 >= 64u)
                        {
                            b7 |= 64;
                        }
                        num17 &= 63u;
                        if (num17 >= 32u)
                        {
                            b7 |= 32;
                        }
                        num17 &= 31u;
                        if (num17 >= 16u)
                        {
                            b7 |= 16;
                        }
                        num17 &= 15u;
                        if (num17 >= 8u)
                        {
                            b7 |= 8;
                        }
                        num17 &= 7u;
                        if (num17 >= 4u)
                        {
                            b7 |= 4;
                        }
                        num17 &= 3u;
                        if (num17 >= 2u)
                        {
                            b7 |= 2;
                        }
                        num17 &= 1u;
                        if (num17 != 0u)
                        {
                            b7 |= 1;
                        }
                        binaryWriter.Write(b7);
                        b7 = 0;
                    }
                }
            }
            else if (this.comboBoxFormat.Text == "1-Bit Bitmap")
            {
                byte b8 = 0;
                uint num18 = 0u;
                binaryWriter.Write((ushort)this.LoadedImage.Width);
                binaryWriter.Write((ushort)this.LoadedImage.Height);
                binaryWriter.Write((ushort)this.LoadedImage.Width);
                binaryWriter.Write(0);
                binaryWriter.Write(0);
                binaryWriter.Write(12u);
                for (int num19 = 0; num19 < this.LoadedImage.Height; num19++)
                {
                    for (int num20 = 0; num20 < this.LoadedImage.Width; num20++)
                    {
                        Color pixel8 = this.LoadedImage.GetPixel(num20, num19);
                        float num21 = (float)(0.299 * (double)((float)pixel8.R) + 0.587 * (double)((float)pixel8.G) + 0.114 * (double)((float)pixel8.B));
                        if ((double)num21 >= 128.0)
                        {
                            b8 = (byte)((int)b8 | 1 << (int)num18);
                        }
                        else
                        {
                            b8 = (byte)((int)b8 & ~(1 << (int)num18));
                        }
                        if (num18 < 7u)
                        {
                            num18 += 1u;
                        }
                        else
                        {
                            binaryWriter.Write(b8);
                            b8 = 0;
                            num18 = 0u;
                        }
                        if (num20 == this.LoadedImage.Width - 1 && num18 != 0u)
                        {
                            binaryWriter.Write(b8);
                            b8 = 0;
                            num18 = 0u;
                        }
                    }
                }
            }
            else if (this.comboBoxFormat.Text == "2-Bit Bitmap")
            {
                byte b9 = 0;
                uint num22 = 0u;
                binaryWriter.Write((ushort)this.LoadedImage.Width);
                binaryWriter.Write((ushort)this.LoadedImage.Height);
                binaryWriter.Write((ushort)this.LoadedImage.Width);
                binaryWriter.Write(1);
                binaryWriter.Write(0);
                binaryWriter.Write(12u);
                for (int num23 = 0; num23 < this.LoadedImage.Height; num23++)
                {
                    for (int num24 = 0; num24 < this.LoadedImage.Width; num24++)
                    {
                        Color pixel9 = this.LoadedImage.GetPixel(num24, num23);
                        float value4 = (float)(0.299 * (double)((float)pixel9.R) + 0.587 * (double)((float)pixel9.G) + 0.114 * (double)((float)pixel9.B));
                        uint num25 = Convert.ToUInt32(value4);
                        if (num25 >= 128u)
                        {
                            b9 = (byte)((int)b9 | 1 << (int)(num22 * 2u + 1u));
                        }
                        else
                        {
                            b9 = (byte)((int)b9 & ~(1 << (int)(num22 * 2u + 1u)));
                        }
                        num25 &= 127u;
                        if (num25 >= 64u)
                        {
                            b9 = (byte)((int)b9 | 1 << (int)(num22 * 2u));
                        }
                        else
                        {
                            b9 = (byte)((int)b9 & ~(1 << (int)(num22 * 2u)));
                        }
                        if (num22 < 3u)
                        {
                            num22 += 1u;
                        }
                        else
                        {
                            binaryWriter.Write(b9);
                            b9 = 0;
                            num22 = 0u;
                        }
                        if (num24 == this.LoadedImage.Width - 1 && num22 != 0u)
                        {
                            binaryWriter.Write(b9);
                            b9 = 0;
                            num22 = 0u;
                        }
                    }
                }
            }
            binaryWriter.Close();
            IDisposable disposable = binaryWriter;
            if (disposable != null)
            {
                disposable.Dispose();
            }
        }

        private void saveHFile()
        {
            StreamWriter streamWriter = new StreamWriter(this.sDlg.OpenFile());
            streamWriter.Write("/* ImaConvApp.exe C header generated file (");
            streamWriter.Write(Path.GetFileName(this.sDlg.FileName));
            streamWriter.Write(") */ \n");
            streamWriter.Write("const uint8_t ");
            streamWriter.Write(Path.GetFileNameWithoutExtension(this.sDlg.FileName));
            streamWriter.Write("_pxdata[] = {\n  ");
            if (this.comboBoxFormat.Text == "6-Bit Color")
            {
                char[] array = new char[]
				{
					'0',
					'x',
					'0',
					'0'
				};
                uint num = 1u;
                for (int i = 0; i < this.LoadedImage.Height; i++)
                {
                    for (int j = 0; j < this.LoadedImage.Width; j++)
                    {
                        Color pixel = this.LoadedImage.GetPixel(j, i);
                        FormatLtf(pixel.A, pixel.R, pixel.G, pixel.B, array);//0xC0, 0x40, 0x40, 0x40, array);//
                        //FormatJDI(pixel.A, pixel.R, pixel.G, pixel.B, array);
                        if (num != 0u)
                        {
                            num = 0u;
                        }
                        else
                        {
                            streamWriter.Write(", ");
                        }
                        streamWriter.Write(array);
                    }
                }
            }
            else if (this.comboBoxFormat.Text == "SPI 3-Bit Color")
            {
                uint num2 = 1u;
                bool flag = false;
                char[] array2 = new char[]
				{
					'0'
				};
                char[] array3 = new char[]
				{
					'0'
				};
                char[] array4 = new char[]
				{
					'0',
					'x',
					'0',
					'0'
				};
                for (int k = 0; k < this.LoadedImage.Height; k++)
                {
                    for (int l = 0; l < this.LoadedImage.Width; l++)
                    {
                        Color pixel2 = this.LoadedImage.GetPixel(l, k);
                        if (flag)
                        {
                            this.FormatSharpColor(pixel2.A, pixel2.R, pixel2.G, pixel2.B, array2);
                            array4[2] = array2[0];
                            array4[3] = array3[0];
                            if (num2 != 0u)
                            {
                                num2 = 0u;
                            }
                            else
                            {
                                streamWriter.Write(", ");
                            }
                            streamWriter.Write(array4);
                            flag = false;
                        }
                        else
                        {
                            this.FormatSharpColor(pixel2.A, pixel2.R, pixel2.G, pixel2.B, array3);
                            flag = true;
                            if (l == this.LoadedImage.Width - 1)
                            {
                                array4[2] = '0';
                                array4[3] = array3[0];
                                streamWriter.Write(", ");
                                streamWriter.Write(array4);
                                flag = false;
                            }
                        }
                    }
                }
            }
            else if (this.comboBoxFormat.Text == "SPI 1-Bit B&W")
            {
                char[] array5 = new char[]
				{
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0'
				};
                uint num3 = 0u;
                char[] array6 = new char[]
				{
					'0',
					'x',
					'0',
					'0'
				};
                uint num4 = 1u;
                for (int m = 0; m < this.LoadedImage.Height; m++)
                {
                    for (int n = 0; n < this.LoadedImage.Width; n++)
                    {
                        Color pixel3 = this.LoadedImage.GetPixel(n, m);
                        float num5 = (float)(0.299 * (double)((float)pixel3.R) + 0.587 * (double)((float)pixel3.G) + 0.114 * (double)((float)pixel3.B));
                        if ((double)num5 >= 128.0)
                        {
                            array5[(int)(num3 * 2u)] = '\u0001';
                        }
                        else
                        {
                            array5[(int)(num3 * 2u)] = '\0';
                        }
                        if (pixel3.A >= 128)
                        {
                            array5[(int)(num3 * 2u + 1u)] = '\u0001';
                        }
                        else
                        {
                            array5[(int)(num3 * 2u + 1u)] = '\0';
                        }
                        if (num3 < 3u)
                        {
                            num3 += 1u;
                        }
                        else
                        {
                            this.FormatSharpBW(array5, array6);
                            if (num4 != 0u)
                            {
                                num4 = 0u;
                            }
                            else
                            {
                                streamWriter.Write(", ");
                            }
                            streamWriter.Write(array6);
                            for (int num6 = 0; num6 < 8; num6++)
                            {
                                array5[num6] = '\0';
                            }
                            num3 = 0u;
                        }
                        if (n == this.LoadedImage.Width - 1 && num3 != 0u)
                        {
                            this.FormatSharpBW(array5, array6);
                            streamWriter.Write(", ");
                            streamWriter.Write(array6);
                            for (int num7 = 0; num7 < 8; num7++)
                            {
                                array5[num7] = '\0';
                            }
                            num3 = 0u;
                        }
                    }
                }
            }
            else if (this.comboBoxFormat.Text == "1bpp Grayscale")
            {
                char[] array7 = new char[]
				{
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0'
				};
                uint num8 = 0u;
                char[] array8 = new char[]
				{
					'0',
					'x',
					'0',
					'0'
				};
                uint num9 = 1u;
                for (int num10 = 0; num10 < this.LoadedImage.Height; num10++)
                {
                    for (int num11 = 0; num11 < this.LoadedImage.Width; num11++)
                    {
                        Color pixel4 = this.LoadedImage.GetPixel(num11, num10);
                        float num12 = (float)(0.299 * (double)((float)pixel4.R) + 0.587 * (double)((float)pixel4.G) + 0.114 * (double)((float)pixel4.B));
                        if ((double)num12 >= 128.0)
                        {
                            array7[(int)num8] = '\u0001';
                        }
                        else
                        {
                            array7[(int)num8] = '\0';
                        }
                        if (num8 < 7u)
                        {
                            num8 += 1u;
                        }
                        else
                        {
                            this.FormatSharpBW(array7, array8);
                            if (num9 != 0u)
                            {
                                num9 = 0u;
                            }
                            else
                            {
                                streamWriter.Write(", ");
                            }
                            streamWriter.Write(array8);
                            for (int num13 = 0; num13 < 8; num13++)
                            {
                                array7[num13] = '\0';
                            }
                            num8 = 0u;
                        }
                        if (num11 == this.LoadedImage.Width - 1 && num8 != 0u)
                        {
                            this.FormatSharpBW(array7, array8);
                            streamWriter.Write(", ");
                            streamWriter.Write(array8);
                            for (int num14 = 0; num14 < 8; num14++)
                            {
                                array7[num14] = '\0';
                            }
                            num8 = 0u;
                        }
                    }
                }
            }
            else if (this.comboBoxFormat.Text == "2bpp Grayscale")
            {
                char[] array9 = new char[]
				{
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0'
				};
                uint num15 = 0u;
                char[] array10 = new char[]
				{
					'0',
					'x',
					'0',
					'0'
				};
                uint num16 = 1u;
                for (int num17 = 0; num17 < this.LoadedImage.Height; num17++)
                {
                    for (int num18 = 0; num18 < this.LoadedImage.Width; num18++)
                    {
                        Color pixel5 = this.LoadedImage.GetPixel(num18, num17);
                        float value = (float)(0.299 * (double)((float)pixel5.R) + 0.587 * (double)((float)pixel5.G) + 0.114 * (double)((float)pixel5.B));
                        uint num19 = Convert.ToUInt32(value);
                        if (num19 >= 128u)
                        {
                            array9[(int)(num15 * 2u + 1u)] = '\u0001';
                        }
                        else
                        {
                            array9[(int)(num15 * 2u + 1u)] = '\0';
                        }
                        num19 &= 127u;
                        if (num19 >= 64u)
                        {
                            array9[(int)(num15 * 2u)] = '\u0001';
                        }
                        else
                        {
                            array9[(int)(num15 * 2u)] = '\0';
                        }
                        if (num15 < 3u)
                        {
                            num15 += 1u;
                        }
                        else
                        {
                            this.FormatSharpBW(array9, array10);
                            if (num16 != 0u)
                            {
                                num16 = 0u;
                            }
                            else
                            {
                                streamWriter.Write(", ");
                            }
                            streamWriter.Write(array10);
                            for (int num20 = 0; num20 < 8; num20++)
                            {
                                array9[num20] = '\0';
                            }
                            num15 = 0u;
                        }
                        if (num18 == this.LoadedImage.Width - 1 && num15 != 0u)
                        {
                            this.FormatSharpBW(array9, array10);
                            streamWriter.Write(", ");
                            streamWriter.Write(array10);
                            for (int num21 = 0; num21 < 8; num21++)
                            {
                                array9[num21] = '\0';
                            }
                            num15 = 0u;
                        }
                    }
                }
            }
            else if (this.comboBoxFormat.Text == "4bpp Grayscale")
            {
                char[] array11 = new char[]
				{
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0'
				};
                uint num22 = 0u;
                char[] array12 = new char[]
				{
					'0',
					'x',
					'0',
					'0'
				};
                uint num23 = 1u;
                for (int num24 = 0; num24 < this.LoadedImage.Height; num24++)
                {
                    for (int num25 = 0; num25 < this.LoadedImage.Width; num25++)
                    {
                        Color pixel6 = this.LoadedImage.GetPixel(num25, num24);
                        float value2 = (float)(0.299 * (double)((float)pixel6.R) + 0.587 * (double)((float)pixel6.G) + 0.114 * (double)((float)pixel6.B));
                        uint num26 = Convert.ToUInt32(value2);
                        if (num26 >= 128u)
                        {
                            array11[(int)(num22 * 4u + 3u)] = '\u0001';
                        }
                        else
                        {
                            array11[(int)(num22 * 4u + 3u)] = '\0';
                        }
                        num26 &= 127u;
                        if (num26 >= 64u)
                        {
                            array11[(int)(num22 * 4u + 2u)] = '\u0001';
                        }
                        else
                        {
                            array11[(int)(num22 * 4u + 2u)] = '\0';
                        }
                        num26 &= 63u;
                        if (num26 >= 32u)
                        {
                            array11[(int)(num22 * 4u + 1u)] = '\u0001';
                        }
                        else
                        {
                            array11[(int)(num22 * 4u + 1u)] = '\0';
                        }
                        num26 &= 31u;
                        if (num26 >= 16u)
                        {
                            array11[(int)(num22 * 4u)] = '\u0001';
                        }
                        else
                        {
                            array11[(int)(num22 * 4u)] = '\0';
                        }
                        if (num22 < 1u)
                        {
                            num22 += 1u;
                        }
                        else
                        {
                            this.FormatSharpBW(array11, array12);
                            if (num23 != 0u)
                            {
                                num23 = 0u;
                            }
                            else
                            {
                                streamWriter.Write(", ");
                            }
                            streamWriter.Write(array12);
                            for (int num27 = 0; num27 < 8; num27++)
                            {
                                array11[num27] = '\0';
                            }
                            num22 = 0u;
                        }
                        if (num25 == this.LoadedImage.Width - 1 && num22 != 0u)
                        {
                            this.FormatSharpBW(array11, array12);
                            streamWriter.Write(", ");
                            streamWriter.Write(array12);
                            for (int num28 = 0; num28 < 8; num28++)
                            {
                                array11[num28] = '\0';
                            }
                            num22 = 0u;
                        }
                    }
                }
            }
            else if (this.comboBoxFormat.Text == "8bpp Grayscale")
            {
                char[] array13 = new char[]
				{
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0'
				};
                char[] array14 = new char[]
				{
					'0',
					'x',
					'0',
					'0'
				};
                uint num29 = 1u;
                for (int num30 = 0; num30 < this.LoadedImage.Height; num30++)
                {
                    for (int num31 = 0; num31 < this.LoadedImage.Width; num31++)
                    {
                        Color pixel7 = this.LoadedImage.GetPixel(num31, num30);
                        float value3 = (float)(0.299 * (double)((float)pixel7.R) + 0.587 * (double)((float)pixel7.G) + 0.114 * (double)((float)pixel7.B));
                        uint num32 = Convert.ToUInt32(value3);
                        if (num32 >= 128u)
                        {
                            array13[7] = '\u0001';
                        }
                        else
                        {
                            array13[7] = '\0';
                        }
                        num32 &= 127u;
                        if (num32 >= 64u)
                        {
                            array13[6] = '\u0001';
                        }
                        else
                        {
                            array13[6] = '\0';
                        }
                        num32 &= 63u;
                        if (num32 >= 32u)
                        {
                            array13[5] = '\u0001';
                        }
                        else
                        {
                            array13[5] = '\0';
                        }
                        num32 &= 31u;
                        if (num32 >= 16u)
                        {
                            array13[4] = '\u0001';
                        }
                        else
                        {
                            array13[4] = '\0';
                        }
                        num32 &= 15u;
                        if (num32 >= 8u)
                        {
                            array13[3] = '\u0001';
                        }
                        else
                        {
                            array13[3] = '\0';
                        }
                        num32 &= 7u;
                        if (num32 >= 4u)
                        {
                            array13[2] = '\u0001';
                        }
                        else
                        {
                            array13[2] = '\0';
                        }
                        num32 &= 3u;
                        if (num32 >= 2u)
                        {
                            array13[1] = '\u0001';
                        }
                        else
                        {
                            array13[1] = '\0';
                        }
                        num32 &= 1u;
                        if (num32 != 0u)
                        {
                            array13[0] = '\u0001';
                        }
                        else
                        {
                            array13[0] = '\0';
                        }
                        this.FormatSharpBW(array13, array14);
                        if (num29 != 0u)
                        {
                            num29 = 0u;
                        }
                        else
                        {
                            streamWriter.Write(", ");
                        }
                        streamWriter.Write(array14);
                    }
                }
            }
            else if (this.comboBoxFormat.Text == "1-Bit Bitmap")
            {
                char[] array15 = new char[]
				{
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0'
				};
                uint num33 = 0u;
                char[] array16 = new char[]
				{
					'0',
					'x',
					'0',
					'0'
				};
                uint num34 = 1u;
                for (int num35 = 0; num35 < this.LoadedImage.Height; num35++)
                {
                    for (int num36 = 0; num36 < this.LoadedImage.Width; num36++)
                    {
                        Color pixel8 = this.LoadedImage.GetPixel(num36, num35);
                        float num37 = (float)(0.299 * (double)((float)pixel8.R) + 0.587 * (double)((float)pixel8.G) + 0.114 * (double)((float)pixel8.B));
                        if ((double)num37 >= 128.0)
                        {
                            array15[(int)num33] = '\u0001';
                        }
                        else
                        {
                            array15[(int)num33] = '\0';
                        }
                        if (num33 < 7u)
                        {
                            num33 += 1u;
                        }
                        else
                        {
                            this.FormatSharpBW(array15, array16);
                            if (num34 != 0u)
                            {
                                num34 = 0u;
                            }
                            else
                            {
                                streamWriter.Write(", ");
                            }
                            streamWriter.Write(array16);
                            for (int num38 = 0; num38 < 8; num38++)
                            {
                                array15[num38] = '\0';
                            }
                            num33 = 0u;
                        }
                        if (num36 == this.LoadedImage.Width - 1 && num33 != 0u)
                        {
                            this.FormatSharpBW(array15, array16);
                            streamWriter.Write(", ");
                            streamWriter.Write(array16);
                            for (int num39 = 0; num39 < 8; num39++)
                            {
                                array15[num39] = '\0';
                            }
                            num33 = 0u;
                        }
                    }
                }
            }
            else if (this.comboBoxFormat.Text == "2-Bit Bitmap")
            {
                char[] array17 = new char[]
				{
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0',
					'\0'
				};
                uint num40 = 0u;
                char[] array18 = new char[]
				{
					'0',
					'x',
					'0',
					'0'
				};
                uint num41 = 1u;
                for (int num42 = 0; num42 < this.LoadedImage.Height; num42++)
                {
                    for (int num43 = 0; num43 < this.LoadedImage.Width; num43++)
                    {
                        Color pixel9 = this.LoadedImage.GetPixel(num43, num42);
                        float value4 = (float)(0.299 * (double)((float)pixel9.R) + 0.587 * (double)((float)pixel9.G) + 0.114 * (double)((float)pixel9.B));
                        uint num44 = Convert.ToUInt32(value4);
                        if (num44 >= 128u)
                        {
                            array17[(int)(num40 * 2u + 1u)] = '\u0001';
                        }
                        else
                        {
                            array17[(int)(num40 * 2u + 1u)] = '\0';
                        }
                        num44 &= 127u;
                        if (num44 >= 64u)
                        {
                            array17[(int)(num40 * 2u)] = '\u0001';
                        }
                        else
                        {
                            array17[(int)(num40 * 2u)] = '\0';
                        }
                        if (num40 < 3u)
                        {
                            num40 += 1u;
                        }
                        else
                        {
                            this.FormatSharpBW(array17, array18);
                            if (num41 != 0u)
                            {
                                num41 = 0u;
                            }
                            else
                            {
                                streamWriter.Write(", ");
                            }
                            streamWriter.Write(array18);
                            for (int num45 = 0; num45 < 8; num45++)
                            {
                                array17[num45] = '\0';
                            }
                            num40 = 0u;
                        }
                        if (num43 == this.LoadedImage.Width - 1 && num40 != 0u)
                        {
                            this.FormatSharpBW(array17, array18);
                            streamWriter.Write(", ");
                            streamWriter.Write(array18);
                            for (int num46 = 0; num46 < 8; num46++)
                            {
                                array17[num46] = '\0';
                            }
                            num40 = 0u;
                        }
                    }
                }
            }
            streamWriter.Write("\n  }; \n\n");
            streamWriter.Write("const LCD_ImgStruct ");
            streamWriter.Write(Path.GetFileNameWithoutExtension(this.sDlg.FileName));
            streamWriter.Write(" = {\n");
            streamWriter.Write("  ");
            streamWriter.Write(Convert.ToString(this.LoadedImage.Width));
            streamWriter.Write(", ");
            streamWriter.Write(Convert.ToString(this.LoadedImage.Height));
            streamWriter.Write(", ");
            streamWriter.Write(Convert.ToString(this.LoadedImage.Width));
            streamWriter.Write(", ");
            if (this.comboBoxFormat.Text == "6-Bit Color")
            {
                streamWriter.Write("LCD_6BIT_COLOR,\n");
            }
            else if (this.comboBoxFormat.Text == "SPI 3-Bit Color")
            {
                streamWriter.Write("LCD_SPI3BIT_COLOR,\n");
            }
            else if (this.comboBoxFormat.Text == "SPI 1-Bit B&W")
            {
                streamWriter.Write("LCD_SPI1BIT_BW, \n");
            }
            else if (this.comboBoxFormat.Text == "1bpp Grayscale")
            {
                streamWriter.Write("LCD_1BPP_GRSCL, \n");
            }
            else if (this.comboBoxFormat.Text == "2bpp Grayscale")
            {
                streamWriter.Write("LCD_2BPP_GRSCL, \n");
            }
            else if (this.comboBoxFormat.Text == "4bpp Grayscale")
            {
                streamWriter.Write("LCD_4BPP_GRSCL, \n");
            }
            else if (this.comboBoxFormat.Text == "8bpp Grayscale")
            {
                streamWriter.Write("LCD_8BPP_GRSCL, \n");
            }
            else if (this.comboBoxFormat.Text == "1-Bit Bitmap")
            {
                streamWriter.Write("LCD_1BIT_BITMAP, \n");
            }
            else if (this.comboBoxFormat.Text == "2-Bit Bitmap")
            {
                streamWriter.Write("LCD_2BBIT_BITMAP, \n");
            }
            streamWriter.Write("  (uint8_t *) ");
            streamWriter.Write(Path.GetFileNameWithoutExtension(this.sDlg.FileName));
            streamWriter.Write("_pxdata\n  };");
            streamWriter.Close();
            IDisposable disposable = streamWriter;
            if (disposable != null)
            {
                disposable.Dispose();
            }
        }
        private sbyte ToHexByte(sbyte rawByte) 
        {
            sbyte retByte = rawByte;
            if (retByte > 9)
            {
                retByte += 55;
            }
            else
            {
                retByte += 48;
            }

            return retByte;
        }

        private void FormatLtf(byte inAlpha, byte inRed, byte inGreen, byte inBlue, char[] outArray)
        {
            sbyte high = (sbyte)(((inAlpha & 0x80) >> 4) | ((inRed & 0x80) >> 5) | ((inGreen & 0x80) >> 6) | ((inBlue & 0x80) >> 7));
            sbyte low = (sbyte)(((inAlpha & 0x40) >> 3) | ((inRed & 0x40) >> 4) | ((inGreen & 0x40) >> 5) | ((inBlue & 0x40) >> 6));

            outArray[0] = '0';
            outArray[1] = 'x';
            outArray[2] = (char)ToHexByte(high);
            outArray[3] = (char)ToHexByte(low);
        }

        private void FormatJDI(byte inAlpha, byte inRed, byte inGreen, byte inBlue, char[] outArray)
        {
            sbyte high = (sbyte)((inAlpha & 192) >> 4 | (inRed & 192) >> 6);
            sbyte low = (sbyte)((inGreen & 192) >> 4 | (inBlue & 192) >> 6);

            outArray[0] = '0';
            outArray[1] = 'x';
            outArray[2] = (char)ToHexByte(high);
            outArray[3] = (char)ToHexByte(low);
        }

        private void FormatSharpColor(byte inAlpha, byte inRed, byte inGreen, byte inBlue, char[] outChar)
        {
            sbyte b = (sbyte)((inAlpha & 128) >> 4 | (inRed & 128) >> 5 | (inGreen & 128) >> 6 | (inBlue & 128) >> 7);

            outChar[0] = (char)ToHexByte(b);
        }

        private void FormatSharpBW(char[] inBuf, char[] outArray)
        {
            sbyte b = 0;
            sbyte b2 = 0;
            if (inBuf[7] != '\0')
            {
                b |= 8;
            }
            if (inBuf[6] != '\0')
            {
                b |= 4;
            }
            if (inBuf[5] != '\0')
            {
                b |= 2;
            }
            if (inBuf[4] != '\0')
            {
                b |= 1;
            }
            if (inBuf[3] != '\0')
            {
                b2 |= 8;
            }
            if (inBuf[2] != '\0')
            {
                b2 |= 4;
            }
            if (inBuf[1] != '\0')
            {
                b2 |= 2;
            }
            if (inBuf[0] != '\0')
            {
                b2 |= 1;
            }

            outArray[0] = '0';
            outArray[1] = 'x';
            outArray[2] = (char)ToHexByte(b);
            outArray[3] = (char)ToHexByte(b2);
        }
    }
}
