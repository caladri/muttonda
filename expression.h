#ifndef	EXPRESSION_H
#define	EXPRESSION_H

#include "ref.h"

#include "name.h"
#include "scalar.h"
#include "string.h"

class Function;

class Expression {
	friend std::ostream& operator<< (std::ostream&, const Expression&);
	friend class Lambda;

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
	std::vector<Ref<Expression> > expressions_;
	String str_;
	Function *function_;
public:
	Expression(const Name&);
	Expression(const Scalar&);
	Expression(const Ref<Expression>&, const Ref<Expression>&);
	Expression(const Function&);
	Expression(const String&);
	Expression(const Ref<Expression>&);

	~Expression();

	static Ref<Expression> bind(const Ref<Expression>&, const Name&, const Ref<Expression>&);

	static Ref<Expression> eval(const Ref<Expression>&);
	static Ref<Expression> simplify(const Ref<Expression>&);

	Name name(void) const;
	Scalar scalar(void) const;
	String string(void) const;

private:
	Expression(const Expression&);
	const Expression& operator= (const Expression&);
};

std::ostream& operator<< (std::ostream&, const Expression&);

#endif /* !EXPRESSION_H */
