



#include <vector>
#include <string>

#include "../VM/vm.h"
#include "../VM/kernel.h"
#include "../VM/instruction.h"

#include "NoiseParser.h"

#include <iostream>
#include <iomanip>

#include <cmath>


#ifdef _WIN32
#include <Windows.h>
#endif

const double RECOMENDED_TOLERANCE = (1.0 / (double)UINT32_MAX);

static int ErrorCount = 0;
static int TestCount = 0;

static const std::string ANSI_RED_BACKGROUND = "\x1b[41m";
static const std::string ANSI_GREEN_BACKGROUND = "\x1b[42m";
static const std::string ANSI_WHITE_FOREGROUND = "\x1b[37m";
static const std::string ANSI_COLOR_RESET = "\x1b[0m";

void outputGreenText(const std::string& str)
{
#ifdef _WIN32
	WORD wOldColorAttrs;
	HANDLE hStdout;
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;

	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(hStdout, &csbiInfo);

	wOldColorAttrs = csbiInfo.wAttributes;

	std::cout.flush();
	SetConsoleTextAttribute(hStdout, BACKGROUND_GREEN | BACKGROUND_INTENSITY);

	std::cout << str;
	std::cout.flush();

	SetConsoleTextAttribute(hStdout, wOldColorAttrs);
#else // _WIN32
	std::cout << ANSI_GREEN_BACKGROUND
		<< ANSI_WHITE_FOREGROUND
		<< str << ANSI_COLOR_RESET;
#endif // _WIN32
}

void outputRedText(const std::string& str)
{
#ifdef _WIN32
	WORD wOldColorAttrs;
	HANDLE hStdout;
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;

	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(hStdout, &csbiInfo);

	wOldColorAttrs = csbiInfo.wAttributes;

	std::cout.flush();
	SetConsoleTextAttribute(hStdout, BACKGROUND_RED | BACKGROUND_INTENSITY);

	std::cout << str;
	std::cout.flush();

	SetConsoleTextAttribute(hStdout, wOldColorAttrs);
#else // _WIN32
	std::cout << ANSI_RED_BACKGROUND
		<< ANSI_WHITE_FOREGROUND
		<< str << ANSI_COLOR_RESET;
#endif // _WIN32
}

static std::string EscapeCharacters(std::string str)
{
	static const std::string newline = "\\n";
	static const std::string carageReturn = "\\r";
	static const std::string tab = "\\t";
	for (int i = 0; i < str.length(); i++)
	{
		switch (str[i])
		{
		case '\n': str.erase(i); str.insert(i, "\\n"); break;
		case '\r': str.erase(i); str.insert(i, "\\r"); break;
		case '\t': str.erase(i); str.insert(i, "\\t"); break;
		default: break;
		}
	}
	return str;
}

static std::string ShortenFile(const std::string& str)
{
	if (str.length() < 10)
		return str;

	return str.substr(str.length() - 10);
}

static void SetTestResultError(const std::string& file, int line, const std::string& expression, double expected, double result)
{
	ErrorCount++;
	TestCount++;

	outputRedText("FAIL");
	std::cout << " " << ShortenFile(file) << " ln" << std::setw(3) << line
		<< " exp: \"" << EscapeCharacters(expression) << "\""
		<< " expected: " << std::setprecision(10) << expected
		<< " found: " << result << "\n";
}

static void SetTestParseError(const std::string& file, int line, const std::string& expression, const std::string& message)
{
	ErrorCount++;
	TestCount++;

	outputRedText("FAIL");
	std::cout << " " << ShortenFile(file) << " ln" << std::setw(3) << line
		<< " parse error: \"" << EscapeCharacters(expression) << "\""
		<< "\n****PARSE ERROR MESSAGE****\n"
		<< message
		<< "\n**** END ERROR MESSAGE ****\n";
}

