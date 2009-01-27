#ifndef	EXPRESSION_H
#define	EXPRESSION_H

#include "name.h"
#include "scalar.h"

class Function;

class Expression {
	friend std::ostream& operator<< (std::ostream&, const Expression&);

	enum Type {
		EVariable,
		EValue,
		EApply,
		EFunction,
	};

	Type type_;
	Name name_;
	Scalar scalar_;
	std::vector<Expression> expressions_;
	Function *function_;
public:
	Expression(const Name&);
	Expression(const Scalar&);
	Expression(const Expression&, const Expression&);
	Expression(const Function&);
	Expression(const Expression&);

	~Expression();

	Expression& operator= (const Expression&);
	Expression operator() (const Expression&) const;

	void bind(const Name&, const Expression&);

	Expression eval(void) const;

	Name name(void) const;
	Scalar scalar(void) const;
};

std::ostream& operator<< (std::ostream&, const Expression&);

#endif /* !EXPRESSION_H */
