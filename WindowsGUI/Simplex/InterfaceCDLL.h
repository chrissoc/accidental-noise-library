#pragma once

#include <Windows.h> // for BSTR
#include <stdint.h>


#define DLL_EXPORT __declspec(dllexport)

// returns non-zero on failure
extern "C" DLL_EXPORT int32_t FillSampleArea(int32_t offsetX, int32_t offsetY, int32_t width, int32_t height, uint32_t* DataDest, int32_t DataDestSize);
extern "C" DLL_EXPORT int32_t MapExpressionToArea(int32_t offsetX, int32_t offsetY, int32_t width, int32_t height, const char* expression, BSTR* ErrorMsg, uint32_t* DataDest, int32_t DataDestSize, int32_t* TotalFolds, int32_t* TotalInstructions);


