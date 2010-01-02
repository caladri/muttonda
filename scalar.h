#ifndef	SCALAR_H
#define	SCALAR_H

class Scalar {
	uintmax_t v_;
public:
	Scalar(void);
	Scalar(const uintmax_t&);
	Scalar(const Scalar&);

	bool operator< (const Scalar&) const;
	bool operator== (const Scalar&) const;
	Scalar operator* (const Scalar&) const;
	Scalar operator+ (const Scalar&) const;

	uintmax_t value(void) const;
};

std::wostream& operator<< (std::wostream&, const Scalar&);

#endif /* !SCALAR_H */
