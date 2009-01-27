#include <ostream>
#include <string>

#include "name.h"

std::ostream&
operator<< (std::ostream& os, const Name& n)
{
	return (os << n.str());
}
