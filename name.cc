#include <ostream>
#include <string>

#include "name.h"

std::wostream&
operator<< (std::wostream& os, const Name& n)
{
	return (os << n.string());
}
