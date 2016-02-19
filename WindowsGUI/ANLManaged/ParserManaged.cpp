#include "ParserManaged.h"
namespace ANLManaged {

		// copies the provided kernel
	CInstructionIndexManaged::CInstructionIndexManaged(const anl::CInstructionIndex& iidx) : ii(new anl::CInstructionIndex(iidx))
	{
	}

		// Deallocate the native object on a destructor
	CInstructionIndexManaged::~CInstructionIndexManaged() {
		delete ii;
	}

	// Deallocate the native object on the finalizer just in case no destructor is called
	CInstructionIndexManaged::!CInstructionIndexManaged() {
		delete ii;
	}

	anl::CInstructionIndex& CInstructionIndexManaged::GetIndex() 
	{ 
		return *ii;
	}

	ParserManaged::ParserManaged(String ^ expression) : parser(new anl::NoiseParser(msclr::interop::marshal_as<std::string>(expression))) 
	{
	}

	// Deallocate the native object on a destructor
	ParserManaged::~ParserManaged() {
		delete parser;
	}

	// Deallocate the native object on the finalizer just in case no destructor is called
	ParserManaged::!ParserManaged() {
		delete parser;
	}


	// returns true for success
	bool ParserManaged::Parse() { return parser->Parse(); }
	String ^ ParserManaged::FormErrorMsgs() { return msclr::interop::marshal_as<String^>(parser->FormErrorMsgs()); }
	int ParserManaged::GetTotalFolds() { return parser->GetTotalFolds(); }
	int ParserManaged::GetTotalInstructions() { return parser->GetTotalInstructions(); }
	CNoiseExecutorManaged ^ ParserManaged::GetVMCopy() { return gcnew CNoiseExecutorManaged(this); }
	CInstructionIndexManaged ^ ParserManaged::GetInstructionIndexRoot() { return gcnew CInstructionIndexManaged(parser->GetParseResult()); }

	// copies the provided kernel
	//CNoiseExecutorManaged::CNoiseExecutorManaged(CKernelManaged ^ kernel) : kernel(kernel), vm(new anl::CNoiseExecutor(kernel->GetKernelUnmanaged())) {}
	CNoiseExecutorManaged::CNoiseExecutorManaged(ParserManaged ^ parser) : Parser(parser), vm(new anl::CNoiseExecutor(parser->GetKernelRef()))
	{
	}

	// Deallocate the native object on a destructor
	CNoiseExecutorManaged::~CNoiseExecutorManaged() {
		delete vm;
	}
	// Deallocate the native object on the finalizer just in case no destructor is called
	CNoiseExecutorManaged::!CNoiseExecutorManaged() {
		delete vm;
	}

	double CNoiseExecutorManaged::evaluateScalar(double x, double y, CInstructionIndexManaged ^ idx) { return vm->evaluateScalar(x, y, idx->GetIndex()); }
	double CNoiseExecutorManaged::evaluateScalar(double x, double y, double z, CInstructionIndexManaged ^ idx) { return vm->evaluateScalar(x, y, z, idx->GetIndex()); }
	double CNoiseExecutorManaged::evaluateScalar(double x, double y, double z, double w, CInstructionIndexManaged ^ idx) { return vm->evaluateScalar(x, y, z, w, idx->GetIndex()); }
	double CNoiseExecutorManaged::evaluateScalar(double x, double y, double z, double w, double u, double v, CInstructionIndexManaged ^ idx) { return vm->evaluateScalar(x, y, z, w, u, v, idx->GetIndex()); }
}
