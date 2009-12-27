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
	virtual Ref<Expression> apply(const Ref<Expression>&) const = 0;
	virtual Ref<Expression> fold(const Ref<Expression>&) const
	{
		return (Ref<Expression>());
	}
	virtual Ref<Expression> simplify(void) const
	{
		return (Ref<Expression>());
	}
	virtual std::ostream& print(std::ostream&) const = 0;
};

class SimpleFunction : public Function {
	const std::string name_;
protected:
	std::vector<Ref<Expression> > expressions_;
public:
	SimpleFunction(const std::string&);
	SimpleFunction(const SimpleFunction&);

	~SimpleFunction();

	Ref<Expression> bind(const Name&, const Ref<Expression>&) const;

	std::string name(void) const;

	std::ostream& print(std::ostream&) const;
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

	Ref<Expression> apply(const Ref<Expression>& v) const
	{
		Builtin bsf(*this);

		bsf.expressions_.push_back(v);

		if (bsf.expressions_.size() == N) {
			return (T::function(bsf.expressions_));
		}

		return (new Expression(bsf));
	}
};

#endif /* !FUNCTION_H */
