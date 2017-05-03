#include "NoiseParserAST.h"
#include "NoiseParserEmitter.h"

#include <math.h>
#include <sstream>

#include "NoiseParser.h"

namespace anl
{
	namespace lang {
		namespace NoiseParserAST
		{
			void Emitter::SetError(std::string msg, const Token& t)
			{
				std::stringstream ss;
				ss << "AST traversal Error ln:" << t.lineNumber << " col:" << t.tokenLocation << " - " << msg;
				ErrorList.push_back(ss.str());
			}

			std::string Emitter::FormErrorMsgs()
			{
				std::string msgs;
				for (auto s : ErrorList)
				{
					msgs += s;
					msgs += '\n';
				}
				return msgs;
			}

			void ANLEmitter::Emit(number * node)
			{
				Stack.push_back(kernel.constant(node->token.number));
			}
			void ANLEmitter::Emit(keyword * node)
			{
				// if we were asked to Emit then we must be a variable
				// check constants first
				std::string& key = node->token.keyword;
				CInstructionIndex newInstruction(NOP);
				bool NewInstructionSet = false;
				EBlend::BlendType bt = NoiseParser::KeywordToBlend(key);
				if (NoiseParser::IsKeyword_pi(key)) {
					newInstruction = kernel.pi(); NewInstructionSet = true;
				}
				else if (NoiseParser::IsKeyword_e(key)) {
					newInstruction = kernel.e(); NewInstructionSet = true;
				}
				else if (NoiseParser::IsKeyword_OP_ValueBasis(key)) {
					newInstruction = kernel.constant(OP_ValueBasis); NewInstructionSet = true;
				}
				else if (NoiseParser::IsKeyword_OP_GradientBasis(key)) {
					newInstruction = kernel.constant(OP_GradientBasis); NewInstructionSet = true;
				}
				else if (NoiseParser::IsKeyword_OP_SimplexBasis(key)) {
					newInstruction = kernel.constant(OP_SimplexBasis); NewInstructionSet = true;
				}
				else if (NoiseParser::IsKeyword_True(key)) {
					newInstruction = kernel.one(); NewInstructionSet = true;
				}
				else if (NoiseParser::IsKeyword_False(key)) {
					newInstruction = kernel.zero(); NewInstructionSet = true;
				}
				else if (bt != EBlend::BLEND_INVALID) {
					newInstruction = kernel.constant(bt); NewInstructionSet = true;
				}

				if (NewInstructionSet)
				{
					Stack.push_back(newInstruction);
					return;// found the constant
				}


				auto var = Variables.find(key);
				if (var != Variables.end())
				{
					Stack.push_back(*var->second);
				}
				else
				{
					// see if the variable is not yet loaded
					auto varEntry = VariableCandidates.find(key);
					if (varEntry != VariableCandidates.end())
					{
						varEntry->second->Emit(this);
						// the variable is the result of the Emit, thus its now on the Stack.
						// the variable is also now in Variables.
					}
					else
					{
						std::string msg = "Undeclared variable: ";
						msg += key;
						SetError(msg, node->token);
						// push a zero in its place to allow use to continue emitting and potentially detect more errors,
						// and not crash.
						Stack.push_back(kernel.zero());
					}
				}
			}
			void ANLEmitter::Emit(domainOperator * node)
			{
				// the object to do the domain operation on is already waiting for us
				// on the stack thanks to ANLEmitter::Emit(domainPrecedence * node)
				CInstructionIndex objectToAffect = Stack.back();
				Stack.pop_back();

				// put the arguments on the stack, and the count on the integer stack.
				node->Child[0]->Emit(this);
				int argCount = StackIntegers.back();
				StackIntegers.pop_back();

				// emits the proper instructions and cleans up the arguments on the stack.
				SetupDomainOperator(objectToAffect, argCount, node->token);
			}
			void ANLEmitter::Emit(argumentList * node)
			{
				StackIntegers.push_back((int)node->Child.size());
				// iterate in order thus pushing things onto the Stack left to right
				for (int i = 0; i < node->Child.size(); ++i)
					node->Child[i]->Emit(this);
			}
			void ANLEmitter::Emit(functionCall * node)
			{
				// we dont actually emit with keywords, we just read their value.
				if (node->Child[0]->IsType(Node::KEYWORD) == false)
					SetError("Improper functionCall Node setup, Child[0] must be a keyword.", node->token);

				node->Child[1]->Emit(this);

				int argc = StackIntegers.back();
				StackIntegers.pop_back();

				EFunction::Function func = NoiseParser::KeywordToFunc(node->Child[0]->token.keyword);
				SetupFunctionCall(argc, func, node->Child[0]->token);
				// setup function call removes the arguments and places the result on the stack.
			}
			void ANLEmitter::Emit(object * node)
			{
				node->Child[0]->Emit(this);
			}
			void ANLEmitter::Emit(domainPrecedence * node)
			{
				if (node->Child.size() == 2)
				{
					// emit the object to do the domain operation on first, so it will
					// be on the stack waiting for us when we call emit on the domainOperator
					node->Child[1]->Emit(this);
					node->Child[0]->Emit(this);
				}
				else
				{
					node->Child[0]->Emit(this);
				}
			}
			void ANLEmitter::Emit(mult * node)
			{
				node->Child[0]->Emit(this);
				node->Child[1]->Emit(this);
				CInstructionIndex right = Stack.back();
				Stack.pop_back();
				CInstructionIndex left = Stack.back();
				Stack.pop_back();
				if (node->token.token == Token::MULT)
					Stack.push_back(kernel.multiply(left, right));
				else
					Stack.push_back(kernel.divide(left, right));
			}
			void ANLEmitter::Emit(add * node)
			{
				node->Child[0]->Emit(this);
				node->Child[1]->Emit(this);
				CInstructionIndex right = Stack.back();
				Stack.pop_back();
				CInstructionIndex left = Stack.back();
				Stack.pop_back();
				if (node->token.token == Token::ADD)
					Stack.push_back(kernel.add(left, right));
				else
					Stack.push_back(kernel.subtract(left, right));
			}
			void ANLEmitter::Emit(grouping * node)
			{
				node->Child[0]->Emit(this);
			}
			void ANLEmitter::Emit(negative * node)
			{
				node->Child[0]->Emit(this);
				CInstructionIndex object = Stack.back();
				Stack.pop_back();
				Stack.push_back(kernel.multiply(NegativeOne, object));
			}
			void ANLEmitter::Emit(expression * node)
			{
				node->Child[0]->Emit(this);
			}
			void ANLEmitter::Emit(statement * node)
			{
				node->Child[0]->Emit(this);
			}
			void ANLEmitter::Emit(assignment * node)
			{
				// emit instructions for the statement
				node->Child[1]->Emit(this);
				// check for existing keyword
				if (node->Child[0])
				{
					std::string& var = node->Child[0]->token.keyword;
					auto existing = Variables.find(var);
					if (existing != Variables.end())
						SetError("variable already declared: " + var, node->Child[0]->token);
					else
						Variables[var] = std::make_unique<CInstructionIndex>(Stack.back());
					// don't pop, saving the expression off to a variable is a side effect,
					// we still have to return the expression to whomever started the Emit()
				}
			}
			void ANLEmitter::Emit(program * node)
			{
				// as we recurse down, try to detect any assignment operations inorder to 
				// make them available later
				if (node->Child[0])
				{
					if (node->Child[0]->IsType(Node::ASSIGNMENT) == false)
					{
						//SetError("program node Child[0] must be of type ASSIGNMENT", node->Child[0]->token);
						// The assignment node may be removed if there is no actual assignment done and its
						// just a pass through.
					}
					else
					{
						assignment* assignmentNode = static_cast<assignment*>(node->Child[0].get());
						if (assignmentNode->Child[0])
						{
							// the first index in the assignment node is the keyword child.
							std::string& key = assignmentNode->Child[0]->token.keyword;
							if (VariableCandidates.find(key) != VariableCandidates.end())
								SetError("Multiply defined variable: " + key, assignmentNode->token);
							else
								VariableCandidates[key] = assignmentNode;
						}
					}
				}

				if (node->Child[1])
				{
					node->Child[1]->Emit(this);
				}
				else if (node->Child[0])
				{
					node->Child[0]->Emit(this);
					if (IsError() == false)
					{
						// the last statment is the one to execute directly, capture it now
						ProgramStart = Stack.back();
					}
				}
			}

