#include <ostream>

#include "scalar.h"

bool
Scalar::operator< (const Scalar& b) const
{
	return (v_ < b.v_);
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

std::wostream&
operator<< (std::wostream& os, const Scalar& s)
{
	return (os << s.value());
}
