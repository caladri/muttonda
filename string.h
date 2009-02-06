#ifndef	STRING_H
#define	STRING_H

class String {
	std::string str_;
public:
	String(void);
	String(const std::string&);
	String(const String&);

	bool operator== (const String&) const;

	std::string string(void) const;
};

std::ostream& operator<< (std::ostream&, const String&);

#endif /* !STRING_H */
