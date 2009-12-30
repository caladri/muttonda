#include <ostream>
#include <string>

#include "string.h"

String::String(void)
: str_(L"")
{ }

String::String(const std::wstring& str)
: str_(str)
{ }

String::String(const String& src)
: str_(src.str_)
{ }

std::wstring
String::string(void) const
{
	return (str_);
}

bool
String::operator== (const String& b) const
{
	return (str_ == b.str_);
}

std::wostream&
operator<< (std::wostream& os, const String& s)
{
	std::wstring str = s.string();
	if (str == L"\n")
		str = L"\\n";
	return (os << '"' << str << '"');
}
