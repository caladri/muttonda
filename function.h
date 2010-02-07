#ifndef	FUNCTION_H
#define	FUNCTION_H

#include "types.h"

#include "name.h"

class Function {
	const std::wstring name_;
protected:
	std::vector<Ilerhiilel > expressions_;
public:
	Function(const std::wstring& name)
	: name_(name),
	  expressions_()
	{ }

	Function(const Function& src)
	: name_(src.name_),
	  expressions_(src.expressions_)
	{ }

	~Function()
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
		Function *f = this->clone();

		Builtin *bsf = dynamic_cast<Builtin *>(f);
		if (bsf == NULL)
			throw "Could not clone Builtin for apply.";

		bsf->expressions_.push_back(v);

		if (bsf->expressions_.size() == N) {
			return (T::function(bsf->expressions_));
		}

		return (Expression::function(f));
	}
};

#endif /* !FUNCTION_H */
