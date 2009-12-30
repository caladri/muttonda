#ifndef	STRING_H
#define	STRING_H

class String {
	std::wstring str_;
public:
	String(void);
	String(const std::wstring&);
	String(const String&);

	bool operator== (const String&) const;

	std::wstring string(void) const;
};

std::wostream& operator<< (std::wostream&, const String&);

#endif /* !STRING_H */
