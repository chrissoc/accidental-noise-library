#include "NoiseBuilder.h"

namespace anl {
	namespace lang {

		INoiseSource::INoiseSource(anl::CKernel& kernel, anl::CInstructionIndex index)
			: Kernel(kernel), Index(index),
			ComponentScalingCount(0), ScalingFinilized(false), Error(false)
		{
		}

		INoiseSource::~INoiseSource()
		{

		}

		anl::CInstructionIndex INoiseSource::GetIndex()
		{
			return Index;
		}

		// component scaling, first use is for x, then second for y, etc...
		INoiseSource& INoiseSource::operator[](double scale)
		{
			if (ScalingFinilized || ComponentScalingCount >= 6)
				Error = true;

			switch (ComponentScalingCount)
			{
			case 0: Index = Kernel.scaleX(Index, Kernel.constant(scale)); break;
			case 1: Index = Kernel.scaleY(Index, Kernel.constant(scale)); break;
			case 2: Index = Kernel.scaleZ(Index, Kernel.constant(scale)); break;
			case 3: Index = Kernel.scaleU(Index, Kernel.constant(scale)); break;
			case 4: Index = Kernel.scaleV(Index, Kernel.constant(scale)); break;
			case 5: // fall-through
			default:Index = Kernel.scaleW(Index, Kernel.constant(scale)); break;
			}
			ComponentScalingCount += 1;

			return *this;
		}

		// Domain scaling, use once to scale the entire input domain
		INoiseSource& INoiseSource::operator()(double scale)
		{
			if (ScalingFinilized || ComponentScalingCount != 0)
				Error = true;

			ScalingFinilized = true;

			Index = Kernel.scaleDomain(Index, Kernel.constant(scale));
			return *this;
		}

		// scales the output
		INoiseSource& INoiseSource::operator*=(double scale)
		{
			Index = Kernel.multiply(Index, Kernel.constant(scale));
			return *this;
		}

		// scales the output
		INoiseSource& INoiseSource::operator*(double scale)
		{
			return *this *= scale;
		}

		INoiseSource& INoiseSource::operator+=(double value)
		{
			Index = Kernel.add(Index, Kernel.constant(value));
			return *this;
		}

		INoiseSource& INoiseSource::operator+(double value)
		{
			return *this += value;
		}

		INoiseSource& INoiseSource::operator+=(INoiseSource& other)
		{
			Index = Kernel.add(Index, other.Index);
			return *this;
		}

		INoiseSource& INoiseSource::operator+(INoiseSource& other)
		{
			return *this += other;
		}

		INoiseSource& INoiseSource::operator-=(double value)
		{
			Index = Kernel.subtract(Index, Kernel.constant(value));
			return *this;
		}

		INoiseSource& INoiseSource::operator-(double value)
		{
			return *this -= value;
		}

		INoiseSource& INoiseSource::operator-=(INoiseSource& other)
		{
			Index = Kernel.subtract(Index, other.Index);
			return *this;
		}

		INoiseSource& INoiseSource::operator-(INoiseSource& other)
		{
			return *this -= other;
		}

		INoiseSource& operator+=(double value, INoiseSource& self)
		{
			return self += value;
		}

		INoiseSource& operator+(double value, INoiseSource& self)
		{
			return self += value;
		}

		INoiseSource& operator-=(double value, INoiseSource& self)
		{
			self.Index = self.Kernel.subtract(self.Kernel.constant(value), self.Index);
			return self;
		}

		INoiseSource& operator-(double value, INoiseSource& self)
		{
			return value -= self;
		}

		INoiseSource& operator*=(double scale, INoiseSource& self)
		{
			return self *= scale;
		}

		INoiseSource& operator*(double scale, INoiseSource& self)
		{
			return self * scale;
		}

	}
}