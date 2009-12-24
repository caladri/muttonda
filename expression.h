#ifndef	EXPRESSION_H
#define	EXPRESSION_H

#include "name.h"
#include "scalar.h"
#include "string.h"

class Function;

class Expression {
	friend std::ostream& operator<< (std::ostream&, const Expression&);

	enum Type {
		EVariable,
		EValue,
		EApply,
		EFunction,
		EString,
	};

	Type type_;
	Name name_;
	Scalar scalar_;
	std::vector<Expression> expressions_;
	String str_;
	Function *function_;
public:
	Expression(const Name&);
	Expression(const Scalar&);
	Expression(const Expression&, const Expression&);
	Expression(const Function&);
	Expression(const String&);
	Expression(const Expression&);

	~Expression();

	Expression& operator= (const Expression&);

	void bind(const Name&, const Expression&);

	Expression eval(void) const;
	Expression simplify(void) const;

	Name name(void) const;
	Scalar scalar(void) const;
	String string(void) const;
};

std::ostream& operator<< (std::ostream&, const Expression&);

#endif /* !EXPRESSION_H */
