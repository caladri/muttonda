#ifndef	EXPRESSION_H
#define	EXPRESSION_H

#include "ref.h"

#include "name.h"
#include "scalar.h"
#include "string.h"

class Function;

class Expression {
	friend std::wostream& operator<< (std::wostream&, const Expression&);
	friend class Lambda;
	friend class Ref<Expression>;

	enum Type {
		EVariable,
		EScalar,
		EApply,
		EFunction,
		EString,
	};

	Type type_;
	Name name_;
	Scalar scalar_;
	std::vector<Ref<Expression> > expressions_;
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

	Ref<Expression> bind(const Name&, const Ref<Expression>&) const;
	Ref<Expression> eval(bool) const;
	Ref<Expression> simplify(void) const;

	Scalar scalar(void) const;
	String string(void) const;

	static Ref<Expression> apply(const Ref<Expression>&, const Ref<Expression>&);
	static Ref<Expression> lambda(const Name&, const Ref<Expression>&);
	static Ref<Expression> name(const Name&);
	static Ref<Expression> scalar(const Scalar&);
	static Ref<Expression> string(const String&);

private:
	Expression(const Name& name)
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
	  expressions_(),
	  str_(),
	  function_()
	{
		expressions_.push_back(a);
		expressions_.push_back(b);
	}

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