			void ANLEmitter::SetupDomainOperator(CInstructionIndex instruction, int argsFound, const Token& t)
			{
				auto args = Stack.end() - argsFound;
				Token::TokenType tt = t.token;
				if (argsFound == 1)
				{
					switch (tt)
					{
					case Token::DOMAIN_SCALE:
						instruction = kernel.scaleDomain(instruction, args[0]);
						break;
					case Token::DOMAIN_SCALE_X:
						instruction = kernel.scaleX(instruction, args[0]);
						break;
					case Token::DOMAIN_SCALE_Y:
						instruction = kernel.scaleY(instruction, args[0]);
						break;
					case Token::DOMAIN_SCALE_Z:
						instruction = kernel.scaleZ(instruction, args[0]);
						break;
					case Token::DOMAIN_SCALE_W:
						instruction = kernel.scaleW(instruction, args[0]);
						break;
					case Token::DOMAIN_SCALE_U:
						instruction = kernel.scaleU(instruction, args[0]);
						break;
					case Token::DOMAIN_SCALE_V:
						instruction = kernel.scaleV(instruction, args[0]);
						break;
					case Token::DOMAIN_TRANSLATE:
						instruction = kernel.translateDomain(instruction, args[0]);
						break;
					case Token::DOMAIN_TRANSLATE_X:
						instruction = kernel.translateX(instruction, args[0]);
						break;
					case Token::DOMAIN_TRANSLATE_Y:
						instruction = kernel.translateY(instruction, args[0]);
						break;
					case Token::DOMAIN_TRANSLATE_Z:
						instruction = kernel.translateZ(instruction, args[0]);
						break;
					case Token::DOMAIN_TRANSLATE_W:
						instruction = kernel.translateW(instruction, args[0]);
						break;
					case Token::DOMAIN_TRANSLATE_U:
						instruction = kernel.translateU(instruction, args[0]);
						break;
					case Token::DOMAIN_TRANSLATE_V:
						instruction = kernel.translateV(instruction, args[0]);
						break;
					default:
						SetError("Unrecognized token in domainPrecedence", t);
						break;
					}
				}
				else if (argsFound == 4)
				{
					switch (tt)
					{
					case Token::DOMAIN_ROTATE:
						instruction = kernel.rotateDomain(instruction, args[0], args[1], args[2], args[3]);
						break;
					default:
						SetError("Unrecognized token in domainPrecedence (multi arg section)", t);
						break;
					}
				}
				else
				{
					SetError("This domain operation requires either 1 or 4 arguments.", t);
				}
				Stack.erase(Stack.end() - argsFound, Stack.end());
				Stack.push_back(instruction);
			}

