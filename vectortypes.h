#ifndef COMMON_VECTOR_TYPES
#define COMMON_VECTOR_TYPES

#include "VM/types.h"

namespace anl
{
	//typedef rhs4D<float> SRGBA;
	
	struct SRGBA
	{
		SRGBA() : r(0), g(0), b(0), a(0){}
		SRGBA(int al) : r(a), g(a), b(a), a(al) {}
		SRGBA(const  SRGBA &rhs) : r(rhs.r), g(rhs.g), b(rhs.b), a(rhs.a){}
		SRGBA(const double t1, const double t2, const double t3, const double t4) : r(t1), g(t2), b(t3), a(t4){}
		~SRGBA(){}

    bool operator ==(const SRGBA &rhs) const
    {
        if(r==rhs.r && g==rhs.g && b==rhs.b && a==rhs.a) return true;
        return false;
    };

    bool operator !=(const SRGBA &rhs) const
    {
        if(r==rhs.r && g==rhs.g && b==rhs.b && a==rhs.a) return false;
        return true;
    };

    SRGBA operator +(const SRGBA &rhs) const
    {
        return SRGBA(r+rhs.r, g+rhs.g, b+rhs.b, a+rhs.a);
    };

    SRGBA operator -(const SRGBA &rhs) const
    {
        return SRGBA(r-rhs.r, g-rhs.g, b-rhs.b, a-rhs.a);
    };

    SRGBA operator *(const SRGBA &rhs) const
    {
        return SRGBA(r*rhs.r, g*rhs.g, b*rhs.b, a*rhs.a);
    };

	SRGBA operator *(const double &rhs) const
	{
		return SRGBA(r*rhs, g*rhs, b*rhs, a*rhs);
	}

    SRGBA operator /(const SRGBA &rhs) const
    {
        return SRGBA(r/rhs.r, g/rhs.g, b/rhs.b, a/rhs.a);
    };

	SRGBA operator /(const double &rhs) const
	{
		return SRGBA(r/rhs, g/rhs, b/rhs, a/rhs);
	}

    SRGBA operator -() const
    {
        return SRGBA(-r, -g, -b, -a);
    };

    SRGBA &operator +=(const SRGBA &rhs)
    {
        r += rhs.r;
        g += rhs.g;
        b += rhs.b;
        a += rhs.a;
        return (*this);
    };

    SRGBA &operator -=(const SRGBA &rhs)
    {
        r -= rhs.r;
        g -= rhs.g;
        b -= rhs.b;
        a -= rhs.a;
        return (*this);
    };

    SRGBA &operator *=(const SRGBA &rhs)
    {
        r *= rhs.r;
        g *= rhs.g;
        b *= rhs.b;
        a *= rhs.a;
        return (*this);
    };

    SRGBA &operator /=(const SRGBA &rhs)
    {
        r /= rhs.r;
        g /= rhs.g;
        b /= rhs.b;
        a /= rhs.a;
        return (*this);
    };



    SRGBA operator +(const float &val) const
    {
        return SRGBA(r+val, g+val, b+val, a+val);
    };

    SRGBA operator -(const float &val) const
    {
        return SRGBA(r-val, g-val, b-val, a-val);
    };

    SRGBA operator *(const float &val) const
    {
        return SRGBA(r*val, g*val, b*val, a*val);
    };

    SRGBA operator /(const float &val) const
    {
        return SRGBA(r/val, g/val, b/val, a/val);
    };

    SRGBA &operator +=(const float &val)
    {
        r+=val;
        g+=val;
        b+=val;
        a+=val;
        return (*this);
    };

    SRGBA &operator -=(const float &val)
    {
        r-=val;
        g-=val;
        b-=val;
        a-=val;
        return (*this);
    };

    SRGBA &operator *=(const float &val)
    {
        r*=val;
        g*=val;
        b*=val;
        a*=val;
        return (*this);
    }

    SRGBA &operator /=(const float &val)
    {
        r/=val;
        g/=val;
        b/=val;
        a/=val;
        return (*this);
    };

		double r;
		double g;
		double b;
		double a;
	};
};



#endif
