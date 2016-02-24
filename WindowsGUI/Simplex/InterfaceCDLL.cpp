
#include "InterfaceCDLL.h"

#include "..\..\VM\vm.h"
#include "..\..\VM\kernel.h"
#include "..\..\VM\instruction.h"

#include "..\..\Expression\NoiseBuilder.h"
#include "..\..\Expression\NoiseParser.h"

#include <iostream>

#include <Windows.h>// for OutputDebugString, BSTR

int32 FillSampleArea(int32 offsetX, int32 offsetY, int32 width, int32 height, uint32* DataDest, int32 DataDestSize)
{
	if (width <= 0 || height <= 0)
		return 1;

	//double pct;
	//for (int32 i = 0; i < width * height; ++i)
	//{
	//	pct = (double)i / (double)(width * height);
	//	DataDest[i] = (uint32)(pct * UINT32_MAX);
	//}


	//anl::CKernel kernel;
	//anl::CNoiseExecutor vm(kernel);

	//anl::CInstructionIndex ii = kernel.gradientBasis(kernel.constant(0), kernel.seed(12345));
	//anl::CInstructionIndex ii2 = kernel.simplexBasis(kernel.seed(12345));

	//ii = kernel.scaleDomain(ii, kernel.constant(0.5));
	////ii = kernel.scaleX(ii, kernel.constant(0.5));
	//ii = kernel.multiply(ii, kernel.constant(0.5));
	//ii2 = kernel.multiply(ii2, kernel.constant(0.5));
	//ii = kernel.add(ii, ii2);

	//auto noise = 0.5 * GradientBasis(kernel, BLEND_NONE, 12345)[0.5][1.5] + 0.5 * SimplexBasis(kernel, 12345)(0.5);
	//anl::CInstructionIndex ii = noise.GetIndex();

	anl::NoiseParser parser("0.5 * GradientBasis(BLEND_NONE, 12345)[1.0][0.1]");
	if (parser.Parse() == false)
	{
		//std::cout << parser.FormErrorMsgs() << std::endl;
		OutputDebugStringA(parser.FormErrorMsgs().c_str());
		return -1;
	}


	double pct;
	anl::CNoiseExecutor& vm = parser.GetVM();
	anl::CInstructionIndex ii = parser.GetParseResult();
	for (int32 y = 0; y < height; ++y)
	{
		for (int32 x = 0; x < width; ++x)
		{
			//pct = (double)i / (double)(width * height);
			pct = vm.evaluateScalar(x * 0.05, y * 0.5, ii);
			pct += 1.0;
			pct /= 2;
			DataDest[y * width + x] = (uint32)(pct * UINT32_MAX);
		}
	}

	return 0;
}

BSTR CStrToBSTR(const char* str)
{
	BSTR bstr;
	int sizeNeeded = ::MultiByteToWideChar(CP_UTF8, 0, str, (int)strlen(str), NULL, 0);
	wchar_t* wstr = new wchar_t[sizeNeeded+1];
	::MultiByteToWideChar(CP_UTF8, 0, str, (int)strlen(str), wstr, sizeNeeded);
	wstr[sizeNeeded] = L'\0';
	bstr = ::SysAllocString(wstr);
	delete[] wstr;
	return bstr;
}

// returns non zero for error
int32 MapExpressionToArea(int32 offsetX, int32 offsetY, int32 width, int32 height, const char* expression, BSTR* ErrorMsg, uint32* DataDest, int32 DataDestSize, int32* TotalFolds, int32* TotalInstructions)
{
	*ErrorMsg = nullptr;
	if (width <= 0 || height <= 0)
	{
		*ErrorMsg = CStrToBSTR("witdth and height must be postivie numbers");
		return 1;
	}

	if (width * height > DataDestSize)
	{
		*ErrorMsg = CStrToBSTR("DataDestSize too small for given width, height.");
		return 2;
	}

	anl::NoiseParser parser(expression);

	if (parser.Parse() == false)
	{
		//std::cout << parser.FormErrorMsgs() << std::endl;
		std::string err = parser.FormErrorMsgs();
		OutputDebugStringA(err.c_str());
		*ErrorMsg = CStrToBSTR(err.c_str());
		return -1;
	}

	*TotalFolds = parser.GetTotalFolds();
	*TotalInstructions = parser.GetTotalInstructions();

	anl::CNoiseExecutor& vm = parser.GetVM();
	anl::CInstructionIndex ii = parser.GetParseResult();
	double pct;
	double scaleX = 1.0 / width;
	double scaleY = 1.0 / height;
	for (int32 y = 0; y < height; ++y)
	{
		for (int32 x = 0; x < width; ++x)
		{
			//pct = (double)i / (double)(width * height);
			pct = vm.evaluateScalar(offsetX + x * scaleX, offsetY + y * scaleY, ii);
			pct /= 2.0;
			pct += 0.5;
			DataDest[y * width + x] = (uint32)(pct * UINT32_MAX);
		}
	}
	return 0;
}


