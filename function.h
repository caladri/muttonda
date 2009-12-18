#ifndef	FUNCTION_H
#define	FUNCTION_H

class Function {
public:
	Function(void)
	{ }

	virtual ~Function()
	{ }

	virtual Function *clone(void) const = 0;
	virtual void bind(const Name&, const Expression&) = 0;
	virtual Expression apply(const Expression&) const = 0;
	virtual Expression fold(bool, const Expression& expr) const
	{
		return (Expression(*this, expr));
	}
	virtual std::ostream& print(std::ostream&) const = 0;
};

class SimpleFunction : public Function {
	const std::string name_;
protected:
	std::vector<Expression> expressions_;
public:
	SimpleFunction(const std::string&);
	SimpleFunction(const SimpleFunction&);

	~SimpleFunction();

	void bind(const Name&, const Expression&);

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

	Expression apply(const Expression& v) const
	{
		Builtin bsf(*this);

		bsf.expressions_.push_back(v);

		if (bsf.expressions_.size() == N) {
			return (T::function(bsf.expressions_));
		}

		return (Expression(bsf));
	}
};

#endif /* !FUNCTION_H */
