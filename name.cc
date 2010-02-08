#include <ostream>
#include <map>
#include <string>

#include <tr1/unordered_map>

#include "name.h"

std::tr1::unordered_map<std::wstring, Ner> Name::name_cache;

std::wostream&
operator<< (std::wostream& os, const Ner& n)
{
	return (os << n->string());
}
