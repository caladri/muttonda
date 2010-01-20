#include <ostream>
#include <string>

#include <tr1/unordered_map>

#include "string.h"

std::wostream&
operator<< (std::wostream& os, const String& s)
{
	std::wstring str = s.string();
	std::wstring::const_iterator it;

	os << "\"";
	for (it = str.begin(); it != str.end(); ++it) {
		switch (*it) {
		case L'\n':
			os << L"\\n";
			break;
		case L'"':
		case L'\\':
			os << L"\\";
		default:
			os << *it;
			break;
		}
	}
	os << "\"";

	return (os);
}
