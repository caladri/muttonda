#include <ostream>
#include <string>

#include "string.h"

String::String(void)
: str_("")
{ }

String::String(const std::string& str)
: str_(str)
{ }

String::String(const String& src)
: str_(src.str_)
{ }

std::string
String::string(void) const
{
	return (str_);
}

bool
String::operator== (const String& b) const
{
	return (str_ == b.str_);
}

std::ostream&
operator<< (std::ostream& os, const String& s)
{
	std::string str = s.string();
	if (str == "\n")
		str = "\\n";
	return (os << '"' << str << '"');
}
