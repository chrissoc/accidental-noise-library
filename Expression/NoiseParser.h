
#pragma once

#include<string>
#include<vector>

#include "../VM/vm.h"
#include "../VM/kernel.h"
#include "../VM/instruction.h"

/*
letter ::= [a-zA-Z]
keyword ::= letter+
digit ::= [0-9]
number ::= digit+ ('.' digit+)?
axisScalar ::= '[' expression ']'
domainScalar ::= '<' expression '>'
argument ::= expression | keyword
argumentList ::= argument (',' argumentList)*
functionCall ::= keyword '(' argumentList* ')'
object ::= functionCall | grouping | negative | number
scalar ::= (axisScalar | domainScalar)? object
mult ::= scalar ('*' mult)?
add ::= mult (('+' | '-') add)?

grouping ::= '(' expression ')'
negative ::= '-' expression
expression ::= grouping | negative | add
*/

namespace anl 
{
	class NoiseParser
	{
	public:
		typedef std::string ParseString;
	private:
		struct Token {
			enum TokenType {
				NONE,
				TOKEN_EOF,
				TOKEN_ERROR,
				KEYWORD,
				NUMBER,
				L_PAREN,
				R_PAREN,
				L_CHEVRON,
				R_CHEVRON,
				L_BRACKET,
				R_BRACKET,
				COMMA,
				MULT,
				ADD,
				SUB,
			};
			TokenType token;
			double number;
			ParseString keyword;
			int tokenLocation;
		};
		enum BlendType {
			BLEND_INVALID = -1,
			BLEND_NONE = 0,
			BLEND_LINEAR = 1,
			BLEND_HERMITE = 2,
			BLEND_QUINTIC = 3,
		};
		enum Function {
			FUNC_INVALID,
			FUNC_SIMPLEX_BASIS,
			FUNC_GRADIENT_BASIS,
		};

		class Tokenizer
		{
			ParseString Data;
			int DataIndex;
			int RewindIndex;// for UnGet()
			ParseString LastError;

		private:
			void SetError(ParseString msg);
			bool IsEof();

		public:
			Tokenizer(ParseString input) : Data(input), DataIndex(0), RewindIndex(-1) {}
			ParseString GetLastError() { return LastError; }
			bool IsError() { return LastError.length() != 0 ? true : false; }
			void UnGet();
			void SetUnGetLocation();
			Token GetToken();
		};

	private:
		Tokenizer tokens;
		CKernel Kernel;
		CNoiseExecutor VM;
		CInstructionIndex NOP;
		CInstructionIndex ParseResult;
		std::vector<ParseString> ErrorMsgs;
		bool Error;

	private:
		bool IsEof(Token& token);
		BlendType KeywordToBlend(const ParseString& keyword);
		Function KeywordToFunc(const ParseString& keyword);
		void SetError(ParseString msg, const Token& cause);
		void SetError(ParseString msg);
		bool axisScalar(CInstructionIndex& instruction);
		bool domainScalar(CInstructionIndex& instruction);
		bool argument(double& result);
		bool argumentList(double args[], int argc, int& argsFound);
		bool functionCall(CInstructionIndex& instruction);
		//bool scaledFunctionCall(CInstructionIndex& instruction);
		bool object(CInstructionIndex& instruction);
		bool scalar(CInstructionIndex& instruction);
		bool mult(CInstructionIndex& instruction);
		bool add(CInstructionIndex& instruction);
		bool grouping(CInstructionIndex& instruction);
		bool negative(CInstructionIndex& instruction);
		bool expression(CInstructionIndex& instruction);

	public:
		NoiseParser(ParseString expression)
			: tokens(expression), VM(Kernel), Error(false), NOP(Kernel.constant(0)), ParseResult(NOP) {}
		virtual ~NoiseParser() {}

		// returns true for success
		bool Parse();
		ParseString FormErrorMsgs();
		CNoiseExecutor& GetVM() { return VM; }
		CInstructionIndex GetParseResult() { return ParseResult; }
		CKernel& GetKernel() { return Kernel; }
	};

}