static void SetTestNoParseErrorError(const std::string& file, int line, const std::string& expression)
{
	ErrorCount++;
	TestCount++;

	outputRedText("FAIL");
	std::cout << " " << ShortenFile(file) << " ln" << std::setw(3) << line
		<< " expected parse error: \"" << EscapeCharacters(expression) << "\"\n";
}

static void SetTestSuccess(const std::string& file, int line, const std::string& expression)
{
	TestCount++;

	outputGreenText("PASS");
	std::cout << " " << ShortenFile(file) << " ln" << std::setw(3) << line
		<< " exp: \"" << EscapeCharacters(expression) << "\"\n";
}

static void TestSimpleExpression(std::string file, int line, std::string expression, double expectedResult, double tolerance = RECOMENDED_TOLERANCE)
{
	anl::NoiseParser parser(expression);

	bool success = parser.Parse();
	if (!success)
	{
		SetTestParseError(file, line, expression, parser.FormErrorMsgs());
		return;
	}

	anl::CKernel& k = parser.GetKernel();
	anl::CNoiseExecutor vm(k);
	anl::CInstructionIndex index = parser.GetParseResult();

	anl::CCoordinate coords[] = {
		anl::CCoordinate(0, 0),
		anl::CCoordinate(0.5, 0.5),
		anl::CCoordinate(-0.5, 0.5),
		anl::CCoordinate(-0.5, -0.5),
		anl::CCoordinate(0.5, -0.5),
	};

	success = true;
	for (auto& c : coords)
	{
		double r = vm.evaluateAt(c, index).outfloat_;
		if (std::abs(r - expectedResult) > tolerance)
		{
			SetTestResultError(file, line, expression, expectedResult, r);
			success = false;
			break;
		}
	}

	if (success)
	{
		SetTestSuccess(file, line, expression);
	}
}

static void TestExpressionMultiple(std::string file, int line, std::string expression, double expectedResult[5], double tolerance = RECOMENDED_TOLERANCE)
{
	anl::NoiseParser parser(expression);

	bool success = parser.Parse();
	if (!success)
	{
		SetTestParseError(file, line, expression, parser.FormErrorMsgs());
		return;
	}

	anl::CKernel& k = parser.GetKernel();
	anl::CNoiseExecutor vm(k);
	anl::CInstructionIndex index = parser.GetParseResult();

	anl::CCoordinate coords[] = {
		anl::CCoordinate(0, 0),
		anl::CCoordinate(0.5, 0.5),
		anl::CCoordinate(-0.5, 0.5),
		anl::CCoordinate(-0.5, -0.5),
		anl::CCoordinate(0.5, -0.5),
	};

	success = true;
	for (int i = 0; i < 5; ++i)
	{
		double r = vm.evaluateAt(coords[i], index).outfloat_;
		if (std::abs(r - expectedResult[i]) > tolerance)
		{
			SetTestResultError(file, line, expression, expectedResult[i], r);
			success = false;
			break;
		}
	}

	if (success)
	{
		SetTestSuccess(file, line, expression);
	}
}

static void TestSimpleExpressionForParseError(std::string file, int line, std::string expression)
{
	anl::NoiseParser parser(expression);

	bool success = parser.Parse();

	if (success)
	{
		SetTestNoParseErrorError(file, line, expression);
	}
	else
	{
		SetTestSuccess(file, line, expression);
	}
}


#define TSE(...) TestSimpleExpression(__FILE__, __LINE__, __VA_ARGS__)
#define TEPE(...) TestSimpleExpressionForParseError(__FILE__, __LINE__, __VA_ARGS__)
#define TEM(...) TestExpressionMultiple(__FILE__, __LINE__, __VA_ARGS__)

