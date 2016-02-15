#include "NoiseParser.h"

#include <math.h>

namespace anl
{
	void NoiseParser::Tokenizer::SetError(ParseString msg)
	{
		LastError = msg;
	}

	bool NoiseParser::Tokenizer::IsEof()
	{
		if (DataIndex >= Data.length())
			return true;
		return false;
	}

	void NoiseParser::Tokenizer::UnGet()
	{
		if (RewindIndex >= 0)
			DataIndex = RewindIndex;
		RewindIndex = -1;
	}

	void NoiseParser::Tokenizer::SetUnGetLocation()
	{
		RewindIndex = DataIndex;
	}

	NoiseParser::Token NoiseParser::Tokenizer::GetToken()
	{
		LastError = "";

		Token token;
		token.token = Token::NONE;
		token.tokenLocation = DataIndex;
		token.number = 0.0;

		bool makingIntegral = false;
		bool makingFractional = false;
		int IntegralDigitCount = 0;
		int FractionalDigitCount = 0;

		SetUnGetLocation();

		if (IsEof())
			return token;

		while (true)
		{
			if (IsEof())
			{
				SetError("Unexpected EOF in tokenizer");
				token.token = Token::ERROR;
				return token;
			}

			char c = Data[DataIndex];
			token.tokenLocation = DataIndex;
			DataIndex++;
			switch (c)
			{
			case ' ':
			case '\t':
			case '\r':
			case '\n':
				// ignore whitespace
				break;
			case '+':
				token.token = Token::ADD;
				return token;
			case '-':
				token.token = Token::SUB;
				return token;
			case '*':
				token.token = Token::MULT;
				return token;
			case '(':
				token.token = Token::L_PAREN;
				return token;
			case ')':
				token.token = Token::R_PAREN;
				return token;
			case '[':
				token.token = Token::L_BRACKET;
				return token;
			case ']':
				token.token = Token::R_BRACKET;
				return token;
			case ',':
				token.token = Token::COMMA;
				return token;
			default:
				while (true)
				{
					if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
					{
						if (IntegralDigitCount || FractionalDigitCount)
						{
							SetError("Unexpected letter found within a number");
							token.token = Token::ERROR;
							return token;
						}

						token.token = Token::KEYWORD;
						token.keyword += c;
					}
					else if (c >= '0' && c <= '9')
					{
						if (token.keyword.length())
						{
							SetError("Unexpected digit found withing keyword");
							token.token = Token::ERROR;
							return token;
						}

						if (IntegralDigitCount == 0 && FractionalDigitCount == 0)
							token.tokenLocation = DataIndex;

						token.token = Token::NUMBER;

						if (makingFractional == true)
						{
							token.number += (double)(c - '0') / (std::pow(10.0, FractionalDigitCount + 1));
							FractionalDigitCount++;
						}
						else
						{
							// we are making the integral then
							token.number = token.number * 10.0 + (double)(c - '0');
							IntegralDigitCount++;
						}
					}
					else if (c == '.')
					{
						if (token.keyword.length())
						{
							SetError("Unexpected '.' found withing keyword");
							token.token = Token::ERROR;
							return token;
						}

						if (IntegralDigitCount == 0)
						{
							SetError("numbers must start with a digit, not a '.'");
							token.token = Token::ERROR;
							return token;
						}
						else if (FractionalDigitCount != 0)
						{
							SetError("Too many '.' in number");
							token.token = Token::ERROR;
							return token;
						}
						else
						{
							makingFractional = true;
						}
					}
					else
					{
						if ((IntegralDigitCount && makingFractional == false) || (IntegralDigitCount && FractionalDigitCount))
						{
							// then we have finished forming the number,
							// unget the character used to determine the end
							DataIndex--;
							return token;
						}

						if (token.keyword.length())
						{
							// then we finished forming the keyword
							// unget the character used to determine the end
							DataIndex--;
							return token;
						}

						ParseString msg = "Unexpected character: ";
						msg += c;
						msg += " at index: ";
						msg += DataIndex;
						SetError(msg);
						token.token = Token::ERROR;
						return token;
					}
					c = Data[DataIndex];
					DataIndex++;
				}
				break;
			}
		}

	}




	/*********  NoiseParser **********************/

	void NoiseParser::SetError(ParseString msg, const Token& cause)
	{
		ParseString ErrorMsg;
		if (tokens.IsError())
		{
			ErrorMsg += "Tokenizer error: ";
			ErrorMsg += tokens.GetLastError();
		}
		ErrorMsg += "Parse Error ln:";
		ErrorMsg += cause.tokenLocation;
		ErrorMsg += " - ";
		ErrorMsg += msg;
		ErrorMsgs.push_back(msg);
		Error = true;
	}

