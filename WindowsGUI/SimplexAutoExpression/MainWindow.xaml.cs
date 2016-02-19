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
                txtParseErrorMsg.Visibility = Visibility.Visible;
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
            lblParseTime.Content = ImageRenderService.ElapsedParseTimeInMs().ToString();
            lblInstructionCount.Content = ImageRenderService.GetInstructionCount().ToString();
            lblConstFoldCount.Content = ImageRenderService.GetFoldCount().ToString();
        }

        private void RenderDoneCallback(float[] buffer, int width, int height)
        {
            this.Dispatcher.InvokeAsync(new Action(() =>
            {
                //System.Windows.Media.Imaging.BitmapSource bitmap = new BitmapSource();
                //System.Windows.Media.Imaging.BitmapImage bitmap = new BitmapImage();
                lblRenderTime.Content = ImageRenderService.ElapsedRenderTimeInMs().ToString();
                imgImage.Source = BitmapSource.Create(width, height, 72, 72, PixelFormats.Gray32Float, BitmapPalettes.Gray256, buffer, width * sizeof(float));
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
    }
}