int main(int argc, const char* args[])
{
	double rlist[5];

	TSE("3;", 3.0);
	TSE("3.0;", 3.0);
	TSE("3.000000;", 3.0);
	TSE("0003;", 3.0);
	TSE("0003.0;", 3.0);
	TEPE(".3;");
	TEPE("3;;");
	TSE("1 + 2;", 3.0);
	TSE("1 - 2;", -1.0);
	TEPE(";");
	TSE("-1;", -1.0);
	TSE("-1 + 1;", 0.0);
	TSE("1 + -1;", 0.0);
	TSE("-(1 + 1);", -2.0);
	TSE("1 * 1;", 1.0);
	TSE("-1 * 1;", -1.0);
	TSE("1 * -1;", -1.0);
	TSE("1 / 0;", NAN);
	TSE("1 / 2;", 0.5);
	TSE("-1 / 2;", -0.5);
	TSE("-1 / -2;", 0.5);
	TSE("// comment \n1;", 1.0);
	TSE("// comment \n\t1;", 1.0);
	TSE("// comment \n 1;", 1.0);
	TSE("/*// comment */\n1;", 1.0);
	TSE("/**/1;", 1.0);
	TSE("2/*c*/ - /**/1;", 1.0);
	TEPE("dx();");
	TEPE("dy();");
	TSE("pi;", atan(1) * 4);
	TSE("e;", std::exp(1.0));

	rlist[0] = 0.0;
	rlist[1] = 0.5;
	rlist[2] = -0.5;
	rlist[3] = -0.5;
	rlist[4] = 0.5;
	TEM("x();", rlist);

	rlist[0] = 0.0;
	rlist[1] = 0.5;
	rlist[2] = 0.5;
	rlist[3] = -0.5;
	rlist[4] = -0.5;
	TEM("y();", rlist);

	rlist[0] = -1;
	rlist[1] = -1;
	rlist[2] = -1;
	rlist[3] = -1;
	rlist[4] = -1;
	TEM("dx(x(), 0.5);", rlist);

	TSE("<s:2>22.0;", 22.0);
	TSE("<t:2>22.0;", 22.0);
	TSE("<tx:2>22.0;", 22.0);
	TSE("<ty:2>22.0;", 22.0);
	TSE("<sx:2>22.0;", 22.0);
	TSE("<sy:2>22.0;", 22.0);
	TSE("<sz:2>22.0;", 22.0);
	TSE("<sw:2>22.0;", 22.0);
	TSE("<su:2>22.0;", 22.0);
	TSE("<sv:2>22.0;", 22.0);
	TSE("<r:90,0,0,1>22.0;", 22.0);

	TEPE("<r:>22.0;");
	TEPE("<r:1>22.0;");
	TEPE("<r:1,2>22.0;");
	TEPE("<r:1,2,3>22.0;");
	TEPE("x(,);");

	rlist[0] = 0.0;
	rlist[1] = 1.0;
	rlist[2] = -1.0;
	rlist[3] = -1.0;
	rlist[4] = 1.0;
	TEM("<s:2>x();", rlist);

	rlist[0] = 0.0;
	rlist[1] = -1.0;
	rlist[2] = 1.0;
	rlist[3] = 1.0;
	rlist[4] = -1.0;
	TEM("<s:-2>x();", rlist);

	rlist[0] = 0.0;
	rlist[1] = -0.5;
	rlist[2] = -0.5;
	rlist[3] = 0.5;
	rlist[4] = 0.5;
	TEM("<r:pi/2,0,0,1>x();", rlist);

	TEPE("var2 = 5.0;");
	TEPE("var = varB = VarC = 5.0;");
	TSE("var = 5.0; var;", 5.0);
	TSE("var = 5.0; varb = 3.0; var;", 5.0);
	TSE("var = 5.0; varb = 3.0; varb;", 3.0);

	TSE("billowtest = 0.5;<s:5>billowtest;", 0.5);
	TEPE("a = 5; a= 6;");


	if (ErrorCount != 0)
	{
		std::cout << "Summary: " << ErrorCount << " test(s) ";
		outputRedText("FAILED");
		std::cout << " of " << TestCount << " tests." << std::endl;
		return 1;
	}
	else
	{ 
		std::cout << "Summary: " << TestCount << " tests ";
		outputGreenText("SUCCEDED");
		std::cout << " out of " << TestCount << " tests." << std::endl;
		return 0;
	}
}
