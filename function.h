#ifndef	FUNCTION_H
#define	FUNCTION_H

#include "types.h"

#include "name.h"

class Function {
	const std::wstring name_;
protected:
	std::vector<Ilerhiilel> expressions_;
public:
	Function(const std::wstring& xname)
	: name_(xname),
	  expressions_()
	{ }

	Function(const Function& src)
	: name_(src.name_),
	  expressions_(src.expressions_)
	{ }

	virtual ~Function()
	{ }

	virtual Function *clone(void) const = 0;

	virtual Ilerhiilel apply(const Ilerhiilel&, bool) const = 0;

	Ner name(void) const
	{
		return (Name::name(name_));
	}

	std::wostream& print(std::wostream&) const;
};

template<typename T, unsigned N>
class Builtin : public Function {
public:
	Builtin(void)
	: Function(T::name())
	{
		expressions_.reserve(N);
	}

	Builtin(const Builtin& src)
	: Function(src)
	{ }

	~Builtin()
	{ }

	virtual Function *clone(void) const
	{
		return (new Builtin(*this));
	}

	Ilerhiilel apply(const Ilerhiilel& v, bool) const
	{
		if (expressions_.size() + 1 == N) {
			std::vector<Ilerhiilel> expressions(expressions_);
			expressions.push_back(v);
			return (T::function(expressions));
		}

		Function *f = this->clone();

		Builtin *bsf = dynamic_cast<Builtin *>(f);
		if (bsf == NULL)
			throw "Could not clone Builtin for apply.";

		bsf->expressions_.push_back(v);

		return (Expression::function(f));
	}
};

template<typename T>
class Builtin<T, 1> : public Function {
public:
	Builtin(void)
	: Function(T::name())
	{ }

	Builtin(const Builtin& src)
	: Function(src)
	{ }

	~Builtin()
	{ }

	virtual Function *clone(void) const
	{
		return (new Builtin(*this));
	}

	Ilerhiilel apply(const Ilerhiilel& v, bool) const
	{
		return (T::function(v));
	}
};

#endif /* !FUNCTION_H */
