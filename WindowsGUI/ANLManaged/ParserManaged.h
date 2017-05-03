#pragma once

#include <Windows.h>
#include <vcclr.h>
#using <System.dll>
#include <msclr\marshal_cppstd.h>
#include "../../lang/NoiseParser.h"

using namespace System;

namespace ANLManaged {

	ref class CNoiseExecutorManaged;

	public ref class CInstructionIndexManaged
	{
	protected:
		anl::CInstructionIndex* ii;

	public:

		// copies the provided kernel
		CInstructionIndexManaged(const anl::CInstructionIndex& iidx);

		// Deallocate the native object on a destructor
		~CInstructionIndexManaged();

	protected:
		// Deallocate the native object on the finalizer just in case no destructor is called
		!CInstructionIndexManaged();


	public:
		anl::CInstructionIndex& GetIndex();
	};

	public ref class ParserManaged
	{
	protected:
		anl::lang::NoiseParser* parser;

	public:

		ParserManaged(String ^ expression);

		// Deallocate the native object on a destructor
		~ParserManaged();

	protected:
		// Deallocate the native object on the finalizer just in case no destructor is called
		!ParserManaged();

	public:

		// returns true for success
		bool Parse();
		String ^ FormErrorMsgs();
		int GetTotalFolds();
		int GetTotalInstructions();
		CNoiseExecutorManaged ^ GetVMCopy();
		anl::CKernel& GetKernelRef() { return parser->GetKernel(); }
		CInstructionIndexManaged ^ GetInstructionIndexRoot();
	};


	public ref class CNoiseExecutorManaged
	{
	protected:
		anl::CNoiseExecutor* vm;
		ParserManaged ^ Parser;// hold reference to parser to ensure the kernel stays alive.
		//CKernelManaged ^ kernel;// hold a reference to the kernel to make sure its not GCed
	public:

		// copies the provided kernel
		//CNoiseExecutorManaged(CKernelManaged ^ kernel);
		CNoiseExecutorManaged(ParserManaged ^ parser);

		// Deallocate the native object on a destructor
		~CNoiseExecutorManaged();

	protected:
		// Deallocate the native object on the finalizer just in case no destructor is called
		!CNoiseExecutorManaged();

	public:

		double evaluateScalar(double x, double y, CInstructionIndexManaged ^ idx);
		double evaluateScalar(double x, double y, double z, CInstructionIndexManaged ^ idx);
		double evaluateScalar(double x, double y, double z, double w, CInstructionIndexManaged ^ idx);
		double evaluateScalar(double x, double y, double z, double w, double u, double v, CInstructionIndexManaged ^ idx);
	};

}
