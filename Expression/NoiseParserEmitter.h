
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
	namespace NoiseParserAST
	{

		class number;
		class keyword;
		class domainModifierLeft;
		class domainModifierRight;
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
			std::vector<CInstructionIndex> Stack;
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

			virtual void Emit(number* node);
			virtual void Emit(keyword* node);
			virtual void Emit(domainOperator* node);
			virtual void Emit(argumentList* node);
			virtual void Emit(functionCall* node);
			virtual void Emit(object* node);
			virtual void Emit(domainPrecedence* node);
			virtual void Emit(mult* node);
			virtual void Emit(add* node);
			virtual void Emit(grouping* node);
			virtual void Emit(negative* node);
			virtual void Emit(expression* node);
			virtual void Emit(statement* node);
			virtual void Emit(assignment* node);
			virtual void Emit(program* node);
		};
		/*
		class SomeOtherExecutionContext : public Emitter {
		};
		*/
	}
}