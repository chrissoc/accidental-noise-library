using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SimplexGUI
{
    public partial class Form1 : Form
    {

        [StructLayout(LayoutKind.Sequential, Pack = 1, Size = 8)]
        struct ARGB
        {
            public ushort a;
            public ushort r;
            public ushort g;
            public ushort b;
        };

        public Form1()
        {
            InitializeComponent();
        }

        static uint ToARGB8(byte a, byte r, byte g, byte b)
        {
            uint A = a, R = r, G = g, B = b;
            return (A << (8 * 3)) | (R << (8 * 2)) | (G << (8 * 1)) | (B << (8 * 0));
        }

        private void btnSample_Click(object sender, EventArgs e)
        {
            int offsetX;
            int offsetY;
            int width;
            int height;

            if(!int.TryParse(txtOffsetX.Text, out offsetX))
            {
                txtOffsetX.Text = "ERROR";
                return;
            }

            if (!int.TryParse(txtOffsetY.Text, out offsetY))
            {
                txtOffsetY.Text = "ERROR";
                return;
            }

            if (!int.TryParse(txtSampleWidth.Text, out width))
            {
                txtSampleWidth.Text = "ERROR";
                return;
            }

            if (!int.TryParse(txtSampleHeight.Text, out height))
            {
                txtSampleHeight.Text = "ERROR";
                return;
            }


            uint[] sampleData = new uint[width * height];

            try
            {
                Stopwatch t = new Stopwatch();
                t.Start();

                if (SimplexImportDLL.FillSampleArea(offsetX, offsetY, width, height, sampleData, sampleData.Length) != 0)
                {
                    MessageBox.Show("Error in FillSampleArea");
                    return;
                }

                t.Stop();
                lblElapsedTime.Text = t.ElapsedMilliseconds.ToString();
            }
            catch(DllNotFoundException exception)
            {
                MessageBox.Show(exception.Message);
                return;
            }
            catch(EntryPointNotFoundException exception)
            {
                MessageBox.Show(exception.Message);
                return;
            }

            // convert our 32 bit heightmap to a 16 bit heightmap for storage in the Bitmap



            uint[] BitmapData = new uint[sampleData.Length];
            for (int i = 0; i < BitmapData.Length; i += 1)
            {
                BitmapData[i] = ToARGB8(byte.MaxValue, (byte)(sampleData[i] >> 24), (byte)(sampleData[i] >> 24), (byte)(sampleData[i] >> 24));
                //BitmapData[i + 0] = byte.MaxValue * 0;//(ushort)((uint)sampleData[i] >> 16);
                //BitmapData[i + 1] = byte.MaxValue * 0;//(ushort)((uint)sampleData[i] >> 16);
                //BitmapData[i + 2] = byte.MaxValue * 1;//(ushort)((uint)sampleData[i] >> 16);
                //BitmapData[i + 3] = byte.MaxValue;//(ushort)((uint)sampleData[i] >> 16);
            }

            //if (sizeof(ARGB) != 8)
            //    MessageBox.Show("ARGB wrong size");

            GCHandle BitmapDataHandle = GCHandle.Alloc(BitmapData, GCHandleType.Pinned);

            Bitmap bitmap;

            try
            {
                bitmap = new Bitmap(width, height, 4 * width, System.Drawing.Imaging.PixelFormat.Format32bppArgb, BitmapDataHandle.AddrOfPinnedObject());
            }
            finally
            {
                BitmapDataHandle.Free();
            }

            pictureBoxSample.Image = bitmap;
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            txtQuickSaveDir.Text = Directory.GetCurrentDirectory() + Path.DirectorySeparatorChar;
            lblElapsedTime.Text = "";
            lblTotalFolds.Text = "";
            lblTotalInstructions.Text = "";
        }

        private void btnQuickSave_Click(object sender, EventArgs e)
        {
            //if (pictureBoxSample.Image)
            try
            {
                string fileName = txtQuickSaveDir.Text + DateTime.Now.ToShortTimeString().Replace(':', '_') + "-qs.png";
                var tmp = pictureBoxSample.Image as Bitmap;
                if(tmp != null)
                    tmp.Save(fileName, System.Drawing.Imaging.ImageFormat.Png);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        private void btnParseExpression_Click(object sender, EventArgs e)
        {
            int offsetX;
            int offsetY;
            int width;
            int height;

            if (!int.TryParse(txtOffsetX.Text, out offsetX))
            {
                txtOffsetX.Text = "ERROR";
                return;
            }

            if (!int.TryParse(txtOffsetY.Text, out offsetY))
            {
                txtOffsetY.Text = "ERROR";
                return;
            }

            if (!int.TryParse(txtSampleWidth.Text, out width))
            {
                txtSampleWidth.Text = "ERROR";
                return;
            }

            if (!int.TryParse(txtSampleHeight.Text, out height))
            {
                txtSampleHeight.Text = "ERROR";
                return;
            }


            uint[] sampleData = new uint[width * height];

            try
            {
                Stopwatch t = new Stopwatch();
                t.Start();

                //if (SimplexImportDLL.FillSampleArea(offsetX, offsetY, width, height, sampleData, sampleData.Length) != 0)
                string ErrorMsg;
                int totalFolds, totalInstructions;
                if(SimplexImportDLL.MapExpressionToArea(
                    offsetX, offsetY, width, height, 
                    txtNoiseExpression.Text, out ErrorMsg, sampleData, sampleData.Length, 
                    out totalFolds, out totalInstructions) != 0)
                {
                    MessageBox.Show(ErrorMsg, "Error in MapExpressionToArea");
                    return;
                }

                lblTotalFolds.Text = totalFolds.ToString();
                lblTotalInstructions.Text = totalInstructions.ToString();

                t.Stop();
                lblElapsedTime.Text = t.ElapsedMilliseconds.ToString();
            }
            catch (DllNotFoundException exception)
            {
                MessageBox.Show(exception.Message);
                return;
            }
            catch (EntryPointNotFoundException exception)
            {
                MessageBox.Show(exception.Message);
                return;
            }

            // convert our 32 bit heightmap to a 16 bit heightmap for storage in the Bitmap



            uint[] BitmapData = new uint[sampleData.Length];
            for (int i = 0; i < BitmapData.Length; i += 1)
            {
                BitmapData[i] = ToARGB8(byte.MaxValue, (byte)(sampleData[i] >> 24), (byte)(sampleData[i] >> 24), (byte)(sampleData[i] >> 24));
                //BitmapData[i + 0] = byte.MaxValue * 0;//(ushort)((uint)sampleData[i] >> 16);
                //BitmapData[i + 1] = byte.MaxValue * 0;//(ushort)((uint)sampleData[i] >> 16);
                //BitmapData[i + 2] = byte.MaxValue * 1;//(ushort)((uint)sampleData[i] >> 16);
                //BitmapData[i + 3] = byte.MaxValue;//(ushort)((uint)sampleData[i] >> 16);
            }

            //if (sizeof(ARGB) != 8)
            //    MessageBox.Show("ARGB wrong size");

            GCHandle BitmapDataHandle = GCHandle.Alloc(BitmapData, GCHandleType.Pinned);

            Bitmap bitmap;

            try
            {
                bitmap = new Bitmap(width, height, 4 * width, System.Drawing.Imaging.PixelFormat.Format32bppArgb, BitmapDataHandle.AddrOfPinnedObject());
            }
            finally
            {
                BitmapDataHandle.Free();
            }

            pictureBoxSample.Image = bitmap;
        }
    }
}