	void NoiseParser::SetError(ParseString msg)
	{
		ErrorMsgs.push_back(msg);
		Error = true;
	}

	NoiseParser::ParseString NoiseParser::FormErrorMsgs()
	{
		ParseString msgs;

		if (Error == false)
		{
			msgs += "Success, noise expression parsed.";
		}
		else
		{
			for (auto msg : ErrorMsgs)
			{
				msgs += msg + "\n";
			}
		}

		return msgs;
	}

	NoiseParser::BlendType NoiseParser::KeywordToBlend(const ParseString& keyword)
	{
		BlendType t = BLEND_INVALID;
		if (keyword == "BLEND_NONE")
			t = BLEND_NONE;
		else if (keyword == "BLEND_LINEAR")
			t = BLEND_LINEAR;
		else if (keyword == "BLEND_HERMITE")
			t = BLEND_HERMITE;
		else if (keyword == "BLEND_QUINTIC")
			t = BLEND_QUINTIC;
		return t;
	}

	NoiseParser::Function NoiseParser::KeywordToFunc(const ParseString& keyword)
	{
		Function f = FUNC_INVALID;
		if (keyword == "SimplexBasis")
			f = FUNC_SIMPLEX_BASIS;
		else if (keyword == "GradientBasis")
			f = FUNC_GRADIENT_BASIS;
		return f;
	}

	bool NoiseParser::axisScalar(double& scale)
	{
		Token t = tokens.GetToken();
		if (t.token != Token::L_BRACKET)
		{
			tokens.UnGet();
			return false;
		}

		t = tokens.GetToken();
		if (t.token != Token::NUMBER)
		{
			SetError("Missing number from axisScalar", t);
			return false;
		}

		scale = t.number;

		t = tokens.GetToken();
		if (t.token != Token::R_BRACKET)
		{
			SetError("Missing closing bracket in axis scalar", t);
			return false;
		}

		return true;
	}

	bool NoiseParser::domainScalar(double& scale)
	{
		Token t = tokens.GetToken();
		if (t.token != Token::L_PAREN)
		{
			tokens.UnGet();
			return false;
		}

		t = tokens.GetToken();
		if (t.token != Token::NUMBER)
		{
			SetError("Missing number from domain scalar", t);
			return false;
		}

		scale = t.number;

		t = tokens.GetToken();
		if (t.token != Token::R_PAREN)
		{
			SetError("Missing closing ')' in domain scalar", t);
			return false;
		}

		return true;
	}

	bool NoiseParser::argument(double& result)
	{
		Token t = tokens.GetToken();
		if (t.token == Token::NUMBER)
		{
			result = t.number;
			return true;
		}
		else if (t.token == Token::KEYWORD)
		{
			BlendType bt = KeywordToBlend(t.keyword);

			if (bt == BLEND_INVALID)
			{
				SetError("Invalid blend type specified as argument", t);
				return false;
			}

			result = (double)bt;
			return true;
		}
		else
		{
			tokens.UnGet();
			return false;
		}
	}

	bool NoiseParser::argumentList(double args[], int argc, int& argsFound)
	{
		if (argc == 0)
		{
			SetError("More arguments specified than supported");
			return false;
		}

		double value;
		if (argument(value))
		{
			args[0] = value;
			argsFound += 1;

			Token t = tokens.GetToken();
			if (t.token == Token::COMMA)
				argumentList(args + 1, argc - 1, argsFound);
			else
				tokens.UnGet();
			return true;// return true, found atleast one.
		}
		else
		{
			return false;
		}
	}

	bool NoiseParser::functionCall(CInstructionIndex& instruction)
	{
		Token t = tokens.GetToken();
		if (t.token != Token::KEYWORD)
		{
			tokens.UnGet();
			return false;
		}

		Function func = KeywordToFunc(t.keyword);
		Token funcToken = t;
		if (func == FUNC_INVALID)
		{
			SetError("Unexpected 'function' name", t);
			return false;
		}

		t = tokens.GetToken();
		if (t.token != Token::L_PAREN)
		{
			SetError("Missing '(' after keyword for 'function call'", t);
			return false;
		}

		const int argc = 10;
		double args[argc];
		int argsFound = 0;
		argumentList(args, argc, argsFound);

		t = tokens.GetToken();
		if (t.token != Token::R_PAREN)
		{
			SetError("Missing matching ')' in 'function call'", t);
			return false;
		}

		// we now have the name of the function and all the arguments
		switch (func)
		{
		case FUNC_GRADIENT_BASIS:
			if (argsFound != 2)
			{
				SetError("GradientBasis accepts 2 arguemnts");
				return false;
			}
			instruction = Kernel.gradientBasis(Kernel.constant(args[0]), Kernel.seed(static_cast<unsigned int>(args[1])));
			return true;
		case FUNC_SIMPLEX_BASIS:
			if (argsFound != 1)
			{
				SetError("SimplexBasis accepts 1 arguemnt");
				return false;
			}
			instruction = Kernel.simplexBasis(Kernel.seed(static_cast<unsigned int>(args[1])));
			return true;
		default:
			SetError("Unkown function type", funcToken);
			return false;
		}
	}

