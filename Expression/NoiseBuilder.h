#pragma once

#include "../VM/vm.h"
#include "../VM/kernel.h"
#include "../VM/instruction.h"

namespace anl {
	namespace lang {

		enum BlendType {
			BLEND_NONE = 0,
			BLEND_LINEAR = 1,
			BLEND_HERMITE = 2,
			BLEND_QUINTIC = 3,
		};

		class INoiseSource
		{
		protected:
			anl::CKernel& Kernel;
			anl::CInstructionIndex Index;

			int ComponentScalingCount;
			bool ScalingFinilized;
			bool Error;

		public:
			INoiseSource(anl::CKernel& kernel, anl::CInstructionIndex index);
			virtual ~INoiseSource();

		public:
			anl::CInstructionIndex GetIndex();

			// component scaling, first use is for x, then second for y, etc...
			INoiseSource& operator[](double scale);

			// Domain scaling, use once to scale the entire input domain
			INoiseSource& operator()(double scale);

			// scales the output
			INoiseSource& operator*=(double scale);
			INoiseSource& operator*(double scale);

			INoiseSource& operator+=(double value);
			INoiseSource& operator+(double value);
			INoiseSource& operator+=(INoiseSource& other);
			INoiseSource& operator+(INoiseSource& other);

			INoiseSource& operator-=(double value);
			INoiseSource& operator-(double value);
			INoiseSource& operator-=(INoiseSource& other);
			INoiseSource& operator-(INoiseSource& other);

			friend INoiseSource& operator+=(double value, INoiseSource& self);
			friend INoiseSource& operator+(double value, INoiseSource& self);
			friend INoiseSource& operator-=(double value, INoiseSource& self);
			friend INoiseSource& operator-(double value, INoiseSource& self);
			friend INoiseSource& operator*=(double scale, INoiseSource& self);
			friend INoiseSource& operator*(double scale, INoiseSource& self);
		};

		class GradientBasis : public INoiseSource
		{
		public:
			GradientBasis(anl::CKernel& kernel, BlendType blendType, unsigned int seed)
				: INoiseSource(kernel, kernel.gradientBasis(kernel.constant(blendType), kernel.seed(seed)))
			{
			};
		};

		class SimplexBasis : public INoiseSource
		{
		public:
			SimplexBasis(anl::CKernel& kernel, unsigned int seed)
				: INoiseSource(kernel, kernel.simplexBasis(kernel.seed(seed)))
			{
			};
		};

		// anl::CKernel k;
		// INoiseSource ns = 0.5 * GradientBasis(k, BLEND_NONE, 12359) + 0.5 * SimplexBasis(k, 98713)(0.5);
	}
}
