#ifndef	NAME_H
#define	NAME_H

class Name {
	std::string name_;
public:
	Name(void)
	: name_()
	{ }

	Name(const char *str)
	: name_(str)
	{ }

	Name(const std::string& str)
	: name_(str)
	{ }

	Name(const Name& src)
	: name_(src.name_)
	{ }

	bool operator== (const Name& b) const
	{
		return (name_ == b.name_);
	}

	std::string string(void) const
	{
		return (name_);
	}
};

std::ostream& operator<< (std::ostream&, const Name&);

#endif /* !NAME_H */
