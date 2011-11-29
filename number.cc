#include <ostream>
#include <map>
#include <string>

#include <tr1/unordered_map>

#include "number.h"

std::wostream&
operator<< (std::wostream& os, const Too& t)
{
	return (os << t->number());
}
