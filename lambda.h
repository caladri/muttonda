#ifndef	LAMBDA_H
#define	LAMBDA_H

class Lambda : public Function {
	friend class Expression;

	std::vector<Name> names_;
	Expression expr_;
public:
	Lambda(const Name&, const Expression&);
	Lambda(const std::vector<Name>&, const Expression&);
	Lambda(const Lambda&);

	~Lambda();

	Function *clone(void) const;

	void bind(const Name&, const Expression&);

	Expression apply(const Expression&) const;
	Expression fold(bool, const Expression&) const;

	std::ostream& print(std::ostream&) const;
};

#endif /* !LAMBDA_H */
