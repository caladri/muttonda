#include <ostream>

#include <tr1/unordered_map>

#include "scalar.h"

std::wostream&
operator<< (std::wostream& os, const Scalar& s)
{
	return (os << s.value());
}
