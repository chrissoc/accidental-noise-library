using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Security.Permissions;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace SimplexAutoExpression
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        Brush ExpressionPathDefaultBrush;
        FileSystemWatcher FileWatcher;
        string FileNameToWatch;
        string DirectoryToWatch;

        bool WaitingForThumbnail = false;
        string CurrentPendingExpression;

        System.Timers.Timer ResizeTimer = new System.Timers.Timer(1000);

        NoiseImageService ImageRenderService = new NoiseImageService();

        public MainWindow()
        {
            ResizeTimer.Elapsed += ResizeTimer_Elapsed;
            InitializeComponent();
        }

        private void SetExpressionFileNotFoundError(bool clear)
        {
            if(clear)
            {
                txtExpressionPath.BorderBrush = ExpressionPathDefaultBrush;
            }
            else
            {
                txtExpressionPath.BorderBrush = Brushes.Red;
            }
        }

        private void SetExpressionFileParseError(bool clear, string msg)
        {
            if (clear)
            {
                txtParseErrorMsg.Visibility = Visibility.Collapsed;
                txtParseErrorMsg.Text = "";
            }
            else
            {
                txtParseErrorMsg.Visibility = Visibility.Visible;
                txtParseErrorMsg.Text = msg;
            }
        }

        private void StartImageRender(string expression)
        {
            string parseError;

            // make sure we are not busy.
            ImageRenderService.AbortImageRender();

            ImageRenderService.RenderDoneCallback = RenderDoneCallback;
            CurrentPendingExpression = expression;

            imgImage.Source = null;

            int width = 50;
            int height = 50;
            height = width = Math.Min(width, height);
            lblDimensions.Content = width.ToString() + "x" + height.ToString();

            WaitingForThumbnail = true;
            if (ImageRenderService.StartRender(expression, out parseError, width, height) == false)
            {
                SetExpressionFileParseError(false, parseError);
            }
            else
            {
                SetExpressionFileParseError(true, string.Empty);
            }

            lblParseTime.Content = ImageRenderService.ElapsedParseTimeInMs().ToString();
            lblInstructionCount.Content = ImageRenderService.GetInstructionCount().ToString();
            lblConstFoldCount.Content = ImageRenderService.GetFoldCount().ToString();
        }

        private void RenderDoneCallback(float[] buffer, int width, int height)
        {
            if (width * height <= 0)
                return;
            this.Dispatcher.InvokeAsync(new Action(() =>
            {
                float min = buffer[0];
                float max = buffer[0];
                //System.Windows.Media.Imaging.BitmapSource bitmap = new BitmapSource();
                //System.Windows.Media.Imaging.BitmapImage bitmap = new BitmapImage();
                lblRenderTime.Content = ImageRenderService.ElapsedRenderTimeInMs().ToString();

                ushort[] colorBuffer = new ushort[buffer.Length * 3];
                bool checkLimits = chkShowLimits.IsChecked.Value;
                for(int i = 0, j = 0; i < buffer.Length; ++i, j += 3)
                {
                    if (buffer[i] < min)
                        min = buffer[i];
                    if (buffer[i] > max)
                        max = buffer[i];
                    if (checkLimits && buffer[i] > 1.0)
                    {
                        colorBuffer[j + 0] = ushort.MaxValue / 2;
                        colorBuffer[j + 1] = 0;
                        colorBuffer[j + 2] = 0;
                    }
                    else if (checkLimits && buffer[i] < 0.0)
                    {
                        colorBuffer[j + 0] = 0;
                        colorBuffer[j + 1] = ushort.MaxValue / 2;
                        colorBuffer[j + 2] = 0;
                    }
                    else
                    {
                        colorBuffer[j + 0] = (ushort)(buffer[i] * ushort.MaxValue);
                        colorBuffer[j + 1] = (ushort)(buffer[i] * ushort.MaxValue);
                        colorBuffer[j + 2] = (ushort)(buffer[i] * ushort.MaxValue);
                    }
                }

                // buffer was already adjusted by the NoiseImageService to be between 0.0 and 1.0,
                // but we want to report our min/max between -1.0 and  1.0.
                min = (min - 0.5f) * 2.0f;
                max = (max - 0.5f) * 2.0f;

                lblSceneMin.Content = min.ToString();
                if(min > 1.0f)
                    lblSceneMin.Foreground = Brushes.Red;
                else if(min >= 0.0f)
                    lblSceneMin.Foreground = Brushes.Black;
                else if(min >= -1.0f)
                    lblSceneMin.Foreground = Brushes.Orange;
                else
                    lblSceneMin.Foreground = Brushes.Purple;

                lblSceneMax.Content = max.ToString();
                if (max > 1.0f)
                    lblSceneMax.Foreground = Brushes.Red;
                else if (max >= 0.0f)
                    lblSceneMax.Foreground = Brushes.Black;
                else if (max >= -1.0f)
                    lblSceneMax.Foreground = Brushes.Orange;
                else
                    lblSceneMax.Foreground = Brushes.Purple;

                imgImage.Source = BitmapSource.Create(width, height, 72, 72, PixelFormats.Rgb48, BitmapPalettes.Gray256, colorBuffer, width * sizeof(ushort) * 3);
                if (WaitingForThumbnail)
                {
                    string parseError;
                    WaitingForThumbnail = false;
                    // then we must have just assigned the thumbnail to the imgImage.
                    // so now make the fullsize one.
                    int w = (int)this.RenderSize.Width;//.Width;
                    int h = (int)this.RenderSize.Height - 75;//.Height - 75;
                    h = w = Math.Min(w, h);
                    lblDimensions.Content = w.ToString() + "x" + h.ToString();
                    
                    if (ImageRenderService.StartRender(CurrentPendingExpression, out parseError, w, h) == false)
                    {
                        SetExpressionFileParseError(false, parseError);
                    }
                    else
                    {
                        SetExpressionFileParseError(true, string.Empty);
                    }
                }
            }));
        }
        // returns true on success
        private bool WatchExpressionFile(string file)
        {
            string dir = System.IO.Path.GetDirectoryName(file);
            FileNameToWatch = System.IO.Path.GetFileName(file);
            if (dir != DirectoryToWatch || FileWatcher == null)
            {
                DirectoryToWatch = dir;
                FileWatcher = new FileSystemWatcher(DirectoryToWatch);
                FileWatcher.Changed += FileWatcherChanged;
                FileWatcher.Created += FileWatcherChanged;
                FileWatcher.Deleted += FileWatcherChanged;
                FileWatcher.EnableRaisingEvents = true;
                //FileWatcher.Renamed += FileWatcherChanged;
                FileWatcher.Error += FilwWatcher_Error;

                // start the first render ourself
                TimedReadAllTextAndRender(DirectoryToWatch + System.IO.Path.DirectorySeparatorChar + FileNameToWatch);
            }
            return false;
        }

        private void FilwWatcher_Error(object sender, ErrorEventArgs e)
        {
            MessageBox.Show("FileWatcher Error. " + e.GetException().Message);
        }

        private void TimedReadAllTextAndRender(string filePath)
        {
            bool done = false;
            int TimeLeftToWait = 10;// in 100s of ms
            while (!done && TimeLeftToWait > 0)
            {
                try
                {
                    StartImageRender(System.IO.File.ReadAllText(filePath));
                    done = true;
                }
                catch(UnauthorizedAccessException)
                {
                    SetExpressionFileNotFoundError(false);
                    return;
                }
                catch (FileNotFoundException)
                {
                    SetExpressionFileNotFoundError(false);
                    return;
                }
                catch (IOException e)
                {
                    const int ERROR_SHARING_VIOLATION = unchecked((int)0x80070020);
                    if (e.HResult != ERROR_SHARING_VIOLATION)
                    {
                        throw e;
                    }
                    else
                    {
                        // otherwise the file was in use, so we should just wait a second and see if its still in use.
                        System.Threading.Thread.Sleep(100);
                        TimeLeftToWait -= 1;
                    }
                }
            }
         }

        private void FileWatcherChanged(object sender, FileSystemEventArgs e)
        {
            if (e.Name == FileNameToWatch)
            {
                this.Dispatcher.InvokeAsync(new Action(() => {
                    if (e.ChangeType == WatcherChangeTypes.Changed)
                    {
                        TimedReadAllTextAndRender(DirectoryToWatch + System.IO.Path.DirectorySeparatorChar + FileNameToWatch);
                    }
                    else if(e.ChangeType == WatcherChangeTypes.Deleted)
                    {
                        SetExpressionFileNotFoundError(false);
                    }
                    else if(e.ChangeType == WatcherChangeTypes.Created || e.ChangeType == WatcherChangeTypes.Renamed)
                    {
                        SetExpressionFileNotFoundError(true);
                        TimedReadAllTextAndRender(DirectoryToWatch + System.IO.Path.DirectorySeparatorChar + FileNameToWatch);
                    }
                }));
            }
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            ExpressionPathDefaultBrush = txtExpressionPath.BorderBrush;
        }

        private void btnBrowse_Click(object sender, RoutedEventArgs e)
        {
            var ofd = new OpenFileDialog();
            var found = ofd.ShowDialog();
            if(found != null && found == true)
            {
                txtExpressionPath.Text = ofd.FileName;
            }
        }

        private void txtExpressionPath_TextChanged(object sender, TextChangedEventArgs e)
        {
            if(File.Exists(txtExpressionPath.Text) == false)
            {
                SetExpressionFileNotFoundError(false);
            }
            else
            {
                SetExpressionFileNotFoundError(true);

                WatchExpressionFile(txtExpressionPath.Text);
            }
        }

        private void Window_Closed(object sender, EventArgs e)
        {
            
        }

        private void Window_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            ResizeTimer.Stop();
            ResizeTimer.Start();
        }

        private void ResizeTimer_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            ResizeTimer.Stop();
            this.Dispatcher.InvokeAsync(new Action(() =>
            {
                TimedReadAllTextAndRender(DirectoryToWatch + System.IO.Path.DirectorySeparatorChar + FileNameToWatch);
            }));
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            ImageRenderService.AbortImageRender();
        }

        private void chkShowLimits_Checked(object sender, RoutedEventArgs e)
        {
            TimedReadAllTextAndRender(DirectoryToWatch + System.IO.Path.DirectorySeparatorChar + FileNameToWatch);
        }
    }
}
