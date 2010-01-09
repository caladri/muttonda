#include <ostream>
#include <string>

#include "string.h"

std::wostream&
operator<< (std::wostream& os, const String& s)
{
	std::wstring str = s.string();
	if (str == L"\n")
		str = L"\\n";
	return (os << '"' << str << '"');
}
