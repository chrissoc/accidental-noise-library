
#include "NoiseParserInterfaceCDLL.h"

#include <iostream>

#include <Windows.h>// for OutputDebugString, BSTR




/************ CInstructionIndex ****************/
void CInstructionIndex_dtor(anl::CInstructionIndex* idx)
{
	delete idx;
}

/************ Parser ****************/
anl::NoiseParser* Parser_ctor(const char* expression)
{
	return new anl::NoiseParser(expression);
}

void Parser_dtor(anl::NoiseParser* parser)
{
	delete parser;
}
// returns non-zero for true
int32_t Parser_Parse(anl::NoiseParser* parser)
{
	return parser->Parse() ? 1 : 0;
}

char* Parser_FormErrorMsgs(anl::NoiseParser* parser)
{
	std::string msg = parser->FormErrorMsgs();
	char* str = (char*)CoTaskMemAlloc(msg.size() + 1);
	strcpy_s(str, msg.size() + 1, msg.c_str());
	return str;
}

int32_t Parser_GetTotalFolds(anl::NoiseParser* parser)
{
	return parser->GetTotalFolds();
}

int32_t Parser_GetTotalInstructions(anl::NoiseParser* parser)
{
	return parser->GetTotalInstructions();
}

anl::CKernel* Parser_GetKernelRef(anl::NoiseParser* parser)
{
	return &parser->GetKernel();
}

anl::CInstructionIndex* Parser_GetInstructionIndexRoot(anl::NoiseParser* parser)
{
	return new anl::CInstructionIndex(parser->GetParseResult());
}

/************ CNoiseExecutor ****************/
anl::CNoiseExecutor* CNoiseExecutor_ctor(anl::CKernel* kernel)
{
	return new anl::CNoiseExecutor(*kernel);
}

void CNoiseExecutor_dtor(anl::CNoiseExecutor* vm)
{
	delete vm;
}

double CNoiseExecutor_evaluateScalar2D(anl::CNoiseExecutor* vm, double x, double y, anl::CInstructionIndex* idx)
{
	return vm->evaluateScalar(x, y, *idx);
}

double CNoiseExecutor_evaluateScalar3D(anl::CNoiseExecutor* vm, double x, double y, double z, anl::CInstructionIndex* idx)
{
	return vm->evaluateScalar(x, y, z, *idx);
}

double CNoiseExecutor_evaluateScalar4D(anl::CNoiseExecutor* vm, double x, double y, double z, double w, anl::CInstructionIndex* idx)
{
	return vm->evaluateScalar(x, y, z, w, *idx);
}

double CNoiseExecutor_evaluateScalar6D(anl::CNoiseExecutor* vm, double x, double y, double z, double w, double u, double v, anl::CInstructionIndex* idx)
{
	return vm->evaluateScalar(x, y, z, w, u, v, *idx);
}

