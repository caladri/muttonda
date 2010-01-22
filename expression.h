#ifndef	EXPRESSION_H
#define	EXPRESSION_H

#include "ref.h"

#include "name.h"
#include "scalar.h"
#include "string.h"

class Function;

/*
 * XXX
 *
 * Should now add back a ::simplify() method, since it's obvious that it can fold
 * constants down, whereas the handling in ::let() can only fold constant expressions
 * upwards (or outwards.)  It just needs to handle recursing into EApply and
 * evaluating constant ELets (well, ones that can't result in variable capture.)
 */
class Expression {
	friend std::wostream& operator<< (std::wostream&, const Expression&);
	friend class Lambda;
	friend class Ref<Expression>;

	enum Type {
		EVariable,
		EScalar,
		EApply,
		EFunction,
		ELambda,
		ELet,
		EString,
	};

	Type type_;
	Ref<Name> name_;
	Scalar scalar_;
	std::pair<Ref<Expression>, Ref<Expression> > expressions_;
	String str_;
	Ref<Function> function_;
public:
	Expression(const Ref<Function>& function)
	: type_(EFunction),
	  name_(),
	  scalar_(),
	  expressions_(),
	  str_(),
	  function_(function)
	{ }

	Ref<Expression> bind(const Ref<Name>&, const Ref<Expression>&) const;
	Ref<Expression> eval(bool) const;

	Ref<Name> name(void) const;
	Scalar scalar(void) const;
	String string(void) const;

	static Ref<Expression> apply(const Ref<Expression>&, const Ref<Expression>&);
	static Ref<Expression> lambda(const Ref<Name>&, const Ref<Expression>&);
	static Ref<Expression> let(const Ref<Name>&, const Ref<Expression>&, const Ref<Expression>&);
	static Ref<Expression> name(const Ref<Name>&);
	static Ref<Expression> scalar(const Scalar&);
	static Ref<Expression> string(const String&);

private:
	Expression(const Ref<Name>& name)
	: type_(EVariable),
	  name_(name),
	  scalar_(),
	  expressions_(),
	  str_(),
	  function_()
	{ }

	Expression(const Scalar& s)
	: type_(EScalar),
	  name_(),
	  scalar_(s),
	  expressions_(),
	  str_(),
	  function_()
	{ }

	Expression(const Ref<Expression>& a, const Ref<Expression>& b)
	: type_(EApply),
	  name_(),
	  scalar_(),
	  expressions_(a, b),
	  str_(),
	  function_()
	{ }

	Expression(const Ref<Name>& name, const Ref<Expression>& expr)
	: type_(ELambda),
	  name_(name),
	  scalar_(),
	  expressions_(),
	  str_(),
	  function_()
	{
		expressions_.first = expr;
	}

	Expression(const Ref<Name>& name, const Ref<Expression>& a, const Ref<Expression>& b)
	: type_(ELet),
	  name_(name),
	  scalar_(),
	  expressions_(a, b),
	  str_(),
	  function_()
	{ }

	Expression(const String& str)
	: type_(EString),
	  name_(),
	  scalar_(),
	  expressions_(),
	  str_(str),
	  function_()
	{ }

	~Expression()
	{ }

	Expression(const Expression&);
	const Expression& operator= (const Expression&);
};

std::wostream& operator<< (std::wostream&, const Ref<Expression>&);
std::wostream& operator<< (std::wostream&, const Expression&);

#endif /* !EXPRESSION_H */
