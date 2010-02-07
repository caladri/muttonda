#ifndef	NAME_H
#define	NAME_H

#include "types.h"

class Name;

typedef Ref<Name> Ner;

class Name {
	friend class Ref<Name>;

	std::wstring name_;

	Name(const std::wstring& str)
	: name_(str)
	{ }

	~Name()
	{ }

public:
	std::wstring string(void) const
	{
		return (name_);
	}

	static Ner name(const wchar_t *str)
	{
		return (name(std::wstring(str)));
	}

	static Ner name(const std::wstring& str)
	{
		static std::tr1::unordered_map<std::wstring, Ner > name_cache;
		std::tr1::unordered_map<std::wstring, Ner >::const_iterator it;

		it = name_cache.find(str);
		if (it != name_cache.end())
			return (it->second);

		Ner name(new Name(str));

		name_cache[str] = name;

		return (name);
	}
};

std::wostream& operator<< (std::wostream&, const Ner&);

#endif /* !NAME_H */
