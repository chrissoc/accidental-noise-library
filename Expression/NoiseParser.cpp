#include "NoiseParser.h"

#include <math.h>
#include <sstream>

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
		RewindIndex = RewindIndex2;
		RewindIndex2 = -1;
	}

	void NoiseParser::Tokenizer::SetUnGetLocation()
	{
		RewindIndex2 = RewindIndex;
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
		{
			token.token = Token::TOKEN_EOF;
			return token;
		}

		while (true)
		{
			if (IsEof())
			{
				token.token = Token::TOKEN_EOF;
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
			case '<':
				token.token = Token::L_CHEVRON;
				return token;
			case '>':
				token.token = Token::R_CHEVRON;
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
			case ';':
				token.token = Token::SEMI_COLON;
				return token;
			case '=':
				token.token = Token::ASSIGNMENT;
				return token;
			case '/':
				// peek ahead to see if this is a block comment
				if (IsEof() == false && Data[DataIndex] == '*')
				{
					DataIndex++;
					// block comment, gobble untill end
					bool foundAsterisk = false;
					bool done = false;
					while (IsEof() == false && !done)
					{
						if (Data[DataIndex] == '*')
						{
							DataIndex++;
							if (IsEof() == false && Data[DataIndex] == '/')
							{
								done = true;
								DataIndex++;
							}
						}
						else
						{
							DataIndex++;
						}
					}
				}
				else if (IsEof() == false && Data[DataIndex] == '/')
				{
					DataIndex++;
					// line comment, gobble untill end
					while (IsEof() == false && Data[DataIndex] != '\n')
						DataIndex++;
					// gobble the newline as well
					if (IsEof() == false)
						DataIndex++;
				}
				else
				{
					token.token = Token::DIV;
					return token;
				}
				break;
			default:
				while (true)
				{
					if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
					{
						if (IntegralDigitCount || FractionalDigitCount)
						{
							SetError("Unexpected letter found within a number");
							token.token = Token::TOKEN_ERROR;
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
							token.token = Token::TOKEN_ERROR;
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
							token.token = Token::TOKEN_ERROR;
							return token;
						}

						if (IntegralDigitCount == 0)
						{
							SetError("numbers must start with a digit, not a '.'");
							token.token = Token::TOKEN_ERROR;
							return token;
						}
						else if (FractionalDigitCount != 0)
						{
							SetError("Too many '.' in number");
							token.token = Token::TOKEN_ERROR;
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
						token.token = Token::TOKEN_ERROR;
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

	NoiseParser::~NoiseParser()
	{
		for (auto p : Variables)
		{
			delete p.second;
		}
	}

	void NoiseParser::SetError(ParseString msg, const Token& cause)
	{
		std::stringstream ss;
		if (tokens.IsError())
		{
			ss << "Tokenizer error: " << tokens.GetLastError() << ". ";
		}
		ss << "Parse Error ln:" << cause.tokenLocation << " - " << msg;
		ErrorMsgs.push_back(ss.str());
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

	bool NoiseParser::KeywordToVariable(CInstructionIndex& instruction, const ParseString& keyword)
	{
		// check constants first
		CInstructionIndex newInstruction(instruction);
		if (keyword == "pi")
			newInstruction = Kernel.pi();
		else if (keyword == "e")
			newInstruction = Kernel.e();

		if (newInstruction != instruction)
		{
			instruction = newInstruction;
			return true;// found the constant
		}

		auto var = Variables.find(keyword);
		if (var != Variables.end())
		{
			instruction = *var->second;
			return true;
		}
		else
		{
			return false;
		}
	}

	void NoiseParser::AddVariable(const ParseString& keyword, CInstructionIndex& value)
	{
		auto var = Variables.find(keyword);
		if (var != Variables.end())
		{
			ParseString msg = "variable initilized multiple times: ";
			msg += keyword;
			SetError(msg);
			return;
		}

		// CInstructionIndex's private constructor prevents us from storing the class within std::map directly
		Variables[keyword] = new CInstructionIndex(value);
	}

	bool NoiseParser::axisScalar(CInstructionIndex& instruction)
	{
		Token t = tokens.GetToken();
		if (t.token != Token::L_BRACKET)
		{
			tokens.UnGet();
			return false;
		}

		if (expression(instruction) == false)
		{
			SetError("Missing expression from axis scalar - Empty []");
			return false;
		}

		t = tokens.GetToken();
		if (t.token != Token::R_BRACKET)
		{
			SetError("Missing closing bracket in axis scalar", t);
			return false;
		}

		return true;
	}

	bool NoiseParser::domainScalar(CInstructionIndex& instruction)
	{
		Token t = tokens.GetToken();
		if (t.token != Token::L_CHEVRON)
		{
			tokens.UnGet();
			return false;
		}

		if (expression(instruction) == false)
		{
			SetError("Missing expression for domain scalar - empty <>");
			return false;
		}

		t = tokens.GetToken();
		if (t.token != Token::R_CHEVRON)
		{
			SetError("Missing closing '>' in domain scalar", t);
			return false;
		}

		return true;
	}

	bool NoiseParser::argumentList(CInstructionIndex args[], int argc, int& argsFound)
	{
		if (argc == 0)
		{
			SetError("More arguments specified than supported");
			return false;
		}

		if (expression(args[0]))
		{
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
			// not a known function, it had better be a constant or a variable
			//SetError("Unexpected 'function' name", t);
			tokens.UnGet();
			return false;
		}

		t = tokens.GetToken();
		if (t.token != Token::L_PAREN)
		{
			//SetError("Missing '(' after keyword for 'function call'", t);
			tokens.UnGet();// put back the '('
			tokens.UnGet();// put back the keyword
			return false;
		}

		const int argc = 10;
		CInstructionIndex args[argc] = { NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP };
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
			instruction = Kernel.gradientBasis(args[0], args[1]);
			return true;
		case FUNC_SIMPLEX_BASIS:
			if (argsFound != 1)
			{
				SetError("SimplexBasis accepts 1 arguemnt");
				return false;
			}
			instruction = Kernel.simplexBasis(args[0]);
			return true;
		default:
			SetError("Unkown function type", funcToken);
			return false;
		}
	}

	bool NoiseParser::object(CInstructionIndex& instruction)
	{
		if (functionCall(instruction))
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
		else if (t.token == Token::KEYWORD)
		{
			if (KeywordToVariable(instruction, t.keyword))
				return true;
			else
				return false;
		}
		else
		{
			tokens.UnGet();
			return false;
		}
	}

	bool NoiseParser::scalar(CInstructionIndex& instruction)
	{
		CInstructionIndex scalar[6+1] = { NOP, NOP, NOP, NOP, NOP, NOP, NOP };

		bool foundDomainScale = false;
		int axisCount = 0;
		while (axisScalar(scalar[axisCount]))
		{
			axisCount++;
			if (axisCount == 7)
				SetError("Too many axis scalars");
		}
			
		if (axisCount == 0)
		{
			if (domainScalar(scalar[0]))
				foundDomainScale = true;
		}

		if (object(instruction))
		{

		}
		else if (axisCount != 0 || foundDomainScale)
		{
			// error
			SetError("No object following unary operator scale, nothing to scale");
			return false;
		}
		else
		{
			// no error, just nothing found here
			return false;
		}

		for (int i = 0; i < axisCount; ++i)
		{
			switch (i)
			{
			case 0:
				instruction = Kernel.scaleX(instruction, scalar[i]);
				break;
			case 1:
				instruction = Kernel.scaleY(instruction, scalar[i]);
				break;
			case 2:
				instruction = Kernel.scaleZ(instruction, scalar[i]);
				break;
			case 3:
				// anl uses WUV instead of UVW
				instruction = Kernel.scaleW(instruction, scalar[i]);
				break;
			case 4:
				instruction = Kernel.scaleU(instruction, scalar[i]);
				break;
			case 5:
				instruction = Kernel.scaleV(instruction, scalar[i]);
				break;
			default:
				SetError("Too many axis scalars");
				return false;
			}
		}

		if (foundDomainScale)
		{
			instruction = Kernel.scaleDomain(instruction, scalar[0]);
		}

		return true;
	}

	bool NoiseParser::mult(CInstructionIndex& instruction)
	{
		if (scalar(instruction) == false)
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

	bool NoiseParser::statement(CInstructionIndex& instruction)
	{
		if (expression(instruction) == false)
			return false;

		Token t = tokens.GetToken();
		if (t.token != Token::SEMI_COLON)
		{
			SetError("Satement does not end in a semicolon", t);
			return false;
		}
		return true;
	}

	bool NoiseParser::assignment(CInstructionIndex& instruction)
	{
		ParseString variableName;
		CInstructionIndex variableValue(NOP);
		bool isAssignment = false;

		Token t = tokens.GetToken();
		if (t.token == Token::KEYWORD)
		{
			if (KeywordToBlend(t.keyword) == BLEND_INVALID && KeywordToFunc(t.keyword) == FUNC_INVALID && KeywordToVariable(variableValue, t.keyword) == false)
			{
				variableName = t.keyword;
				t = tokens.GetToken();
				if (t.token == Token::ASSIGNMENT)
				{
					isAssignment = true;
				}
				else
				{
					ParseString msg = "Unrecognized keyword: ";
					msg += variableName;
					SetError(msg);
					return false;
				}
			}
			else
			{
				tokens.UnGet();
			}
		}
		else
		{
			tokens.UnGet();
		}

		if (statement(instruction) == false)
		{
			if (isAssignment)
			{
				SetError("Missing statement following assignment operator");
				return false;
			}
			return false;
		}
		else
		{
			if(isAssignment)
				AddVariable(variableName, instruction);
			return true;
		}
	}

	bool NoiseParser::program(CInstructionIndex& instruction)
	{
		if (assignment(instruction) == false)
			return false;

		program(instruction);
		return true;
	}

	bool NoiseParser::Parse()
	{
		// return true if there was an expression found and no error
		bool result = program(ParseResult);

		Token t;
		if (IsEof(t) == false)
		{
			SetError("expression ended prematurly", t);
		}

		result = result && Error == false;
		return result;
	}

	// return true if at end of file, position will conatain the location of the token used to make the determination
	bool NoiseParser::IsEof(Token& token)
	{
		token = tokens.GetToken();
		if (token.token != Token::TOKEN_EOF)
		{
			tokens.UnGet();
			return false;
		}
		return true;
	}
}