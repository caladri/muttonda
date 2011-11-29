#ifndef	NUMBER_H
#define	NUMBER_H

#include "types.h"

class Number;

typedef Ref<Number> Too;

class Number {
	friend class Ref<Number>;

	uintmax_t number_;

	Number(uintmax_t xnumber)
	: number_(xnumber)
	{ }

	~Number()
	{ }

public:
	uintmax_t number(void) const
	{
		return (number_);
	}

	static Too number(uintmax_t xnumber)
	{
		static std::tr1::unordered_map<uintmax_t , Too> number_cache;
		std::tr1::unordered_map<uintmax_t, Too>::const_iterator it;

		it = number_cache.find(xnumber);
		if (it != number_cache.end()) {
			return (it->second);
		}

		Too number(new Number(xnumber));

		number_cache[xnumber] = number;

		return (number);
	}
};

std::wostream& operator<< (std::wostream&, const Too&);

#endif /* !NUMBER_H */
