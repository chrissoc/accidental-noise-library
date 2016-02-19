#pragma once
#include<stdint.h>
#include <vector>

class CBooleanBits
{
public:
#if _WIN64
	typedef uint64_t Type;
#elif _WIN32
	typedef uint32_t Type;
#else
	typedef uint32_t Type;
#endif

public:
	class BoolRef {
		friend CBooleanBits;
	private:
		Type& word;
		const Type mask;
		BoolRef(Type& w, int bit) : word(w), mask(1 << bit) {}
	public:
		BoolRef& operator=(bool value) 
		{
			if (value)
				word |= mask;
			else
				word &= ~mask;
			return *this; 
		}

		operator bool()
		{
			return (word & mask) != 0;
		}

		bool operator!()
		{
			return !((word & mask) != 0);
		}

		bool operator==(bool value)
		{
			return ((bool)*this) == value;
		}

		bool operator!=(bool value)
		{
			return ((bool)*this) != value;
		}
	};

	Type* BitArray;
	int32_t ArrayCapacity;// number of Type in BitArray
	int32_t ArraySize;// amount currently stored

protected:
	int32_t BitElementCount() 
	{
		return ArrayCapacity * sizeof(Type) * 8;
	}

	void Copy(Type* dest, int32_t destSize, Type* src, int32_t srcSize)
	{
		for (int i = 0; i < srcSize && i < destSize; ++i)
		{
			*dest = *src;
		}
	}

public:
	CBooleanBits()
		: BitArray(nullptr), ArrayCapacity(0), ArraySize(0)
	{
	}

private:
	//CBooleanBits(CBooleanBits& other) { throw "CBooleanBits Copy ctor not allowed"; }
	CBooleanBits(const CBooleanBits& other) { throw "CBooleanBits Copy ctor not allowed"; }
public:

	virtual ~CBooleanBits()
	{
		delete[] BitArray;
	}

	int32_t size() { return ArraySize; }

	void resize(int32_t newSize)
	{
		if (BitElementCount() < newSize)
		{
			const int32_t newElementCount = newSize / (sizeof(Type) * 8) + 1;
			Type* newArray = new Type[newElementCount];
			Copy(newArray, newElementCount, BitArray, ArrayCapacity);
			delete[] BitArray;
			ArrayCapacity = newElementCount;
			BitArray = newArray;
			
		}

		ArraySize = newSize;
	}

	void ClearAllBits()
	{
		Type* end = BitArray + ArrayCapacity;
		Type* begin = BitArray;
		while (begin != end)
		{
			*begin = 0;
			begin++;
		}
	}

private:
#if _DEBUG
	void BoundsCheck(int32_t index) const { if(index < 0 || index >= ArrayCapacity) throw "Bounds check failed for BooleanBits";  }
#define ANL_BOOLEANBITS_DEBUG_BOUNDS_CHECK(index) BoundsCheck(index)
#else
#define ANL_BOOLEANBITS_DEBUG_BOUNDS_CHECK(index)
#endif
	public:

	BoolRef operator[](int32_t index)
	{
		ANL_BOOLEANBITS_DEBUG_BOUNDS_CHECK(index / (sizeof(Type) * 8));
		return BoolRef(BitArray[index / (sizeof(Type) * 8)], index % (sizeof(Type) * 8));
	}

	const bool operator[](int32_t index) const
	{
		ANL_BOOLEANBITS_DEBUG_BOUNDS_CHECK(index / (sizeof(Type) * 8));
		return (BitArray[index / (sizeof(Type) * 8)] & (1ll << index % (sizeof(Type) * 8))) != 0;
	}
};