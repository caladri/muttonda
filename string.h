#ifndef	STRING_H
#define	STRING_H


class String {
	std::wstring str_;
public:
	String(void)
	: str_()
	{ }

	String(const wchar_t *str)
	: str_(str)
	{ }

	String(const std::wstring& str)
	: str_(str)
	{ }

	String(const String& src)
	: str_(src.str_)
	{ }

	bool operator== (const String& b) const
	{
		return (str_ == b.str_);
	}

	bool operator< (const String& b) const
	{
		return (str_ < b.str_);
	}

	std::wstring string(void) const
	{
		return (str_);
	}
};

std::wostream& operator<< (std::wostream&, const String&);

#endif /* !STRING_H */
