#ifndef	NAME_H
#define	NAME_H

#include "ref.h"

class Name {
	std::wstring name_;

	Name(const std::wstring& str)
	: name_(str)
	{ }

public:
	std::wstring string(void) const
	{
		return (name_);
	}

	static Ref<Name> name(const wchar_t *str)
	{
		return (name(std::wstring(str)));
	}

	static Ref<Name> name(const std::wstring& str)
	{
		static std::map<std::wstring, Ref<Name> > cache;
		std::map<std::wstring, Ref<Name> >::const_iterator it;

		it = cache.find(str);
		if (it != cache.end())
			return (it->second);

		Ref<Name> name(new Name(str));

		cache[str] = name;

		return (name);
	}
};

std::wostream& operator<< (std::wostream&, const Ref<Name>&);

#endif /* !NAME_H */
