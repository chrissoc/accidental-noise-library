using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace SimplexAutoExpression
{
    class NoiseImageService : IDisposable
    {
        Thread WorkerThreadPrimaryImage = null;

        string ExpressionToRender;
        bool DoneWithRender = true;
        bool AbortRequest = false;

        int width = 400;
        int height = 400;
        double XScale = 0.05;
        double YScale = 0.05;

        long ElapsedRenderTimeMs = 0;
        long ElapsedParseTimeMs = 0;
        int InstructionCount = 0;
        int FoldCount = 0;


        ANL.Parser Parser;

        public delegate void RenderComplete(float[] buffer, int width, int height);
        public RenderComplete RenderDoneCallback;

        public NoiseImageService()
        {
            

        }

        private static void PrimaryThreadStart(object self)
        {
            (self as NoiseImageService).PrimaryImageThread();
        }

        private void PrimaryImageThread()
        {
            string expression = ExpressionToRender;
            if (expression == null || expression == string.Empty)
            {
                DoneWithRender = true;
                return;
            }

            float[] buffer = new float[width * height];

            ElapsedRenderTimeMs = 0;

            System.Diagnostics.Stopwatch stopwatch = new System.Diagnostics.Stopwatch();
            stopwatch.Start();
            Parallel.For<ANL.CNoiseExecutor>(0, height,
                () =>
                {
                    return Parser.GetVMCopy();
                },
                (y, loopSate, taskVmCopy) =>
                {
                    if (AbortRequest)
                    {
                        loopSate.Stop();
                        return taskVmCopy;
                    }
                    var root = Parser.GetInstructionIndexRoot();
                    for (int x = 0; x < width; x += 1)
                    {
                        if (loopSate.ShouldExitCurrentIteration || AbortRequest)
                            return taskVmCopy;
                        double d = taskVmCopy.evaluateScalar(x * XScale, y * YScale, root);
                        d /= 2.0;
                        d += 0.5;
                        
                        buffer[y * width + x] = (float)d;
                    }
                    return taskVmCopy;
                },
                taskVmCopy =>
                {
                    // finally
                }
            );

            stopwatch.Stop();
            ElapsedRenderTimeMs = stopwatch.ElapsedMilliseconds;

            if (AbortRequest == false && RenderDoneCallback != null)
                RenderDoneCallback(buffer, width, height);

            DoneWithRender = true;
        }

        public long ElapsedRenderTimeInMs()
        {
            return ElapsedRenderTimeMs;
        }

        public long ElapsedParseTimeInMs()
        {
            return ElapsedParseTimeMs;
        }

        public int GetInstructionCount()
        {
            return InstructionCount;
        }

        public int GetFoldCount()
        {
            return FoldCount;
        }

        public void AbortImageRender()
        {
            if (DoneWithRender || WorkerThreadPrimaryImage == null)
                return;
            AbortRequest = true;
            ThreadState ts = WorkerThreadPrimaryImage.ThreadState;
            try
            {
                while(DoneWithRender == false)
                    WorkerThreadPrimaryImage.Join(50);
            }
            catch (ThreadStateException)
            {
            }
        }

        // returns true on succes
        public bool StartRender(string NoiseExpression, out string parseError, int imgWidth, int imgHeight)
        {
            parseError = string.Empty;
            ExpressionToRender = NoiseExpression;

            if (!DoneWithRender)
                AbortImageRender();

            width = imgWidth;
            height = imgHeight;
            XScale = 1.0 / (double)width;
            YScale = 1.0 / (double)height;
            Parser = new ANL.Parser(ExpressionToRender);

            ElapsedParseTimeMs = 0;
            System.Diagnostics.Stopwatch stopwatch = new System.Diagnostics.Stopwatch();
            stopwatch.Start();
            bool success = Parser.Parse();
            stopwatch.Stop();
            ElapsedParseTimeMs = stopwatch.ElapsedMilliseconds;
            InstructionCount = Parser.GetTotalInstructions();
            FoldCount = Parser.GetTotalFolds();

            if (success == false)
            {
                parseError = Parser.FormErrorMsgs();
                return false;
            }

            DoneWithRender = false;
            AbortRequest = false;
            WorkerThreadPrimaryImage = new Thread(PrimaryThreadStart);
            WorkerThreadPrimaryImage.Start(this);

            return true;
        }

        #region IDisposable Support
        private bool disposedValue = false; // To detect redundant calls

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    // TODO: dispose managed state (managed objects).
                    AbortImageRender();
                }

                // TODO: free unmanaged resources (unmanaged objects) and override a finalizer below.
                // TODO: set large fields to null.

                disposedValue = true;
            }
        }

        // TODO: override a finalizer only if Dispose(bool disposing) above has code to free unmanaged resources.
        // ~RenderNoiseImageService() {
        //   // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
        //   Dispose(false);
        // }

        // This code added to correctly implement the disposable pattern.
        public void Dispose()
        {
            // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
            Dispose(true);
            // TODO: uncomment the following line if the finalizer is overridden above.
            // GC.SuppressFinalize(this);
        }
        #endregion
    }
}
