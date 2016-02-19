#pragma once

#include <string>
#include <vector>

/*
letter ::= [a-zA-Z]
keyword ::= letter+
digit ::= [0-9]
number ::= digit+ '.'* digit*
axisScalar ::= '[' number ']'
domainScalar ::= '(' number ')'
argument ::= number | keyword
argumentList ::= argument (',' argumentList)*
functionCall ::= keyword '(' argumentList* ')'
object ::= (functionCall | number)
multiply ::= object '*' object
add ::= (object | multiply) '+' (object | multiply)
sub ::= (object | multiply) '-' (object | multiply)
expression ::= (object | add | sub | multiply | domainScalar | axisScalar)
*/

class NoiseBuilderParser
{
public:
	typedef std::string NoiseString;

private:

	struct ArgumentValue {
		NoiseString Keyword;
		double Number;
		bool IsNumber;
	};

	struct FunctionCall {
		NoiseString Keyword;
		std::vector<ArgumentValue> Args;
	};

	NoiseString String;
	int ParseIndex;

	std::vector<NoiseString> ErrorMsgs;

public:
	NoiseBuilderParser(const NoiseString& str)
		: ParseIndex(0)
	{
		// remove all whitespace, check for invalid characters and transfer to String.
		for each (char c in str)
		{
			if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
			{
				if (isalnum(c))
				{
					String += c;
				}
				else
				{
					SetError("Encountered non alpha-numeric character in string");
				}
			}
		}
	}

	~NoiseBuilderParser();


private:

	void SetError(NoiseString err)
	{
		ErrorMsgs.push_back(err);
	}

	bool IsEnd()
	{
		if (String.length() <= ParseIndex)
			return true;
		else
			return false;
	}

	bool letter(char& result)
	{
		result = '\0';
		if (IsEnd())
			return false;

		// is our current index a letter? If so then return the letter and advance index
		char c = String[ParseIndex];
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
		{
			result = c;
			ParseIndex++;
			return true;
		}
		return false;
	}

	bool keyword(NoiseString& str)
	{
		str = "";
		if (IsEnd())
			return false;

		char c;
		while (letter(c))
		{
			str += c;
		}
		if (str.length() != 0)
			return true;
		return false;
	}

	bool digit(char& d)
	{
		d = '\0';
		if (IsEnd())
			return false;

		char c = String[ParseIndex];
		if (c >= '0' && c <= '9')
		{
			d = c;
			ParseIndex++;
			return true;
		}
		return false;
	}

	bool number(double& number)
	{
		number = 0.0;

		char d;
		int integralCount = 0;
		while (digit(d))
		{
			number = number * integralCount * 10.0 + (int)(d - '0');
			integralCount++;
		}

		if (integralCount == 0)
			return false;

		if (IsEnd())
			return true;

		if (String[ParseIndex] == '.')
		{
			// consume the decimal
			ParseIndex++;
		}
		else
		{
			return true;
		}

		// if we made it here we need atleast one digit for the fractional portion
		int fractionalCount = 0;
		while (digit(d))
		{
			number = number + (double)(d - '0') / (10.0 * (fractionalCount + 1));
			fractionalCount++;
		}

		if (fractionalCount == 0)
		{
			SetError("Missing digit after decimal in number");
			return false;
		}

		return true;
	}

	bool axisScalar(double& scalar)
	{
		scalar = 1.0;
		if (IsEnd())
			return false;

		char c = String[ParseIndex];
		if (c == '[')
		{
			ParseIndex++;
		}
		else
		{
			return false;
		}

		double n;
		if (number(n))
		{
			scalar = n;
		}
		else
		{
			SetError("Missing number for axis scalar");
			return false;
		}

		if (IsEnd())
		{
			SetError("Unexpected End during axis scalar");
			return false;
		}

		c = String[ParseIndex];
		if (c == ']')
		{
			ParseIndex++;
		}
		else
		{
			SetError("Missing ']' for axis scalar");
			return false;
		}

		return true;
	}

	bool domainScalar(double& scalar)
	{
		scalar = 1.0;
		if (IsEnd())
			return false;

		char c = String[ParseIndex];
		if (c == '(')
		{
			ParseIndex++;
		}
		else
		{
			return false;
		}

		if (number(scalar) == false)
		{
			SetError("Missing number for domain scalar");
			return false;
		}

		if (IsEnd())
		{
			SetError("Unexpected end in domain scalar");
			return false;
		}

		c = String[ParseIndex];
		if (c == ')')
		{
			ParseIndex++;
		}
		else
		{
			SetError("Missing ')' in domain scalar");
			return false;
		}

		return true;
	}

	bool argument(ArgumentValue& argV)
	{
		argV.IsNumber = true;
		argV.Number = 0.0;
		argV.Keyword = "";

		if (number(argV.Number))
		{
			argV.IsNumber = true;
			return true;
		}
		else if (keyword(argV.Keyword))
		{
			argV.IsNumber = false;
			return true;
		}
		else
		{
			return false;
		}
	}

	bool argumentList(std::vector<ArgumentValue>& args)
	{
		ArgumentValue av;
		if (argument(av))
		{
			args.push_back(av);
			
			if (IsEnd())
				return true;

			char c = String[ParseIndex];
			if (c == ',')
			{
				ParseIndex++;
				if (argumentList(args) == false)
				{
					SetError("Missing Argument after ',' in arguement list");
					return false;
				}
			}
			return true;
		}
		else
		{
			return false;
		}
	}

	bool functionCall()
	{
		FunctionCall fc;
		fc.Args.clear();
		fc.Keyword = "";

		// save our position incase this is not a FunctionCall after all
		int rewindPoint = ParseIndex;

		if (keyword(fc.Keyword) == false)
		{
			return false;
		}

		if (IsEnd())
		{
			// was not a function call, rewind to save point
			ParseIndex = rewindPoint;
			return false;
		}

		char c = String[ParseIndex];
		if (c == '(')
		{
			// consume
			ParseIndex++;
		}
		else
		{
			// was not a function call, rewind to save point
			ParseIndex = rewindPoint;
			return false;
		}

		// we don't care at this point if there are actually any arguments
		// since we are looking for zero or more arguments
		argumentList(fc.Args);

		if (IsEnd())
		{
			SetError("Unexpected end, missing closing ')' in function call");
			return false;
		}

		c = String[ParseIndex];
		if (c == ')')
		{
			ParseIndex++;
		}
		else
		{
			SetError("Missing closing ')' in function call");
			return false;
		}

		return true;
	}

	bool object()
	{
		double num;
		if (functionCall())
			return true;
		else if (number(num))
			return true;
		else
			return false;
	}

	bool multiply()
	{
		// multiply is allowed to rewind since it only works on basic types
		int rewindPoint = ParseIndex;

		if (object() == false)
			return false;

		if (IsEnd())
		{
			ParseIndex = rewindPoint;
			return false;
		}

		char c = String[ParseIndex];
		if (c == '*')
		{
			ParseIndex++;
		}
		else
		{
			ParseIndex = rewindPoint;
			return false;
		}

		if (object())
		{
			// TODO do multiply
		}
		else
		{
			SetError("Unexpected char after multiply (\'*\') operation");
			return false;
		}
	}

	bool add()
	{
		int rewindPoint = ParseIndex;

		if (multiply())
		{

		}
		else if (object())
		{

		}
		else
		{
			return false;
		}

		char c = String[ParseIndex];
		if (c == '+')
		{
			ParseIndex++;
		}
		else
		{
			SetError("Expected '+' in add");
			return false;
		}
	}

};

