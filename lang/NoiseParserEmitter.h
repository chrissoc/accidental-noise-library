
#pragma once

#include<string>
#include<vector>
#include<stack>
#include <memory>

#include "../VM/vm.h"
#include "../VM/kernel.h"
#include "../VM/instruction.h"


namespace anl 
{
	namespace lang {
		namespace NoiseParserAST
		{

			class number;
			class keyword;
			class string;
			class domainOperator;
			class argumentList;
			class functionCall;
			class object;
			class domainPrecedence;
			class mult;
			class add;
			class grouping;
			class negative;
			class expression;
			class statement;
			class assignment;
			class program;
			// An Emitter provides a way to traverse the AST and emitt instructions
			// in a variaty of ways. The Emitter is passed to a Nodes Emit method
			class Emitter {
			protected:
				std::vector<std::string> ErrorList;
				void SetError(std::string msg, const Token& t);

			public:
				bool IsError() { return ErrorList.size() != 0; }
				std::string FormErrorMsgs();

				virtual void Emit(number* node) = 0;
				virtual void Emit(keyword* node) = 0;
				virtual void Emit(string * node) = 0;
				virtual void Emit(domainOperator* node) = 0;
				virtual void Emit(argumentList* node) = 0;
				virtual void Emit(functionCall* node) = 0;
				virtual void Emit(object* node) = 0;
				virtual void Emit(domainPrecedence* node) = 0;
				virtual void Emit(mult* node) = 0;
				virtual void Emit(add* node) = 0;
				virtual void Emit(grouping* node) = 0;
				virtual void Emit(negative* node) = 0;
				virtual void Emit(expression* node) = 0;
				virtual void Emit(statement* node) = 0;
				virtual void Emit(assignment* node) = 0;
				virtual void Emit(program* node) = 0;
			};
			class ANLEmitter : public Emitter {
			protected:
				// Stack may have one of the following out of bounds indices in it
				// indicating a special case
				enum {
					// special case of CInstructionIndex indicating that the
					// value is not yet in the kernal but the value is a string
					// located on the StringStack
					STACK_INDEX_STRING_STACK = 1000000001,
				};
				std::vector<CInstructionIndex> Stack;
				std::vector<std::string> StringStack;
				// provided to simply pass integer data between functions
				std::vector<int> StackIntegers;
				std::map<std::string, std::unique_ptr<CInstructionIndex>> Variables;
				std::map<std::string, assignment*> VariableCandidates;
				CKernel& kernel;
				CInstructionIndex NegativeOne;
				CInstructionIndex NOP;
				CInstructionIndex ProgramStart;

				void SetupFunctionCall(int argCount, EFunction::Function func, Token& funcToken);
				void SetupDomainOperator(CInstructionIndex instruction, int argsFound, const Token& t);
			public:
				ANLEmitter(CKernel& k) : kernel(k), NegativeOne(kernel.constant(-1.0)), NOP(kernel.zero()), ProgramStart(kernel.zero()) {}

				CInstructionIndex GetProgramStart() { return ProgramStart; }

				virtual void Emit(number* node) override;
				virtual void Emit(keyword* node) override;
				virtual void Emit(string * node) override;
				virtual void Emit(domainOperator* node) override;
				virtual void Emit(argumentList* node) override;
				virtual void Emit(functionCall* node) override;
				virtual void Emit(object* node) override;
				virtual void Emit(domainPrecedence* node) override;
				virtual void Emit(mult* node) override;
				virtual void Emit(add* node) override;
				virtual void Emit(grouping* node) override;
				virtual void Emit(negative* node) override;
				virtual void Emit(expression* node) override;
				virtual void Emit(statement* node) override;
				virtual void Emit(assignment* node) override;
				virtual void Emit(program* node) override;
			};
			/*
			class SomeOtherExecutionContext : public Emitter {
			};
			*/
		}
	}
}