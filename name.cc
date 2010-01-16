#include <ostream>
#include <map>
#include <string>

#include "name.h"

std::wostream&
operator<< (std::wostream& os, const Ref<Name>& n)
{
	return (os << n->string());
}
