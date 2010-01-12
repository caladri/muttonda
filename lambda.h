#ifndef	LAMBDA_H
#define	LAMBDA_H

class Lambda : public Function {
	friend class Expression;

	Name name_;
	Ref<Expression> expr_;
public:
	Lambda(const Name& name, const Ref<Expression>& expr)
	: Function(),
	  name_(name),
	  expr_(expr)
	{ }

	Lambda(const Lambda& src)
	: Function(src),
	  name_(src.name_),
	  expr_(src.expr_)
	{ }

	~Lambda()
	{ }

	Function *clone(void) const
	{
		return (new Lambda(*this));
	}

	Ref<Expression> bind(const Name&, const Ref<Expression>&) const;
	Ref<Expression> apply(const Ref<Expression>&, bool) const;
	Ref<Expression> fold(const Ref<Expression>&, bool) const;
	Ref<Expression> simplify(void) const;

	std::wostream& print(std::wostream&) const;
};

#endif /* !LAMBDA_H */
