#pragma once


#include "SimplexTypes.h"
#include "Simplex.h"
#include <Windows.h> // for BSTR


// returns non-zero on failure
extern "C" DLL_EXPORT int32 FillSampleArea(int32 offsetX, int32 offsetY, int32 width, int32 height, uint32* DataDest, int32 DataDestSize);
extern "C" DLL_EXPORT int32 MapExpressionToArea(int32 offsetX, int32 offsetY, int32 width, int32 height, const char* expression, BSTR* ErrorMsg, uint32* DataDest, int32 DataDestSize, int32* TotalFolds, int32* TotalInstructions);


