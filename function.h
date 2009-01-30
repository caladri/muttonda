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

template<typename T>
class Builtin : public SimpleFunction {
	unsigned n_;
public:
	Builtin(unsigned n)
	: SimpleFunction(T::name()),
	  n_(n)
	{ }

	Builtin(const Builtin& src)
	: SimpleFunction(src),
	  n_(src.n_)
	{ }

	~Builtin()
	{ }

	Function *clone(void) const
	{
		return (new Builtin(*this));
	}

	Expression apply(const Expression& v) const
	{
		Builtin bsf(*this);

		bsf.expressions_.push_back(v);

		if (bsf.expressions_.size() == n_) {
			return (T::function(bsf.expressions_));
		}

		return (Expression(bsf));
	}
};

#endif /* !FUNCTION_H */
