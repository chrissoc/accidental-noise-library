
#pragma once

#include<string>
#include<vector>
#include<stack>

#include "../VM/vm.h"
#include "../VM/kernel.h"
#include "../VM/instruction.h"

namespace anl
{
	namespace lang {
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
			std::string keyword;
			int tokenLocation;
			int lineNumber;
		};

		namespace EBlend {
			enum BlendType {
				BLEND_INVALID = -1,
				BLEND_NONE = 0,
				BLEND_LINEAR = 1,
				BLEND_HERMITE = 2,
				BLEND_QUINTIC = 3,
			};
		}
		namespace EFunction {
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
		}

	}
}