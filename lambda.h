#ifndef	LAMBDA_H
#define	LAMBDA_H

class Lambda : public Function {
	Name v_;
	Expression e_;
public:
	Lambda(const Name&, const Expression&);
	Lambda(const Lambda&);

	~Lambda();

	Function *clone(void) const;

	void bind(const Name&, const Expression&);

	Expression apply(const Expression&) const;

	std::ostream& print(std::ostream&) const;
};

#endif /* !LAMBDA_H */
