#pragma once

#include "..\..\VM\vm.h"
#include "..\..\VM\kernel.h"
#include "..\..\VM\instruction.h"

#include "..\..\lang\NoiseBuilder.h"
#include "..\..\lang\NoiseParser.h"

#include <Windows.h> // for BSTR
#include <stdint.h>

// the following functions provide a C interface to allow .net applications to pinvoke to.

#define DLL_EXPORT __declspec(dllexport)

/************ CInstructionIndex ****************/
extern "C" DLL_EXPORT void CInstructionIndex_dtor(anl::CInstructionIndex* idx);

/************ Parser ****************/
extern "C" DLL_EXPORT anl::lang::NoiseParser* Parser_ctor(const char* expression);
extern "C" DLL_EXPORT void Parser_dtor(anl::lang::NoiseParser* parser);
// returns non-zero for true
extern "C" DLL_EXPORT int32_t Parser_Parse(anl::lang::NoiseParser* parser);
extern "C" DLL_EXPORT char* Parser_FormErrorMsgs(anl::lang::NoiseParser* parser);
extern "C" DLL_EXPORT int32_t Parser_GetTotalFolds(anl::lang::NoiseParser* parser);
extern "C" DLL_EXPORT int32_t Parser_GetTotalInstructions(anl::lang::NoiseParser* parser);
// the object pointed to by the returned CKernel has a lifetime tied to the parser, no need to free.
extern "C" DLL_EXPORT anl::CKernel* Parser_GetKernelRef(anl::lang::NoiseParser* parser);
// the CInstructionIndex returned must be freed by a call to CInstructionIndex_dtor
extern "C" DLL_EXPORT anl::CInstructionIndex* Parser_GetInstructionIndexRoot(anl::lang::NoiseParser* parser);

/************ CNoiseExecutor ****************/
extern "C" DLL_EXPORT anl::CNoiseExecutor* CNoiseExecutor_ctor(anl::CKernel* kernel);
extern "C" DLL_EXPORT void CNoiseExecutor_dtor(anl::CNoiseExecutor* vm);
extern "C" DLL_EXPORT double CNoiseExecutor_evaluateScalar2D(anl::CNoiseExecutor* vm, double x, double y, anl::CInstructionIndex* idx);
extern "C" DLL_EXPORT double CNoiseExecutor_evaluateScalar3D(anl::CNoiseExecutor* vm, double x, double y, double z, anl::CInstructionIndex* idx);
extern "C" DLL_EXPORT double CNoiseExecutor_evaluateScalar4D(anl::CNoiseExecutor* vm, double x, double y, double z, double w, anl::CInstructionIndex* idx);
extern "C" DLL_EXPORT double CNoiseExecutor_evaluateScalar6D(anl::CNoiseExecutor* vm, double x, double y, double z, double w, double u, double v, anl::CInstructionIndex* idx);
