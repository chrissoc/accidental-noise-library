
#pragma once

#include<string>
#include<vector>
#include<stack>

#include "../VM/vm.h"
#include "../VM/kernel.h"
#include "../VM/instruction.h"

#include "NoiseParserToken.h"
#include "NoiseParserAST.h"

/*
whiteSpace ::= ' ' | '\n' | '\r' | '\t'
printableChar ::= whiteSpace | [!-~]
commentLine ::= '/' '/' printableChar*  '\n'
commentBlock ::= '/' '*' printableChar* '*' '/'

letter ::= [a-zA-Z]
keyword ::= letter+
digit ::= [0-9]
number ::= digit+ ('.' digit+)?
domainOp ::= 's' | 't' | 'r'
component ::= 'x' | 'y' | 'z' | 'w' | 'u' | 'v'
domainModifierLeft ::= '<' domainOp component? ':'
domainModifierRight ::= '>'

domainOperator ::= domainModifierLeft argumentList domainModifierRight
argumentList ::= expression (',' argumentList)*
functionCall ::= keyword ( '(' argumentList* ')' )?
object ::= functionCall | grouping | negative | number | keyword
domainPrecedence ::= (domainOperator domainPrecedence) | object
mult ::= domainPrecedence (('*' | '/') mult)?
add ::= mult (('+' | '-') add)?

grouping ::= '(' expression ')'
negative ::= '-' (object | domainPrecendence)
expression ::= add | negative | grouping
statement ::= expression ';'
assignment ::= (keyword '=')? statement
program ::= assignment program?
*/

namespace anl
{
	namespace lang {
		class NoiseParser
		{
		public:
			typedef std::string ParseString;
		private:
			class Tokenizer
			{
				ParseString Data;
				int DataIndex;
				int RewindIndex;// for UnGet()
				int RewindIndex2;// for second UnGet()
				int LineNumber;
				int RewindLineNumber;
				int RewindLineNumber2;
				int ColumnStartOffset;
				int RewindColumnStartOffset;
				int RewindColumnStartOffset2;
				ParseString LastError;

			private:
				void SetError(ParseString msg);
				bool IsEof();

			public:
				Tokenizer(ParseString input) : Data(input), DataIndex(0), RewindIndex(-1), RewindIndex2(-1),
					LineNumber(1), RewindLineNumber(-1), RewindLineNumber2(-1),// line number starts at 1 since we count lines from 1
					ColumnStartOffset(0), RewindColumnStartOffset(-1), RewindColumnStartOffset2(-1)
				{
				}
				ParseString GetLastError() { return LastError; }
				bool IsError() { return LastError.length() != 0 ? true : false; }
				void UnGet();
				void SetUnGetLocation();
				Token GetToken();
			};

		private:
			typedef NoiseParserAST::Node::NodePtr NodePtr;// for our ease of use
			Tokenizer tokens;
			CKernel Kernel;
			CNoiseExecutor VM;
			CInstructionIndex NOP;
			CInstructionIndex ParseResult;
			std::vector<ParseString> ErrorMsgs;
			std::vector<NodePtr> Stack;
			int TotalFolds, TotalInstructions;
			bool Error;

		public:
			static EBlend::BlendType KeywordToBlend(const std::string& keyword);
			static EFunction::Function KeywordToFunc(const std::string& keyword);
			static bool IsKeyword_OP_ValueBasis(const std::string& keyword);
			static bool IsKeyword_OP_GradientBasis(const std::string& keyword);
			static bool IsKeyword_OP_SimplexBasis(const std::string& keyword);
			static bool IsKeyword_True(const std::string& keyword);
			static bool IsKeyword_False(const std::string& keyword);
			static bool IsKeyword_e(const std::string& keyword);
			static bool IsKeyword_pi(const std::string& keyword);

		private:
			bool IsEof(Token& token);

			void SetError(ParseString msg, const Token& cause);
			void SetError(ParseString msg);
			bool domainOperator();
			bool argumentList();
			bool functionCall();
			bool object();
			bool domainPrecedence();
			bool mult();
			bool add();
			bool grouping();
			bool negative();
			bool expression();
			bool statement();
			bool assignment();
			bool program();

		public:
			NoiseParser(ParseString expression)
				: tokens(expression), VM(Kernel), Error(false), NOP(Kernel.constant(0)), ParseResult(NOP), TotalFolds(0), TotalInstructions(0) {}
			virtual ~NoiseParser();

			// returns true for success
			bool Parse();
			ParseString FormErrorMsgs();
			CNoiseExecutor& GetVM() { return VM; }
			CInstructionIndex GetParseResult() { return ParseResult; }
			CKernel& GetKernel() { return Kernel; }
			int GetTotalFolds() { return TotalFolds; }
			int GetTotalInstructions() { return TotalInstructions; }
		};

	}
}