#ifndef	SCALAR_H
#define	SCALAR_H

class Scalar {
	uintmax_t v_;
public:
	Scalar(void)
	: v_()
	{ }

	Scalar(const uintmax_t& v)
	: v_(v)
	{ }

	Scalar(const Scalar& src)
	: v_(src.v_)
	{ }

	bool operator< (const Scalar&) const;
	bool operator== (const Scalar&) const;
	Scalar operator* (const Scalar&) const;
	Scalar operator+ (const Scalar&) const;

	uintmax_t value(void) const
	{
		return (v_);
	}
};

std::wostream& operator<< (std::wostream&, const Scalar&);

#endif /* !SCALAR_H */
