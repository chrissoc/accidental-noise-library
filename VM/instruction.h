#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "../vectortypes.h"
#include <string>
#include <vector>

const int MaxSourceCount=20;
const int SInstructionDefaultIndex = 1000000;

namespace anl
{
enum EOpcodes
{
    OP_NOP,
    OP_Seed,
    OP_Constant,
	OP_NamedInput,
    OP_ValueBasis,
    OP_GradientBasis,
    OP_SimplexBasis,
    OP_CellularBasis,
    OP_Add,
    OP_Subtract,
    OP_Multiply,
    OP_Divide,
    OP_ScaleDomain,
    OP_ScaleX,
    OP_ScaleY,
    OP_ScaleZ,
    OP_ScaleW,
    OP_ScaleU,
    OP_ScaleV,
    OP_TranslateDomain,
    OP_TranslateX,
    OP_TranslateY,
    OP_TranslateZ,
    OP_TranslateW,
    OP_TranslateU,
    OP_TranslateV,
    OP_RotateDomain,
    OP_Blend,
    OP_Select,
    OP_Min,
    OP_Max,
    OP_Abs,
    OP_Pow,
    OP_Clamp,
    OP_Radial,
    OP_Sin,
    OP_Cos,
    OP_Tan,
    OP_ASin,
    OP_ACos,
    OP_ATan,
    OP_Bias,
    OP_Gain,
    OP_Tiers,
    OP_SmoothTiers,

    OP_X,
    OP_Y,
    OP_Z,
    OP_W,
    OP_U,
    OP_V,

    OP_DX,
    OP_DY,
    OP_DZ,
    OP_DW,
    OP_DU,
    OP_DV,

    OP_Sigmoid,

    // Patterns
    OP_HexTile,
    OP_HexBump,

    // RGBA operations
    OP_Color,
    OP_ExtractRed,
    OP_ExtractGreen,
    OP_ExtractBlue,
    OP_ExtractAlpha,
    OP_Grayscale,
    OP_CombineRGBA
};

struct SInstruction
{
    // Out fields
    double outfloat_;
    SRGBA outrgba_;

    // Source input indices
    unsigned int sources_[MaxSourceCount];

    // Instruction opcode
    unsigned int opcode_;
    //unsigned int seed_;

	// used with OP_NamedInput, name of the input variable to look up at runtime
	std::string namedInput;

    SInstruction()
    {
        for(unsigned int c=0; c<MaxSourceCount; ++c) sources_[c]=SInstructionDefaultIndex;
        outfloat_=0;
        outrgba_=SRGBA(0,0,0,1);
        opcode_=0;
        //seed_=12345;
    }
};

class CKernel;
class CNoiseExecutor;
class CInstructionIndex
{
public:
    CInstructionIndex(const CInstructionIndex &c)
    {
        index_=c.index_;
    }

    CInstructionIndex operator +(const CInstructionIndex &c)
    {
        return CInstructionIndex(index_+c.index_);
    }

    CInstructionIndex &operator +=(const CInstructionIndex &c)
    {
        index_+=c.index_;
        return *this;
    }

    CInstructionIndex operator -(const CInstructionIndex &c)
    {
        return CInstructionIndex(index_-c.index_);
    }

    CInstructionIndex &operator -=(const CInstructionIndex &c)
    {
        index_-=c.index_;
        return *this;
    }

    CInstructionIndex operator +(const unsigned int c)
    {
        return CInstructionIndex(index_+c);
    }

    CInstructionIndex &operator +=(const unsigned int c)
    {
        index_+=c;
        return *this;
    }

    CInstructionIndex operator -(const unsigned int c)
    {
        return CInstructionIndex(index_-c);
    }

    CInstructionIndex &operator -=(const unsigned int c)
    {
        index_-=c;
        return *this;
    }

    CInstructionIndex &operator++()
    {
        index_++;
        return *this;
    }

    CInstructionIndex operator++(int)
    {
        index_++;
        return *this;
    }

    CInstructionIndex &operator=(unsigned int c)
    {
        index_=c;
        return *this;
    }

    CInstructionIndex &operator=(const CInstructionIndex &c)
    {
        index_=c.index_;
        return *this;
    }

    bool operator!=(const CInstructionIndex &c)
    {
        return c.index_ != index_;
    }

    bool operator==(const CInstructionIndex &c)
    {
        return c.index_ == index_;
    }

    unsigned int GetIndex() const { return index_; }

    private:
    CInstructionIndex()
    {
        index_=0;
    }

    explicit CInstructionIndex(unsigned int i)
    {
        index_=i;
    }

    unsigned int index_;
    friend class CKernel;
    friend class CNoiseExecutor;
    friend class CExpressionBuilder;
};
};

#endif
