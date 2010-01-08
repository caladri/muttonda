#ifndef	LAMBDA_H
#define	LAMBDA_H

class Lambda : public Function {
	friend class Expression;

	Name name_;
	Ref<Expression> expr_;
public:
	Lambda(const Name&, const Ref<Expression>&);
	Lambda(const Lambda&);

	~Lambda();

	Function *clone(void) const;

	Ref<Expression> bind(const Name&, const Ref<Expression>&) const;
	Ref<Expression> apply(const Ref<Expression>&, bool) const;
	Ref<Expression> fold(const Ref<Expression>&) const;
	Ref<Expression> simplify(void) const;

	std::wostream& print(std::wostream&) const;
};

#endif /* !LAMBDA_H */
