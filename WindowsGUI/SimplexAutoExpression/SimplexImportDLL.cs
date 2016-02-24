using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace SimplexAutoExpression.ANL
{
    class SimplexImportDLL
    {
#if DEBUG
        public const string DLL_NAME = "Simplexd.dll";
#else
        public const string DLL_NAME = "Simplex.dll";
#endif

        #region SimpleImport
        [DllImport(DLL_NAME)]//[MarshalAs(UnmanagedType.LPArray)]
        public static extern int FillSampleArea(int offsetX, int offsetY, int width, int height, [In, Out] uint[] DataDest, int DataDestSize);

        [DllImport(DLL_NAME)]
        public static extern int MapExpressionToArea(int offsetX, int offsetY, int width, int height, [MarshalAs(UnmanagedType.LPStr)] string expression, [MarshalAs(UnmanagedType.BStr)] out string ErrorMsg, [In, Out] uint[] DataDest, int DataDestSize, out int TotalFolds, out int TotalInstructions);
        #endregion
        
    }
    
    public class CInstructionIndex : IDisposable
    {
        protected IntPtr Index;

        // CInstructionIndex is responsible for freeing the IntPtr index
        public CInstructionIndex(IntPtr index)
        {
            Index = index;
        }

        public IntPtr GetIndex()
        {
            return Index;
        }

        [DllImport(SimplexImportDLL.DLL_NAME)]
        protected extern static void CInstructionIndex_dtor(IntPtr idx);

        #region IDisposable Support
        private bool disposedValue = false; // To detect redundant calls

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    // TODO: dispose managed state (managed objects).
                }

                // TODO: free unmanaged resources (unmanaged objects) and override a finalizer below.
                // TODO: set large fields to null.
                CInstructionIndex_dtor(Index);
                Index = IntPtr.Zero;

                disposedValue = true;
            }
        }

        // TODO: override a finalizer only if Dispose(bool disposing) above has code to free unmanaged resources.
        ~CInstructionIndex()
        {
            // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
            Dispose(false);
        }

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

    public class Parser : IDisposable
    {
        protected IntPtr ParserPtr;

        [DllImport(SimplexImportDLL.DLL_NAME)]
        protected extern static IntPtr Parser_ctor([MarshalAs(UnmanagedType.LPStr)] string expression);
        public Parser(string expression)
        {
            ParserPtr = Parser_ctor(expression);
        }

        [DllImport(SimplexImportDLL.DLL_NAME)]
        protected extern static int Parser_Parse(IntPtr parser);
        public bool Parse()
        {
            return Parser_Parse(ParserPtr) != 0;
        }

        [DllImport(SimplexImportDLL.DLL_NAME, CharSet = CharSet.Ansi)]
        protected extern static string Parser_FormErrorMsgs(IntPtr parser);
        public string FormErrorMsgs()
        {
            return Parser_FormErrorMsgs(ParserPtr);
        }

        [DllImport(SimplexImportDLL.DLL_NAME)]
        protected extern static int Parser_GetTotalFolds(IntPtr parser);
        public int GetTotalFolds()
        {
            return Parser_GetTotalFolds(ParserPtr);
        }

        [DllImport(SimplexImportDLL.DLL_NAME)]
        protected extern static int Parser_GetTotalInstructions(IntPtr parser);
        public int GetTotalInstructions()
        {
            return Parser_GetTotalInstructions(ParserPtr);
        }
        
        public CNoiseExecutor GetVMCopy()
        {
            return new CNoiseExecutor(this);
        }

        [DllImport(SimplexImportDLL.DLL_NAME)]
        protected extern static IntPtr Parser_GetKernelRef(IntPtr parser);
        public IntPtr GetKernelRef()
        {
            return Parser_GetKernelRef(ParserPtr);
        }

        [DllImport(SimplexImportDLL.DLL_NAME)]
        protected extern static IntPtr Parser_GetInstructionIndexRoot(IntPtr parser);
        public CInstructionIndex GetInstructionIndexRoot()
        {
            return new CInstructionIndex(Parser_GetInstructionIndexRoot(ParserPtr));
        }

        [DllImport(SimplexImportDLL.DLL_NAME)]
        protected extern static void Parser_dtor(IntPtr parser);

        #region IDisposable Support
        private bool disposedValue = false; // To detect redundant calls

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    // TODO: dispose managed state (managed objects).
                }

                // TODO: free unmanaged resources (unmanaged objects) and override a finalizer below.
                // TODO: set large fields to null.
                Parser_dtor(ParserPtr);
                ParserPtr = IntPtr.Zero;

                disposedValue = true;
            }
        }

        //TODO: override a finalizer only if Dispose(bool disposing) above has code to free unmanaged resources.
         ~Parser()
        {
            // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
            Dispose(false);
        }

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

    public class CNoiseExecutor : IDisposable
    {
        protected IntPtr VM;
        protected Parser ParserRef;// hold reference to parser to ensure the kernel stays alive.

        [DllImport(SimplexImportDLL.DLL_NAME)]
        protected extern static IntPtr CNoiseExecutor_ctor(IntPtr kernel);
        public CNoiseExecutor(Parser parser)
        {
            ParserRef = parser;
            VM = CNoiseExecutor_ctor(parser.GetKernelRef());
        }

        [DllImport(SimplexImportDLL.DLL_NAME)]
        protected extern static double CNoiseExecutor_evaluateScalar2D(IntPtr vm, double x, double y, IntPtr idx);
        public double evaluateScalar(double x, double y, CInstructionIndex idx)
        {
            return CNoiseExecutor_evaluateScalar2D(VM, x, y, idx.GetIndex());
        }

        [DllImport(SimplexImportDLL.DLL_NAME)]
        protected extern static double CNoiseExecutor_evaluateScalar3D(IntPtr vm, double x, double y, double z, IntPtr idx);
        public double evaluateScalar(double x, double y, double z, CInstructionIndex idx)
        {
            return CNoiseExecutor_evaluateScalar3D(VM, x, y, z, idx.GetIndex());
        }

        [DllImport(SimplexImportDLL.DLL_NAME)]
        protected extern static double CNoiseExecutor_evaluateScalar4D(IntPtr vm, double x, double y, double z, double w, IntPtr idx);
        public double evaluateScalar(double x, double y, double z, double w, CInstructionIndex idx)
        {
            return CNoiseExecutor_evaluateScalar4D(VM, x, y, z, w, idx.GetIndex());
        }

        [DllImport(SimplexImportDLL.DLL_NAME)]
        protected extern static double CNoiseExecutor_evaluateScalar6D(IntPtr vm, double x, double y, double z, double w, double u, double v, IntPtr idx);
        public double evaluateScalar(double x, double y, double z, double w, double u, double v, CInstructionIndex idx)
        {
            return CNoiseExecutor_evaluateScalar6D(VM, x, y, z, w, u, v, idx.GetIndex());
        }


        [DllImport(SimplexImportDLL.DLL_NAME)]
        protected extern static void CNoiseExecutor_dtor(IntPtr vm);

        #region IDisposable Support
        private bool disposedValue = false; // To detect redundant calls

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    // TODO: dispose managed state (managed objects).
                }

                // TODO: free unmanaged resources (unmanaged objects) and override a finalizer below.
                // TODO: set large fields to null.
                CNoiseExecutor_dtor(VM);
                VM = IntPtr.Zero;

                disposedValue = true;
            }
        }

        // TODO: override a finalizer only if Dispose(bool disposing) above has code to free unmanaged resources.
        ~CNoiseExecutor()
        {
            // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
            Dispose(false);
        }

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
