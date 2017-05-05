#include "NoiseParser.h"
#include "NoiseParserAST.h"

#include <math.h>
#include <sstream>

namespace anl
{
	namespace lang {
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

			if (RewindLineNumber >= 0)
				LineNumber = RewindLineNumber;
			RewindLineNumber = RewindLineNumber2;
			RewindLineNumber2 = -1;

			if (RewindColumnStartOffset >= 0)
				ColumnStartOffset = RewindColumnStartOffset;
			RewindColumnStartOffset = RewindColumnStartOffset2;
			RewindColumnStartOffset2 = -1;
		}

		void NoiseParser::Tokenizer::SetUnGetLocation()
		{
			RewindIndex2 = RewindIndex;
			RewindIndex = DataIndex;

			RewindLineNumber2 = RewindLineNumber;
			RewindLineNumber = LineNumber;

			RewindColumnStartOffset2 = RewindColumnStartOffset;
			RewindColumnStartOffset = ColumnStartOffset;
		}

		Token NoiseParser::Tokenizer::GetToken()
		{
			LastError = "";

			Token token;
			token.token = Token::NONE;
			token.tokenLocation = ColumnStartOffset - DataIndex;
			token.number = 0.0;

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
				token.tokenLocation = DataIndex - ColumnStartOffset;
				token.lineNumber = LineNumber;
				DataIndex++;
				switch (c)
				{
				case ' ':
				case '\t':
				case '\r':
					// ignore whitespace
					break;
				case '\n':
					// ignore whitespace, but count lines
					LineNumber++;
					ColumnStartOffset = DataIndex;
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
				case ';':
					token.token = Token::SEMI_COLON;
					return token;
				case '=':
					token.token = Token::ASSIGNMENT;
					return token;
				case '<':
				{
					token.token = Token::TOKEN_ERROR;
					if (IsEof())
					{
						SetError("Unexpected eof found within a domain operator '<>'");
						return token;
					}

					c = Data[DataIndex];
					DataIndex++;
					Token::TokenType tt;
					if (c == 's')
						tt = Token::DOMAIN_SCALE;
					else if (c == 't')
						tt = Token::DOMAIN_TRANSLATE;
					else if (c == 'r')
						tt = Token::DOMAIN_ROTATE;
					else
					{
						ParseString msg = "Unexpected character found within a domain operator '<>' char:'";
						msg += c;
						msg += "'";
						SetError(msg);
						return token;
					}

					if (IsEof())
					{
						SetError("Unexpected eof found within a domain operator '<>'");
						return token;
					}

					c = Data[DataIndex];
					DataIndex++;
					if (c == 'x' || c == 'y' || c == 'z' || c == 'w' || c == 'u' || c == 'v')
					{
						// this domain operation only applys in one of the 6 domains.
						if (c == 'x')
							tt = (Token::TokenType)(tt + 1);
						else if (c == 'y')
							tt = (Token::TokenType)(tt + 2);
						else if (c == 'z')
							tt = (Token::TokenType)(tt + 3);
						else if (c == 'w')
							tt = (Token::TokenType)(tt + 4);
						else if (c == 'u')
							tt = (Token::TokenType)(tt + 5);
						else if (c == 'v')
							tt = (Token::TokenType)(tt + 6);
						// advance
						c = Data[DataIndex];
						DataIndex++;
					}

					if (IsEof())
					{
						SetError("Unexpected eof found within a domain operator '<>'");
						return token;
					}

					if (c != ':')
					{
						ParseString msg = "Unexpected character found within a domain operator '<>' char:'";
						msg += c;
						msg += "'";
						SetError(msg);
						return token;
					}
					token.token = tt;
					return token;
				}
				case '>':
					token.token = Token::R_CHEVRON;
					return token;
				case '/':
					// peek ahead to see if this is a block comment
					if (IsEof() == false && Data[DataIndex] == '*')
					{
						DataIndex++;
						// block comment, gobble untill end, but still count lines
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
							else if (Data[DataIndex] == '\n')
							{
								DataIndex++;
								LineNumber++;
								ColumnStartOffset = DataIndex;
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
						// gobble the newline as well, and count the line number
						if (IsEof() == false)
						{
							DataIndex++;
							LineNumber++;
							ColumnStartOffset = DataIndex;
						}
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
						if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || (token.keyword.length() != 0 && c >= '0' && c <= '9'))
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
								token.tokenLocation = DataIndex - ColumnStartOffset;

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
							msg += std::to_string(DataIndex);
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

		}

		void NoiseParser::SetError(ParseString msg, const Token& cause)
		{
			std::stringstream ss;
			if (tokens.IsError())
			{
				ss << "Tokenizer error: " << tokens.GetLastError() << ". ";
			}
			ss << "Parse Error ln:" << cause.lineNumber << " col:" << cause.tokenLocation << " - " << msg;
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

		EBlend::BlendType NoiseParser::KeywordToBlend(const std::string& keyword)
		{
			EBlend::BlendType t = EBlend::BLEND_INVALID;
			if (keyword == "BLEND_NONE")
				t = EBlend::BLEND_NONE;
			else if (keyword == "BLEND_LINEAR")
				t = EBlend::BLEND_LINEAR;
			else if (keyword == "BLEND_HERMITE")
				t = EBlend::BLEND_HERMITE;
			else if (keyword == "BLEND_QUINTIC")
				t = EBlend::BLEND_QUINTIC;
			return t;
		}

		EFunction::Function NoiseParser::KeywordToFunc(const std::string& keyword)
		{
			EFunction::Function f = EFunction::FUNC_INVALID;
			if (keyword == "valueBasis")
				f = EFunction::FUNC_VALUE_BASIS;
			else if (keyword == "gradientBasis")
				f = EFunction::FUNC_GRADIENT_BASIS;
			else if (keyword == "simplexBasis")
				f = EFunction::FUNC_SIMPLEX_BASIS;
			else if (keyword == "cellularBasis")
				f = EFunction::FUNC_CELLULAR_BASIS;
			else if (keyword == "max")
				f = EFunction::FUNC_MAX;
			else if (keyword == "min")
				f = EFunction::FUNC_MIN;
			else if (keyword == "abs")
				f = EFunction::FUNC_ABS;
			else if (keyword == "pow")
				f = EFunction::FUNC_POW;
			else if (keyword == "bias")
				f = EFunction::FUNC_BIAS;
			else if (keyword == "gain")
				f = EFunction::FUNC_GAIN;
			else if (keyword == "cos")
				f = EFunction::FUNC_COS;
			else if (keyword == "sin")
				f = EFunction::FUNC_SIN;
			else if (keyword == "tan")
				f = EFunction::FUNC_TAN;
			else if (keyword == "acos")
				f = EFunction::FUNC_ACOS;
			else if (keyword == "asin")
				f = EFunction::FUNC_ASIN;
			else if (keyword == "atan")
				f = EFunction::FUNC_ATAN;
			else if (keyword == "tiers")
				f = EFunction::FUNC_TIERS;
			else if (keyword == "smoothTiers")
				f = EFunction::FUNC_SMOOTH_TIERS;
			else if (keyword == "blend")
				f = EFunction::FUNC_BLEND;
			else if (keyword == "select")
				f = EFunction::FUNC_SELECT;
			else if (keyword == "simpleRidgedMultifractal")
				f = EFunction::FUNC_SIMPLE_RIDGED_MULTIFRACTAL;
			else if (keyword == "simplefBm")
				f = EFunction::FUNC_SIMPLE_FBM;
			else if (keyword == "simpleBillow")
				f = EFunction::FUNC_SIMPLE_BILLOW;
			else if (keyword == "x")
				f = EFunction::FUNC_X;
			else if (keyword == "y")
				f = EFunction::FUNC_Y;
			else if (keyword == "z")
				f = EFunction::FUNC_Z;
			else if (keyword == "w")
				f = EFunction::FUNC_W;
			else if (keyword == "u")
				f = EFunction::FUNC_U;
			else if (keyword == "v")
				f = EFunction::FUNC_V;
			else if (keyword == "dx")
				f = EFunction::FUNC_DX;
			else if (keyword == "dy")
				f = EFunction::FUNC_DY;
			else if (keyword == "dz")
				f = EFunction::FUNC_DZ;
			else if (keyword == "dw")
				f = EFunction::FUNC_DW;
			else if (keyword == "du")
				f = EFunction::FUNC_DU;
			else if (keyword == "dv")
				f = EFunction::FUNC_DV;
			else if (keyword == "sigmoid")
				f = EFunction::FUNC_SIGMOID;
			else if (keyword == "scaleOffset")
				f = EFunction::FUNC_SCALE_OFFSET;
			else if (keyword == "radial")
				f = EFunction::FUNC_RADIAL;
			else if (keyword == "clamp")
				f = EFunction::FUNC_CLAMP;
			else if (keyword == "rgba")
				f = EFunction::FUNC_RGBA;
			else if (keyword == "color")
				f = EFunction::FUNC_COLOR;
			else if (keyword == "hexTile")
				f = EFunction::FUNC_HEX_TILE;
			else if (keyword == "hexBump")
				f = EFunction::FUNC_HEX_BUMP;
			return f;
		}

		bool NoiseParser::IsKeyword_OP_ValueBasis(const std::string& keyword)
		{
			if (keyword == "OP_ValueBasis")
				return true;
			return false;
		}

		bool NoiseParser::IsKeyword_OP_GradientBasis(const std::string& keyword)
		{
			if (keyword == "OP_GradientBasis")
				return true;
			return false;
		}

		bool NoiseParser::IsKeyword_OP_SimplexBasis(const std::string& keyword)
		{
			if (keyword == "OP_SimplexBasis")
				return true;
			return false;
		}

		bool NoiseParser::IsKeyword_True(const std::string& keyword)
		{
			if (keyword == "true" || keyword == "TRUE" || keyword == "True")
				return true;
			return false;
		}

		bool NoiseParser::IsKeyword_False(const std::string& keyword)
		{
			if (keyword == "false" || keyword == "FALSE" || keyword == "False")
				return true;
			return false;
		}

		bool NoiseParser::IsKeyword_e(const std::string& keyword)
		{
			if (keyword == "e")
				return true;
			return false;
		}

		bool NoiseParser::IsKeyword_pi(const std::string& keyword)
		{
			if (keyword == "pi")
				return true;
			return false;
		}

		bool NoiseParser::domainOperator()
		{
			Token opToken = tokens.GetToken();
			if (opToken.token < Token::DOMAIN_OP_BEGIN || opToken.token >= Token::DOMAIN_OP_END)
			{
				if (tokens.IsError())
					SetError("Malformed token", opToken);
				tokens.UnGet();
				return false;
			}

			if (argumentList() == false)
			{
				SetError("domainOperator '< expression >' requires an expression", opToken);
				return false;
			}

			Token t = tokens.GetToken();
			if (t.token != Token::R_CHEVRON)
			{
				SetError("Unable to find closing chevron '>'", t);
				return false;
			}

			NodePtr args = Stack.back();
			Stack.pop_back();
			Stack.push_back(std::make_shared<NoiseParserAST::domainOperator>(opToken, args));

			return true;
		}

		bool NoiseParser::argumentList()
		{
			NodePtr argList = nullptr;
			NodePtr arg = nullptr;
			if (expression())
			{
				arg = Stack.back();
				Stack.pop_back();

				Token t = tokens.GetToken();
				if (t.token == Token::COMMA)
				{
					argumentList();
					argList = Stack.back();
					Stack.pop_back();
				}
				else
				{
					tokens.UnGet();
				}
				Stack.push_back(std::make_shared<NoiseParserAST::argumentList>(arg, argList));
				return true;// return true, found atleast one.
			}
			else
			{
				// return an object even if its empty
				Stack.push_back(std::make_shared<NoiseParserAST::argumentList>(arg, argList));
				return false;
			}
		}

		bool NoiseParser::functionCall()
		{
			Token t = tokens.GetToken();
			if (t.token != Token::KEYWORD)
			{
				tokens.UnGet();
				return false;
			}

			EFunction::Function func = KeywordToFunc(t.keyword);
			Token funcToken = t;
			if (func == EFunction::FUNC_INVALID)
			{
				// not a known function, it had better be a constant or a variable
				//SetError("Unexpected 'function' name", t);
				tokens.UnGet();
				return false;
			}

			NodePtr funcNode = std::make_shared<NoiseParserAST::keyword>(funcToken);

			t = tokens.GetToken();
			if (t.token != Token::L_PAREN)
			{
				//SetError("Missing '(' after keyword for 'function call'", t);
				tokens.UnGet();// put back the '('
				tokens.UnGet();// put back the keyword
				return false;
			}

			argumentList();

			t = tokens.GetToken();
			if (t.token != Token::R_PAREN)
			{
				SetError("Missing matching ')' in 'function call'", t);
				return false;
			}

			NodePtr args = Stack.back();
			Stack.pop_back();
			Stack.push_back(std::make_shared<NoiseParserAST::functionCall>(funcNode, args));

			return true;
		}

		bool NoiseParser::object()
		{
			bool foundSimple = false;
			if (functionCall())
				foundSimple = true;
			else if (grouping())
				foundSimple = true;
			else if (negative())
				foundSimple = true;

			if (foundSimple)
			{
				NodePtr obj = Stack.back();
				Stack.pop_back();
				Stack.push_back(std::make_shared<NoiseParserAST::object>(obj));
				return true;
			}

			Token t = tokens.GetToken();
			if (t.token == Token::NUMBER)
			{
				NodePtr num = std::make_shared<NoiseParserAST::number>(t);
				Stack.push_back(std::make_shared<NoiseParserAST::object>(num));
				return true;
			}
			else if (t.token == Token::KEYWORD)
			{
				NodePtr key = std::make_shared<NoiseParserAST::keyword>(t);
				Stack.push_back(std::make_shared<NoiseParserAST::object>(key));
				return true;
			}
			else
			{
				tokens.UnGet();
				return false;
			}
		}

		bool NoiseParser::domainPrecedence()
		{
			NodePtr domainOp = nullptr;
			NodePtr domainPrec = nullptr;
			NodePtr obj = nullptr;

			// optional
			bool hasDomainOperator = domainOperator();
			if (hasDomainOperator)
			{
				domainOp = Stack.back();
				Stack.pop_back();
			}


			bool foundExpr = false;
			if (hasDomainOperator && domainPrecedence())
			{
				foundExpr = true;
				domainPrec = Stack.back();
				Stack.pop_back();
			}
			else if (object())
			{
				foundExpr = true;
				obj = Stack.back();
				Stack.pop_back();
			}

			if (hasDomainOperator && foundExpr == false)
			{
				SetError("Domain operator '<xx: argumentList  > object' requires an object ");
				return false;
			}
			else if (foundExpr == false)
			{
				return false;
			}

			Stack.push_back(std::make_shared<NoiseParserAST::domainPrecedence>(domainOp, domainPrec, obj));
			return true;
		}

		bool NoiseParser::mult()
		{
			if (domainPrecedence() == false)
				return false;

			Token t = tokens.GetToken();
			if (t.token != Token::MULT && t.token != Token::DIV)
			{
				tokens.UnGet();
				// not an error, we are just returning the object
				return true;
			}

			if (mult())
			{
				NodePtr right = Stack.back();
				Stack.pop_back();
				NodePtr left = Stack.back();
				Stack.pop_back();

				Stack.push_back(std::make_shared<NoiseParserAST::mult>(t, left, right));
				return true;
			}
			else
			{
				SetError("Missing value to the right of the multiply operator", t);
				return false;
			}
		}

		bool NoiseParser::add()
		{
			if (mult() == false)
				return false;

			Token t = tokens.GetToken();
			if (t.token != Token::ADD && t.token != Token::SUB)
			{
				tokens.UnGet();
				// not an error, we are just returning the object
				return true;
			}

			if (add())
			{
				NodePtr right = Stack.back();
				Stack.pop_back();
				NodePtr left = Stack.back();
				Stack.pop_back();

				Stack.push_back(std::make_shared<NoiseParserAST::add>(t, left, right));

				return true;
			}
			else
			{
				SetError("Missing value to the right of the +/- operator", t);
				return false;
			}
		}

		bool NoiseParser::grouping()
		{
			Token t = tokens.GetToken();
			if (t.token != Token::L_PAREN)
			{
				tokens.UnGet();
				return false;
			}

			if (expression() == false)
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

			// for these simple pass through portions of the grammer, just wrap the
			// node we are passing the the appropriately named node type.
			NodePtr node = Stack.back();
			Stack.pop_back();
			Stack.push_back(std::make_shared<NoiseParserAST::grouping>(node));

			return true;
		}

		bool NoiseParser::negative()
		{
			Token t = tokens.GetToken();
			if (t.token != Token::SUB)
			{
				tokens.UnGet();
				return false;
			}

			if (object() == false && domainPrecedence() == false)
			{
				SetError("Attempting to negate nothing", t);
				return false;
			}

			// for these simple pass through portions of the grammer, just wrap the
			// node we are passing the the appropriately named node type.
			NodePtr node = Stack.back();
			Stack.pop_back();
			Stack.push_back(std::make_shared<NoiseParserAST::negative>(t, node));
			return true;
		}

		bool NoiseParser::expression()
		{
			bool result;
			if (add())
				result = true;
			else if (negative())
				result = true;
			else if (grouping())
				result = true;
			else
				result = false;

			if (result)
			{
				// for these simple pass through portions of the grammer, just wrap the
				// node we are passing the the appropriately named node type.
				NodePtr node = Stack.back();
				Stack.pop_back();
				Stack.push_back(std::make_shared<NoiseParserAST::expression>(node));
			}

			return result;
		}

		bool NoiseParser::statement()
		{
			if (expression() == false)
				return false;

			// for these simple pass through portions of the grammer, just wrap the
			// node we are passing the the appropriately named node type.
			NodePtr node = Stack.back();
			Stack.pop_back();
			Stack.push_back(std::make_shared<NoiseParserAST::statement>(node));

			Token t = tokens.GetToken();
			if (t.token != Token::SEMI_COLON)
			{
				SetError("Statement does not end in a semicolon", t);
				return false;
			}
			return true;
		}

		bool NoiseParser::assignment()
		{
			ParseString variableName;
			CInstructionIndex variableValue(NOP);
			bool isAssignment = false;

			Token keywordToken = tokens.GetToken();
			NodePtr keyword = nullptr;
			if (keywordToken.token == Token::KEYWORD)
			{
				variableName = keywordToken.keyword;
				Token t = tokens.GetToken();
				if (t.token == Token::ASSIGNMENT)
				{
					keyword = std::make_shared<NoiseParserAST::keyword>(keywordToken);
					isAssignment = true;
				}
				else
				{
					// the keyword we thought we were supposed to assign to, turns out to be a
					// variable reference, put it back and try for a statment
					tokens.UnGet();
					tokens.UnGet();
				}
			}
			else
			{
				tokens.UnGet();
			}

			if (statement() == false)
			{
				if (isAssignment)
					SetError("Missing statement following assignment operator", keywordToken);
				return false;
			}
			else
			{
				NodePtr statementPtr = Stack.back();
				Stack.pop_back();
				Stack.push_back(std::make_shared<NoiseParserAST::assignment>(keywordToken, keyword, statementPtr));
				return true;
			}
		}

		bool NoiseParser::program()
		{
			if (assignment() == false)
				return false;

			NodePtr assignmentPtr = Stack.back();
			Stack.pop_back();

			NodePtr programPtr = nullptr;
			if (program())
			{
				programPtr = Stack.back();
				Stack.pop_back();
			}

			Stack.push_back(std::make_shared<NoiseParserAST::program>(assignmentPtr, programPtr));
			return true;
		}

		bool NoiseParser::Parse()
		{
			// return true if there was an expression found and no error
			bool success = program();
			//bool success = true;
			//ParseResult = Kernel.simpleBillow(OP_SimplexBasis,EBlend::BLEND_QUINTIC, 5, 1.0, 654989732, true);


			ParseResult = NOP;

			Token t;
			if (IsEof(t) == false)
			{
				SetError("expression ended prematurly", t);
			}

			success = success && Error == false;

			if (success)
			{
				NodePtr ASTRoot = Stack.back();

				// ShapeUp is required before emitting
				ASTRoot->ShapeUpAll();
				// RemoveIntermidiates is technically optional at the moment 
				ASTRoot->RemoveIntermidiates();

				NoiseParserAST::ANLEmitter emitter(Kernel);
				ASTRoot->Emit(&emitter);
				if (emitter.IsError())
				{
					std::string msgs = emitter.FormErrorMsgs();
					SetError("ANLEmitter Error: " + msgs);
					success = false;
				}
				else
				{
					ParseResult = emitter.GetProgramStart();
					Kernel.optimize(TotalFolds, TotalInstructions);
				}
			}

			Stack.clear();

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
}