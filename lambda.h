#ifndef	LAMBDA_H
#define	LAMBDA_H

class Lambda : public Function {
	friend class Expression;

	std::vector<Name> names_;
	Ref<Expression> expr_;
public:
	Lambda(const Name&, const Ref<Expression>&);
	Lambda(const std::vector<Name>&, const Ref<Expression>&);
	Lambda(const Lambda&);

	~Lambda();

	Function *clone(void) const;

	Ref<Expression> bind(const Name&, const Ref<Expression>&) const;
	Ref<Expression> apply(const Ref<Expression>&) const;
	Ref<Expression> fold(const Ref<Expression>&) const;
	Ref<Expression> simplify(const Ref<Expression>&) const;

	std::ostream& print(std::ostream&) const;
};

#endif /* !LAMBDA_H */
