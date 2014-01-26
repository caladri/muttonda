#include <ostream>
#include <map>
#include <string>
#include <unordered_map>

#include "number.h"

std::wostream&
operator<< (std::wostream& os, const Too& t)
{
	return (os << t->number());
}
