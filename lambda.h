#ifndef	LAMBDA_H
#define	LAMBDA_H

class Lambda : public Function {
	Name v_;
	Expression e_;
public:
	Lambda(const Name& v, const Expression& e);
	Lambda(const Lambda& src);

	~Lambda();

	Function *clone(void) const;

	void bind(const Name& v, const Expression& e);

	Expression apply(const Expression& v) const;

	std::ostream& print(std::ostream&) const;
};

#endif /* !LAMBDA_H */
