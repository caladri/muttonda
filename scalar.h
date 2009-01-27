#ifndef	SCALAR_H
#define	SCALAR_H

class Scalar {
	float v_;
public:
	Scalar(void);
	Scalar(const float& v);
	Scalar(const Scalar& src);

	bool operator== (const Scalar& b) const;
	Scalar operator* (const Scalar& b) const;
	Scalar operator+ (const Scalar& b) const;

	float value(void) const;
};

std::ostream& operator<< (std::ostream&, const Scalar&);

#endif /* !SCALAR_H */
