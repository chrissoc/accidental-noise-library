using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace SimplexAutoExpression
{
    class SimplexImportDLL
    {
#if DEBUG
        protected const string DLL_NAME = "Simplexd.dll";
#else
        protected const string DLL_NAME = "Simplex.dll";
#endif

        [DllImport(DLL_NAME)]//[MarshalAs(UnmanagedType.LPArray)]
        public static extern int FillSampleArea(int offsetX, int offsetY, int width, int height, [In, Out] uint[] DataDest, int DataDestSize);

        [DllImport(DLL_NAME)]
        public static extern int MapExpressionToArea(int offsetX, int offsetY, int width, int height, [MarshalAs(UnmanagedType.LPStr)] string expression, [MarshalAs(UnmanagedType.BStr)] out string ErrorMsg, [In, Out] uint[] DataDest, int DataDestSize, out int TotalFolds, out int TotalInstructions);
    }
}
