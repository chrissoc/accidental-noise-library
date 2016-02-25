
#pragma once

#include<string>
#include<vector>
#include<stack>

#include "../VM/vm.h"
#include "../VM/kernel.h"
#include "../VM/instruction.h"

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
negative ::= '-' object
expression ::= add | negative | grouping
statement ::= expression ';'
assignment ::= (keyword '=')? statement
program ::= assignment program?
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
				//L_CHEVRON,// replaced with DOMAIN_* operations
				R_CHEVRON,
				L_BRACKET,
				R_BRACKET,
				COMMA,
				SEMI_COLON,
				ASSIGNMENT,
				MULT,
				DIV,
				ADD,
				SUB,
				DOMAIN_OP_BEGIN,
				DOMAIN_SCALE = DOMAIN_OP_BEGIN,
				DOMAIN_SCALE_X,
				DOMAIN_SCALE_Y,
				DOMAIN_SCALE_Z,
				DOMAIN_SCALE_W,
				DOMAIN_SCALE_U,
				DOMAIN_SCALE_V,
				DOMAIN_TRANSLATE,
				DOMAIN_TRANSLATE_X,
				DOMAIN_TRANSLATE_Y,
				DOMAIN_TRANSLATE_Z,
				DOMAIN_TRANSLATE_W,
				DOMAIN_TRANSLATE_U,
				DOMAIN_TRANSLATE_V,
				DOMAIN_ROTATE,
				DOMAIN_OP_END,
			};
			TokenType token;
			double number;
			ParseString keyword;
			int tokenLocation;
			int lineNumber;
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
			FUNC_VALUE_BASIS,
			FUNC_GRADIENT_BASIS,
			FUNC_SIMPLEX_BASIS,
			FUNC_CELLULAR_BASIS,
			FUNC_MAX,
			FUNC_MIN,
			FUNC_ABS,
			FUNC_POW,
			FUNC_BIAS,
			FUNC_GAIN,
			FUNC_COS,
			FUNC_SIN,
			FUNC_TAN,
			FUNC_ACOS,
			FUNC_ASIN,
			FUNC_ATAN,
			FUNC_TIERS,
			FUNC_SMOOTH_TIERS,
			FUNC_BLEND,
			FUNC_SELECT,
			FUNC_SIMPLE_RIDGED_MULTIFRACTAL,
			FUNC_SIMPLE_FBM,
			FUNC_SIMPLE_BILLOW,
			FUNC_X,
			FUNC_Y,
			FUNC_Z,
			FUNC_W,
			FUNC_U,
			FUNC_V,
			FUNC_DX,
			FUNC_DY,
			FUNC_DZ,
			FUNC_DW,
			FUNC_DU,
			FUNC_DV,
			FUNC_SIGMOID,
			FUNC_SCALE_OFFSET,
			FUNC_RADIAL,
			FUNC_CLAMP,
			FUNC_RGBA,// uses combineRGBA
			FUNC_COLOR,// alias for combineRGBA
			FUNC_HEX_TILE,
			FUNC_HEX_BUMP,
		};

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
		Tokenizer tokens;
		CKernel Kernel;
		CNoiseExecutor VM;
		CInstructionIndex NOP;
		CInstructionIndex ParseResult;
		std::vector<ParseString> ErrorMsgs;
		std::map<ParseString, CInstructionIndex*> Variables;
		std::vector<CInstructionIndex> Stack;
		int TotalFolds, TotalInstructions;
		bool Error;

	private:
		bool IsEof(Token& token);
		CInstructionIndex TopNPop();// returns the top of the stack and pops it at the same time.
		BlendType KeywordToBlend(const ParseString& keyword);
		Function KeywordToFunc(const ParseString& keyword);
		// variables include constants
		bool KeywordToVariable(const ParseString& keyword);
		void AddVariable(const ParseString& keyword, const CInstructionIndex& value);
		void SetError(ParseString msg, const Token& cause);
		void SetError(ParseString msg);
		bool domainOperator(int& argsFound, Token::TokenType& OperationToken);
		bool argumentList(int& argsFound);
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