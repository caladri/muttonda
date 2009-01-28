#ifndef	SCALAR_H
#define	SCALAR_H

class Scalar {
	unsigned v_;
public:
	Scalar(void);
	Scalar(const unsigned&);
	Scalar(const Scalar&);

	bool operator== (const Scalar&) const;
	Scalar operator* (const Scalar&) const;
	Scalar operator+ (const Scalar&) const;

	unsigned value(void) const;
};

std::ostream& operator<< (std::ostream&, const Scalar&);

#endif /* !SCALAR_H */
