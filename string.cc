#include <ostream>
#include <string>

#include <tr1/unordered_map>

#include "string.h"

std::wostream&
operator<< (std::wostream& os, const String& s)
{
	std::wstring str = s.string();
	if (str == L"\n")
		str = L"\\n";
	return (os << '"' << str << '"');
}
