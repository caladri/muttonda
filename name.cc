#include <ostream>
#include <map>
#include <string>
#include <unordered_map>

#include "name.h"

std::wostream&
operator<< (std::wostream& os, const Ner& n)
{
	return (os << n->string());
}
