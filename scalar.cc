#include <math.h>

#include <ostream>

#include "scalar.h"

Scalar::Scalar(void)
: v_(nanf(""))
{ }

Scalar::Scalar(const unsigned& v)
: v_(v)
{ }

Scalar::Scalar(const Scalar& src)
: v_(src.v_)
{ }

unsigned
Scalar::value(void) const
{
	return (v_);
}

bool
Scalar::operator== (const Scalar& b) const
{
	return (v_ == b.v_);
}

Scalar
Scalar::operator* (const Scalar& b) const
{
	return (Scalar(v_ * b.v_));
}

Scalar
Scalar::operator+ (const Scalar& b) const
{
	return (Scalar(v_ + b.v_));
}

std::ostream&
operator<< (std::ostream& os, const Scalar& s)
{
	return (os << s.value());
}
