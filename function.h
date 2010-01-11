#ifndef	FUNCTION_H
#define	FUNCTION_H

class Function {
public:
	Function(void)
	{ }

	virtual ~Function()
	{ }

	virtual Function *clone(void) const = 0;
	virtual Ref<Expression> bind(const Name&, const Ref<Expression>&) const = 0;
	virtual Ref<Expression> apply(const Ref<Expression>&, bool) const = 0;
	virtual Ref<Expression> fold(const Ref<Expression>&) const
	{
		return (Ref<Expression>());
	}
	virtual Ref<Expression> simplify(void) const
	{
		return (Ref<Expression>());
	}
	virtual std::wostream& print(std::wostream&) const = 0;
};

class SimpleFunction : public Function {
	const std::wstring name_;
protected:
	std::vector<Ref<Expression> > expressions_;
public:
	SimpleFunction(const std::wstring& name)
	: Function(),
	  name_(name),
	  expressions_()
	{ }

	SimpleFunction(const SimpleFunction& src)
	: Function(src),
	  name_(src.name_),
	  expressions_(src.expressions_)
	{ }

	~SimpleFunction()
	{ }

	Ref<Expression> bind(const Name&, const Ref<Expression>&) const;

	std::wstring name(void) const
	{
		return (name_);
	}

	std::wostream& print(std::wostream&) const;
};

template<typename T, unsigned N>
class Builtin : public SimpleFunction {
public:
	Builtin(void)
	: SimpleFunction(T::name())
	{ }

	Builtin(const Builtin& src)
	: SimpleFunction(src)
	{ }

	~Builtin()
	{ }

	virtual Function *clone(void) const
	{
		return (new Builtin(*this));
	}

	Ref<Expression> apply(const Ref<Expression>& v, bool) const
	{
		Function *f = this->clone();

		Builtin *bsf = dynamic_cast<Builtin *>(f);
		if (bsf == NULL)
			throw "Could not clone Builtin for apply.";

		bsf->expressions_.push_back(v);

		if (bsf->expressions_.size() == N) {
			return (T::function(bsf->expressions_));
		}

		return (new Expression(f));
	}
};

#endif /* !FUNCTION_H */