	bool NoiseParser::scaledFunctionCall(CInstructionIndex& instruction)
	{
		if (functionCall(instruction) == false)
			return false;

		double scalar;
		if (domainScalar(scalar))
		{
			instruction = Kernel.scaleDomain(instruction, Kernel.constant(scalar));
			return true;
		}

		int axisCount = 0;
		while (axisScalar(scalar))
		{
			axisCount++;
			switch (axisCount)
			{
			case 1:
				instruction = Kernel.scaleX(instruction, Kernel.constant(scalar));
				break;
			case 2:
				instruction = Kernel.scaleY(instruction, Kernel.constant(scalar));
				break;
			case 3:
				instruction = Kernel.scaleZ(instruction, Kernel.constant(scalar));
				break;
			case 4:
				// anl uses WUV instead of UVW
				instruction = Kernel.scaleW(instruction, Kernel.constant(scalar));
				break;
			case 5:
				instruction = Kernel.scaleU(instruction, Kernel.constant(scalar));
				break;
			case 6:
				instruction = Kernel.scaleV(instruction, Kernel.constant(scalar));
				break;
			default:
				SetError("Too many axis scalars");
				return false;
			}
		}

		return true;
	}

	bool NoiseParser::object(CInstructionIndex& instruction)
	{
		if (scaledFunctionCall(instruction))
			return true;
		else if (grouping(instruction))
			return true;
		else if (negative(instruction))
			return true;

		Token t = tokens.GetToken();
		if (t.token == Token::NUMBER)
		{
			instruction = Kernel.constant(t.number);
			return true;
		}
		else
		{
			tokens.UnGet();
			return false;
		}
	}

	bool NoiseParser::mult(CInstructionIndex& instruction)
	{
		if (object(instruction) == false)
			return false;

		Token t = tokens.GetToken();
		if (t.token != Token::MULT)
		{
			tokens.UnGet();
			// not an error, we are just returning the object
			return true;
		}

		CInstructionIndex operandRight(instruction);
		if (mult(operandRight))
		{
			instruction = Kernel.multiply(instruction, operandRight);
			return true;
		}
		else
		{
			SetError("missing value to the right of the multiply operator", t);
			return false;
		}
	}

	bool NoiseParser::add(CInstructionIndex& instruction)
	{
		if (mult(instruction) == false)
			return false;

		Token t = tokens.GetToken();
		if (t.token != Token::ADD && t.token != Token::SUB)
		{
			tokens.UnGet();
			// not an error, we are just returning the object
			return true;
		}

		CInstructionIndex operandRight(instruction);
		if (add(operandRight))
		{
			if (t.token == Token::ADD)
				instruction = Kernel.add(instruction, operandRight);
			else
				instruction = Kernel.subtract(instruction, operandRight);
			return true;
		}
		else
		{
			SetError("missing value to the right of the +/- operator", t);
			return false;
		}
	}

	bool NoiseParser::grouping(CInstructionIndex& instruction)
	{
		Token t = tokens.GetToken();
		if (t.token != Token::L_PAREN)
		{
			tokens.UnGet();
			return false;
		}

		if (expression(instruction) == false)
		{
			SetError("Missing expression within grouping, empty ()", t);
			return false;
		}

		t = tokens.GetToken();
		if (t.token != Token::R_PAREN)
		{
			SetError("Missing closing parenthesis", t);
			return false;
		}
		return true;
	}

	bool NoiseParser::negative(CInstructionIndex& instruction)
	{
		Token t = tokens.GetToken();
		if (t.token != Token::SUB)
		{
			tokens.UnGet();
			return false;
		}

		if (expression(instruction) == false)
		{
			SetError("Attempting to negate nothing", t);
			return false;
		}

		SInstruction& i = (*Kernel.getKernel())[instruction.GetIndex()];
		if (i.opcode_ == OP_Constant)
		{
			// negate the constant directly
			i.outfloat_ = -i.outfloat_;
			return true;
		}
		else
		{
			// since there is no constant to directly negate, we must multiply by -1.0
			instruction = Kernel.multiply(instruction, Kernel.constant(-0.1));
			return true;
		}
	}

	bool NoiseParser::expression(CInstructionIndex& instruction)
	{
		if (grouping(instruction))
			return true;
		else if (negative(instruction))
			return true;
		else if (add(instruction))
			return true;
		else
			return false;
	}

	bool NoiseParser::Parse()
	{
		return expression(ParseResult);
	}
}