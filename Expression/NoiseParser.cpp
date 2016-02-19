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
		if (DataIndex >= (int)Data.length())
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
		if (keyword == "valueBasis")
			f = FUNC_VALUE_BASIS;
		else if (keyword == "gradientBasis")
			f = FUNC_GRADIENT_BASIS; 
		else if (keyword == "simplexBasis")
			f = FUNC_SIMPLEX_BASIS;
		else if (keyword == "cellularBasis")
			f = FUNC_CELLULAR_BASIS;
		else if (keyword == "max")
			f = FUNC_MAX;
		else if (keyword == "min")
			f = FUNC_MIN;
		else if (keyword == "abs")
			f = FUNC_ABS;
		else if (keyword == "pow")
			f = FUNC_POW;
		else if (keyword == "bias")
			f = FUNC_BIAS;
		else if (keyword == "gain")
			f = FUNC_GAIN;
		else if (keyword == "cos")
			f = FUNC_COS;
		else if (keyword == "sin")
			f = FUNC_SIN;
		else if (keyword == "tan")
			f = FUNC_TAN;
		else if (keyword == "acos")
			f = FUNC_ACOS;
		else if (keyword == "asin")
			f = FUNC_ASIN;
		else if (keyword == "atan")
			f = FUNC_ATAN;
		else if (keyword == "tiers")
			f = FUNC_TIERS;
		else if (keyword == "smoothTiers")
			f = FUNC_SMOOTH_TIERS;
		else if (keyword == "blend")
			f = FUNC_BLEND;
		else if (keyword == "select")
			f = FUNC_SELECT;
		else if (keyword == "simpleRidgedMultifractal")
			f = FUNC_SIMPLE_RIDGED_MULTIFRACTAL;
		else if (keyword == "simplefBm")
			f = FUNC_SIMPLE_FBM;
		else if (keyword == "simpleBillow")
			f = FUNC_SIMPLE_BILLOW;
		else if (keyword == "x")
			f = FUNC_X;
		else if (keyword == "y")
			f = FUNC_Y;
		else if (keyword == "z")
			f = FUNC_Z;
		else if (keyword == "w")
			f = FUNC_W;
		else if (keyword == "u")
			f = FUNC_U;
		else if (keyword == "v")
			f = FUNC_V;
		else if (keyword == "dx")
			f = FUNC_DX;
		else if (keyword == "dy")
			f = FUNC_DY;
		else if (keyword == "dz")
			f = FUNC_DZ;
		else if (keyword == "dw")
			f = FUNC_DW;
		else if (keyword == "du")
			f = FUNC_DU;
		else if (keyword == "dv")
			f = FUNC_DV;
		else if (keyword == "sigmoid")
			f = FUNC_SIGMOID;
		else if (keyword == "scaleOffset")
			f = FUNC_SCALE_OFFSET;
		else if (keyword == "radial")
			f = FUNC_RADIAL;
		else if (keyword == "clamp")
			f = FUNC_CLAMP;
		else if (keyword == "rgba")
			f = FUNC_RGBA;
		else if (keyword == "color")
			f = FUNC_COLOR;
		else if (keyword == "hexTile")
			f = FUNC_HEX_TILE;
		else if (keyword == "hexBump")
			f = FUNC_HEX_BUMP;
		return f;
	}

	bool NoiseParser::KeywordToVariable(CInstructionIndex& instruction, const ParseString& keyword)
	{
		// check constants first
		CInstructionIndex newInstruction(instruction);
		BlendType bt = KeywordToBlend(keyword);
		if (keyword == "pi")
			newInstruction = Kernel.pi();
		else if (keyword == "e")
			newInstruction = Kernel.e();
		else if (bt != BLEND_INVALID)
			newInstruction = Kernel.constant(bt);

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
		case FUNC_VALUE_BASIS:
			if (argsFound != 2)
			{
				SetError("valueBasis accepts 2 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.valueBasis(args[0], args[1]);
			break;
		case FUNC_GRADIENT_BASIS:
			if (argsFound != 2)
			{
				SetError("GradientBasis accepts 2 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.gradientBasis(args[0], args[1]);
			break;
		case FUNC_SIMPLEX_BASIS:
			if (argsFound != 1)
			{
				SetError("SimplexBasis accepts 1 arguemnt", funcToken);
				return false;
			}
			instruction = Kernel.simplexBasis(args[0]);
			break;
		case FUNC_CELLULAR_BASIS:
			if (argsFound != 10)
			{
				SetError("cellularBasis accepts 10 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.cellularBasis(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]);
			break;
		case FUNC_MAX:
			if (argsFound != 2)
			{
				SetError("max accepts 2 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.maximum(args[0], args[1]);
			break;
		case FUNC_MIN:
			if (argsFound != 2)
			{
				SetError("min accepts 2 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.minimum(args[0], args[1]);
			break;
		case FUNC_ABS:
			if (argsFound != 1)
			{
				SetError("abs accepts 1 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.abs(args[0]);
			break;
		case FUNC_POW:
			if (argsFound != 2)
			{
				SetError("pow accepts 2 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.pow(args[0], args[1]);
			break;
		case FUNC_BIAS:
			if (argsFound != 2)
			{
				SetError("bias accepts 2 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.bias(args[0], args[1]);
			break;
		case FUNC_GAIN:
			if (argsFound != 2)
			{
				SetError("gain accepts 2 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.gain(args[0], args[1]);
			break;
		case FUNC_COS:
			if (argsFound != 1)
			{
				SetError("cos accepts 1 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.cos(args[0]);
			break;
		case FUNC_SIN:
			if (argsFound != 1)
			{
				SetError("sin accepts 1 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.sin(args[0]);
			break;
		case FUNC_TAN:
			if (argsFound != 1)
			{
				SetError("tan accepts 1 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.tan(args[0]);
			break;
		case FUNC_ACOS:
			if (argsFound != 1)
			{
				SetError("acos accepts 1 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.acos(args[0]);
			break;
		case FUNC_ASIN:
			if (argsFound != 1)
			{
				SetError("asin accepts 1 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.asin(args[0]);
			break;
		case FUNC_ATAN:
			if (argsFound != 1)
			{
				SetError("atan accepts 1 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.atan(args[0]);
			break;
		case FUNC_TIERS:
			if (argsFound != 2)
			{
				SetError("tiers accepts 2 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.tiers(args[0], args[1]);
			break;
		case FUNC_SMOOTH_TIERS:
			if (argsFound != 2)
			{
				SetError("smoothTiers accepts 2 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.smoothTiers(args[0], args[1]);
			break;
		case FUNC_BLEND:
			if (argsFound != 3)
			{
				SetError("blend accepts 3 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.blend(args[0], args[1], args[2]);
			break;
		case FUNC_SELECT:
			if (argsFound != 5)
			{
				SetError("select accepts 5 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.select(args[0], args[1], args[2], args[3], args[4]);
			break;
		case FUNC_SIMPLE_RIDGED_MULTIFRACTAL:
			if (argsFound != 6)
			{
				SetError("simpleRidgedMultifractal accepts 6 arguemnts", funcToken);
				return false;
			}
			SetError("simpleRidgedMultifractal not supported");
			//instruction = Kernel.simpleRidgedMultifractal(args[0], args[1], args[2], args[3], args[4], args[5]);
			break;
		case FUNC_SIMPLE_FBM:
			if (argsFound != 6)
			{
				SetError("simplefBm accepts 6 arguemnts", funcToken);
				return false;
			}
			SetError("simplefBm not supported");
			//instruction = Kernel.simplefBm(args[0], args[1], args[2], args[3], args[4], args[5]);
			break;
		case FUNC_SIMPLE_BILLOW:
			if (argsFound != 6)
			{
				SetError("simpleBillow accepts 6 arguemnts", funcToken);
				return false;
			}
			SetError("simpleBillow not supported");
			//instruction = Kernel.simpleBillow(args[0], args[1], args[2], args[3], args[4], args[5]);
			break;
		case FUNC_X:
			if (argsFound != 0)
			{
				SetError("x accepts 0 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.x();
			break;
		case FUNC_Y:
			if (argsFound != 0)
			{
				SetError("y accepts 0 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.y();
			break;
		case FUNC_Z:
			if (argsFound != 0)
			{
				SetError("z accepts 0 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.z();
			break;
		case FUNC_W:
			if (argsFound != 0)
			{
				SetError("w accepts 0 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.w();
			break;
		case FUNC_U:
			if (argsFound != 0)
			{
				SetError("u accepts 0 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.u();
			break;
		case FUNC_V:
			if (argsFound != 0)
			{
				SetError("v accepts 0 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.v();
			break;
		case FUNC_DX:
			if (argsFound != 2)
			{
				SetError("dx accepts 2 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.dx(args[0], args[1]);
			break;
		case FUNC_DY:
			if (argsFound != 2)
			{
				SetError("dy accepts 2 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.dy(args[0], args[1]);
			break;
		case FUNC_DZ:
			if (argsFound != 2)
			{
				SetError("dz accepts 2 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.dz(args[0], args[1]);
			break;
		case FUNC_DW:
			if (argsFound != 2)
			{
				SetError("dw accepts 2 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.dw(args[0], args[1]);
			break;
		case FUNC_DU:
			if (argsFound != 2)
			{
				SetError("du accepts 2 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.du(args[0], args[1]);
			break;
		case FUNC_DV:
			if (argsFound != 2)
			{
				SetError("dv accepts 2 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.dv(args[0], args[1]);
			break;
		case FUNC_SIGMOID:
			if (argsFound != 1 && argsFound != 3)
			{
				SetError("dv accepts 1 or 3 arguemnts", funcToken);
				return false;
			}
			if(argsFound == 1)
				instruction = Kernel.sigmoid(args[0]);
			else
				instruction = Kernel.sigmoid(args[0], args[1], args[2]);
			break;
		case FUNC_SCALE_OFFSET:
			if (argsFound != 3)
			{
				SetError("scaleOffset accepts 3 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.scaleOffset(args[0], args[1], args[2]);
			break;
		case FUNC_RADIAL:
			if (argsFound != 0)
			{
				SetError("radial accepts 0 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.radial();
			break;
		case FUNC_CLAMP:
			if (argsFound != 3)
			{
				SetError("clamp accepts 3 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.clamp(args[0], args[1], args[2]);
			break;
		case FUNC_RGBA:
		case FUNC_COLOR:
			if (argsFound != 4)
			{
				SetError("rgba/color accepts 4 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.combineRGBA(args[0], args[1], args[2], args[3]);
			break;
		case FUNC_HEX_TILE:
			if (argsFound != 1)
			{
				SetError("hexTile accepts 1 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.hexTile(args[0]);
			break;
		case FUNC_HEX_BUMP:
			if (argsFound != 0)
			{
				SetError("hexBump accepts 0 arguemnts", funcToken);
				return false;
			}
			instruction = Kernel.hexBump();
			break;
		default:
			SetError("Unkown function type", funcToken);
			return false;
		}
		return true;
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
		if (t.token != Token::MULT && t.token != Token::DIV)
		{
			tokens.UnGet();
			// not an error, we are just returning the object
			return true;
		}

		CInstructionIndex operandRight(instruction);
		if (mult(operandRight))
		{
			if(t.token == Token::MULT)
				instruction = Kernel.multiply(instruction, operandRight);
			else
				instruction = Kernel.divide(instruction, operandRight);
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
		bool success = program(ParseResult);

		Token t;
		if (IsEof(t) == false)
		{
			SetError("expression ended prematurly", t);
		}

		success = success && Error == false;

		if (success)
		{
			Kernel.optimize(TotalFolds, TotalInstructions);
		}

		return success;
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