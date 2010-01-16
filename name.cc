#include <ostream>
#include <map>
#include <string>

#include <tr1/unordered_map>

#include "name.h"

std::wostream&
operator<< (std::wostream& os, const Ref<Name>& n)
{
	return (os << n->string());
}
