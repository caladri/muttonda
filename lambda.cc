#include <ostream>
#include <string>
#include <vector>

#include "expression.h"
#include "function.h"
#include "lambda.h"
#include "name.h"

Lambda::Lambda(const Name& name, const Expression& expr)
: Function(),
  names_(),
  expr_(expr)
{
	names_.push_back(name);
}

Lambda::Lambda(const std::vector<Name>& names, const Expression& expr)
: Function(),
  names_(names),
  expr_(expr)
{
	if (names.empty())
		throw "Empty vector of lambda variable names.";
}

Lambda::Lambda(const Lambda& src)
: Function(src),
  names_(src.names_),
  expr_(src.expr_)
{ }

Lambda::~Lambda()
{ }

Function *
Lambda::clone(void) const
{
	return (new Lambda(*this));
}

void
Lambda::bind(const Name& name, const Expression& expr)
{
	std::vector<Name>::const_iterator it;

	/* Do not rename if the name is shadowed.  */
	for (it = names_.begin(); it != names_.end(); ++it)
		if (*it == name)
			return;
	expr_.bind(name, expr);
}

/*
 * XXX
 * If the name is shadowed we should just avoid the
 * call to bind entirely.
 */
Expression
Lambda::apply(const Expression& v) const
{
	std::vector<Name> names(names_.begin() + 1, names_.end());

	Expression expr(expr_);
	if (names.empty()) {
		expr.bind(names_.front(), v);
		return (expr.eval());
	}

	expr = Lambda(names, expr);
	expr.bind(names_.front(), v);
	return (expr);
}

/*
 * XXX
 * If the name is shadowed we should just avoid the
 * call to bind entirely.
 */
Expression
Lambda::fold(bool bound, const Expression& v) const
{
	if (!bound)
		return (Expression(*this, v));

	std::vector<Name> names(names_.begin() + 1, names_.end());

	Expression expr(expr_);
	if (names.empty()) {
		expr.bind(names_.front(), v);
		return (expr);
	}

	expr = Lambda(names, expr);
	expr.bind(names_.front(), v);
	return (expr);
}

std::ostream&
Lambda::print(std::ostream& os) const
{
	std::vector<Name>::const_iterator it;

	os << "\\";
	for (it = names_.begin(); it != names_.end(); ++it)
		os << *it << " ";
	os << "-> " << expr_;

	return (os);
}