			void ANLEmitter::SetupFunctionCall(int argsFound, EFunction::Function func, Token& funcToken)
			{
				auto args = Stack.end() - argsFound;
				CInstructionIndex instruction = NOP;

				int nonConstArgIndex = -1;

				// we now have the name of the function and all the arguments
				switch (func)
				{
				case EFunction::FUNC_VALUE_BASIS:
					if (argsFound != 2)
					{
						SetError("valueBasis accepts 2 arguemnts", funcToken);
						return;
					}
					instruction = kernel.valueBasis(args[0], args[1]);
					break;
				case EFunction::FUNC_GRADIENT_BASIS:
					if (argsFound != 2)
					{
						SetError("GradientBasis accepts 2 arguemnts", funcToken);
						return;
					}
					instruction = kernel.gradientBasis(args[0], args[1]);
					break;
				case EFunction::FUNC_SIMPLEX_BASIS:
					if (argsFound != 1)
					{
						SetError("SimplexBasis accepts 1 arguemnt", funcToken);
						return;
					}
					instruction = kernel.simplexBasis(args[0]);
					break;
				case EFunction::FUNC_CELLULAR_BASIS:
					if (argsFound != 10)
					{
						SetError("cellularBasis accepts 10 arguemnts", funcToken);
						return;
					}
					instruction = kernel.cellularBasis(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]);
					break;
				case EFunction::FUNC_MAX:
					if (argsFound != 2)
					{
						SetError("max accepts 2 arguemnts", funcToken);
						return;
					}
					instruction = kernel.maximum(args[0], args[1]);
					break;
				case EFunction::FUNC_MIN:
					if (argsFound != 2)
					{
						SetError("min accepts 2 arguemnts", funcToken);
						return;
					}
					instruction = kernel.minimum(args[0], args[1]);
					break;
				case EFunction::FUNC_ABS:
					if (argsFound != 1)
					{
						SetError("abs accepts 1 arguemnts", funcToken);
						return;
					}
					instruction = kernel.abs(args[0]);
					break;
				case EFunction::FUNC_POW:
					if (argsFound != 2)
					{
						SetError("pow accepts 2 arguemnts", funcToken);
						return;
					}
					instruction = kernel.pow(args[0], args[1]);
					break;
				case EFunction::FUNC_BIAS:
					if (argsFound != 2)
					{
						SetError("bias accepts 2 arguemnts", funcToken);
						return;
					}
					instruction = kernel.bias(args[0], args[1]);
					break;
				case EFunction::FUNC_GAIN:
					if (argsFound != 2)
					{
						SetError("gain accepts 2 arguemnts", funcToken);
						return;
					}
					instruction = kernel.gain(args[0], args[1]);
					break;
				case EFunction::FUNC_COS:
					if (argsFound != 1)
					{
						SetError("cos accepts 1 arguemnts", funcToken);
						return;
					}
					instruction = kernel.cos(args[0]);
					break;
				case EFunction::FUNC_SIN:
					if (argsFound != 1)
					{
						SetError("sin accepts 1 arguemnts", funcToken);
						return;
					}
					instruction = kernel.sin(args[0]);
					break;
				case EFunction::FUNC_TAN:
					if (argsFound != 1)
					{
						SetError("tan accepts 1 arguemnts", funcToken);
						return;
					}
					instruction = kernel.tan(args[0]);
					break;
				case EFunction::FUNC_ACOS:
					if (argsFound != 1)
					{
						SetError("acos accepts 1 arguemnts", funcToken);
						return;
					}
					instruction = kernel.acos(args[0]);
					break;
				case EFunction::FUNC_ASIN:
					if (argsFound != 1)
					{
						SetError("asin accepts 1 arguemnts", funcToken);
						return;
					}
					instruction = kernel.asin(args[0]);
					break;
				case EFunction::FUNC_ATAN:
					if (argsFound != 1)
					{
						SetError("atan accepts 1 arguemnts", funcToken);
						return;
					}
					instruction = kernel.atan(args[0]);
					break;
				case EFunction::FUNC_TIERS:
					if (argsFound != 2)
					{
						SetError("tiers accepts 2 arguemnts", funcToken);
						return;
					}
					instruction = kernel.tiers(args[0], args[1]);
					break;
				case EFunction::FUNC_SMOOTH_TIERS:
					if (argsFound != 2)
					{
						SetError("smoothTiers accepts 2 arguemnts", funcToken);
						return;
					}
					instruction = kernel.smoothTiers(args[0], args[1]);
					break;
				case EFunction::FUNC_BLEND:
					if (argsFound != 3)
					{
						SetError("blend accepts 3 arguemnts", funcToken);
						return;
					}
					instruction = kernel.blend(args[0], args[1], args[2]);
					break;
				case EFunction::FUNC_SELECT:
					if (argsFound != 5)
					{
						SetError("select accepts 5 arguemnts", funcToken);
						return;
					}
					instruction = kernel.select(args[0], args[1], args[2], args[3], args[4]);
					break;
				case EFunction::FUNC_SIMPLE_RIDGED_MULTIFRACTAL:
					if (argsFound != 6 && argsFound != 5)
					{
						SetError("simpleRidgedMultifractal accepts 5 or 6 arguemnts", funcToken);
						return;
					}
					else
					{
						CInstructionIndex boolRot = kernel.one(); // default to true;
						if (argsFound == 6)
							boolRot = args[5];
						instruction = kernel.simpleRidgedMultifractal(args[0], args[1], args[2], args[3], args[4], boolRot, nonConstArgIndex);
						if (nonConstArgIndex >= 0)
						{
							std::string msg = "simpleRidgedMultifractal requires argument index ";
							msg += std::to_string(nonConstArgIndex);
							msg += " to be constant";
							SetError(msg, funcToken);
							return;
						}
					}
					break;
				case EFunction::FUNC_SIMPLE_FBM:
					if (argsFound != 6 && argsFound != 5)
					{
						SetError("simplefBm accepts 5 or 6 arguemnts", funcToken);
						return;
					}
					else
					{
						CInstructionIndex boolRot = kernel.one(); // default to true;
						if (argsFound == 6)
							boolRot = args[5];
						instruction = kernel.simplefBm(args[0], args[1], args[2], args[3], args[4], boolRot, nonConstArgIndex);
						if (nonConstArgIndex >= 0)
						{
							std::string msg = "simplefBm requires argument index ";
							msg += std::to_string(nonConstArgIndex);
							msg += " to be constant";
							SetError(msg, funcToken);
							return;
						}
					}
					break;
				case EFunction::FUNC_SIMPLE_BILLOW:
					if (argsFound != 6 && argsFound != 5)
					{
						SetError("simpleBillow accepts 5 or 6 arguemnts", funcToken);
						return;
					}
					else
					{
						CInstructionIndex boolRot = kernel.one(); // default to true;
						if (argsFound == 6)
							boolRot = args[5];
						instruction = kernel.simpleBillow(args[0], args[1], args[2], args[3], args[4], boolRot, nonConstArgIndex);
						if (nonConstArgIndex >= 0)
						{
							std::string msg = "simpleBillow requires argument index ";
							msg += std::to_string(nonConstArgIndex);
							msg += " to be constant";
							SetError(msg, funcToken);
							return;
						}
					}
					break;
				case EFunction::FUNC_X:
					if (argsFound != 0)
					{
						SetError("x accepts 0 arguemnts", funcToken);
						return;
					}
					instruction = kernel.x();
					break;
				case EFunction::FUNC_Y:
					if (argsFound != 0)
					{
						SetError("y accepts 0 arguemnts", funcToken);
						return;
					}
					instruction = kernel.y();
					break;
				case EFunction::FUNC_Z:
					if (argsFound != 0)
					{
						SetError("z accepts 0 arguemnts", funcToken);
						return;
					}
					instruction = kernel.z();
					break;
				case EFunction::FUNC_W:
					if (argsFound != 0)
					{
						SetError("w accepts 0 arguemnts", funcToken);
						return;
					}
					instruction = kernel.w();
					break;
				case EFunction::FUNC_U:
					if (argsFound != 0)
					{
						SetError("u accepts 0 arguemnts", funcToken);
						return;
					}
					instruction = kernel.u();
					break;
				case EFunction::FUNC_V:
					if (argsFound != 0)
					{
						SetError("v accepts 0 arguemnts", funcToken);
						return;
					}
					instruction = kernel.v();
					break;
				case EFunction::FUNC_DX:
					if (argsFound != 2)
					{
						SetError("dx accepts 2 arguemnts", funcToken);
						return;
					}
					instruction = kernel.dx(args[0], args[1]);
					break;
				case EFunction::FUNC_DY:
					if (argsFound != 2)
					{
						SetError("dy accepts 2 arguemnts", funcToken);
						return;
					}
					instruction = kernel.dy(args[0], args[1]);
					break;
				case EFunction::FUNC_DZ:
					if (argsFound != 2)
					{
						SetError("dz accepts 2 arguemnts", funcToken);
						return;
					}
					instruction = kernel.dz(args[0], args[1]);
					break;
				case EFunction::FUNC_DW:
					if (argsFound != 2)
					{
						SetError("dw accepts 2 arguemnts", funcToken);
						return;
					}
					instruction = kernel.dw(args[0], args[1]);
					break;
				case EFunction::FUNC_DU:
					if (argsFound != 2)
					{
						SetError("du accepts 2 arguemnts", funcToken);
						return;
					}
					instruction = kernel.du(args[0], args[1]);
					break;
				case EFunction::FUNC_DV:
					if (argsFound != 2)
					{
						SetError("dv accepts 2 arguemnts", funcToken);
						return;
					}
					instruction = kernel.dv(args[0], args[1]);
					break;
				case EFunction::FUNC_SIGMOID:
					if (argsFound != 1 && argsFound != 3)
					{
						SetError("sigmoid accepts 1 or 3 arguemnts", funcToken);
						return;
					}
					if (argsFound == 1)
						instruction = kernel.sigmoid(args[0]);
					else
						instruction = kernel.sigmoid(args[0], args[1], args[2]);
					break;
				case EFunction::FUNC_SCALE_OFFSET:
					if (argsFound != 3)
					{
						SetError("scaleOffset accepts 3 arguemnts", funcToken);
						return;
					}
					instruction = kernel.scaleOffset(args[0], args[1], args[2]);
					break;
				case EFunction::FUNC_RADIAL:
					if (argsFound != 0)
					{
						SetError("radial accepts 0 arguemnts", funcToken);
						return;
					}
					instruction = kernel.radial();
					break;
				case EFunction::FUNC_CLAMP:
					if (argsFound != 3)
					{
						SetError("clamp accepts 3 arguemnts", funcToken);
						return;
					}
					instruction = kernel.clamp(args[0], args[1], args[2]);
					break;
				case EFunction::FUNC_RGBA:
				case EFunction::FUNC_COLOR:
					if (argsFound != 4)
					{
						SetError("rgba/color accepts 4 arguemnts", funcToken);
						return;
					}
					instruction = kernel.combineRGBA(args[0], args[1], args[2], args[3]);
					break;
				case EFunction::FUNC_HEX_TILE:
					if (argsFound != 1)
					{
						SetError("hexTile accepts 1 arguemnts", funcToken);
						return;
					}
					instruction = kernel.hexTile(args[0]);
					break;
				case EFunction::FUNC_HEX_BUMP:
					if (argsFound != 0)
					{
						SetError("hexBump accepts 0 arguemnts", funcToken);
						return;
					}
					instruction = kernel.hexBump();
					break;
				default:
					SetError("Unkown function type", funcToken);
					return;
				}
				Stack.erase(Stack.end() - argsFound, Stack.end());
				Stack.push_back(instruction);
			}
		}
	}
}