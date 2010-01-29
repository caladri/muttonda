#ifndef	EXPRESSION_H
#define	EXPRESSION_H

#include "ref.h"

#include "name.h"
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
	uintmax_t number_;
	std::pair<Ref<Expression>, Ref<Expression> > expressions_;
	String string_;
	Ref<Function> function_;
	std::set<Ref<Name> > free_;

public:
	Expression(const Ref<Function>& function)
	: type_(EFunction),
	  name_(),
	  number_(),
	  expressions_(),
	  string_(),
	  function_(function),
	  free_()
	{ }

	Ref<Expression> bind(const Ref<Name>&, const Ref<Expression>&) const;
	Ref<Expression> eval(bool) const;

	Ref<Name> name(void) const;
	uintmax_t scalar(void) const;
	String string(void) const;

	static Ref<Expression> apply(const Ref<Expression>&, const Ref<Expression>&);
	static Ref<Expression> lambda(const Ref<Name>&, const Ref<Expression>&);
	static Ref<Expression> let(const Ref<Name>&, const Ref<Expression>&, const Ref<Expression>&);
	static Ref<Expression> name(const Ref<Name>&);
	static Ref<Expression> scalar(const uintmax_t&, const Ref<Expression>& = Ref<Expression>(), const Ref<Expression>& = Ref<Expression>());
	static Ref<Expression> string(const String&);

private:
	Expression(const Ref<Name>& name)
	: type_(EVariable),
	  name_(name),
	  number_(),
	  expressions_(),
	  string_(),
	  function_(),
	  free_()
	{
		free_.insert(name);
	}

	Expression(const uintmax_t& number, const Ref<Expression>& f, const Ref<Expression>& x)
	: type_(EScalar),
	  name_(),
	  number_(number),
	  expressions_(f, x),
	  string_(),
	  function_(),
	  free_()
	{
		if (!f.null()) {
			if (!x.null()) {
				std::merge(f->free_.begin(), f->free_.end(),
					   x->free_.begin(), x->free_.end(),
					   std::inserter(free_, free_.begin()));
			} else {
				free_ = x->free_;
			}
		}
	}

	Expression(const Ref<Expression>& a, const Ref<Expression>& b)
	: type_(EApply),
	  name_(),
	  number_(),
	  expressions_(a, b),
	  string_(),
	  function_(),
	  free_()
	{
		std::merge(a->free_.begin(), a->free_.end(),
			   b->free_.begin(), b->free_.end(),
			   std::inserter(free_, free_.begin()));
	}

	Expression(const Ref<Name>& name, const Ref<Expression>& expr)
	: type_(ELambda),
	  name_(name),
	  number_(),
	  expressions_(),
	  string_(),
	  function_(),
	  free_(expr->free_)
	{
		expressions_.first = expr;

		free_.erase(name);
	}

	Expression(const Ref<Name>& name, const Ref<Expression>& a, const Ref<Expression>& b)
	: type_(ELet),
	  name_(name),
	  number_(),
	  expressions_(a, b),
	  string_(),
	  function_(),
	  free_()
	{
		std::merge(a->free_.begin(), a->free_.end(),
			   b->free_.begin(), b->free_.end(),
			   std::inserter(free_, free_.begin()));

		if (a->free_.find(name) == a->free_.end()) {
			free_.erase(name);
		}
	}

	Expression(const String& string)
	: type_(EString),
	  name_(),
	  number_(),
	  expressions_(),
	  string_(string),
	  function_(),
	  free_()
	{ }

	~Expression()
	{ }

	Expression(const Expression&);
	const Expression& operator= (const Expression&);
};

std::wostream& operator<< (std::wostream&, const Ref<Expression>&);
std::wostream& operator<< (std::wostream&, const Expression&);

#endif /* !EXPRESSION_H */
