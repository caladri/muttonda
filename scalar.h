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

	bool operator< (const Scalar& b) const
	{
		return (v_ < b.v_);
	}

	bool operator== (const Scalar& b) const
	{
		return (v_ == b.v_);
	}

	Scalar operator* (const Scalar& b) const
	{
		return (v_ * b.v_);
	}

	Scalar operator+ (const Scalar& b) const
	{
		return (v_ + b.v_);
	}

	uintmax_t value(void) const
	{
		return (v_);
	}
};

namespace std {
	namespace tr1 {
		template<>
		struct hash<Scalar> {
			size_t operator() (const Scalar& scalar) const
			{
				return (hash<uintmax_t>()(scalar.value()));
			}
		};
	};
};

std::wostream& operator<< (std::wostream&, const Scalar&);

#endif /* !SCALAR_H */
