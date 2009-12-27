#include <ostream>
#include <string>
#include <vector>

#include "expression.h"
#include "function.h"
#include "lambda.h"
#include "name.h"

Lambda::Lambda(const Name& name, const Ref<Expression>& expr)
: Function(),
  names_(),
  expr_(expr)
{
	names_.push_back(name);
}

Lambda::Lambda(const std::vector<Name>& names, const Ref<Expression>& expr)
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

Ref<Expression>
Lambda::bind(const Name& name, const Ref<Expression>& expr)
{
	std::vector<Name>::const_iterator it;

	/* Do not rename if the name is shadowed.  */
	for (it = names_.begin(); it != names_.end(); ++it)
		if (*it == name)
			return (new Expression(*this)); /* XXX self */
	return (new Expression(Lambda(names_, Expression::bind(expr_, name, expr))));
}

/*
 * XXX
 * If the name is shadowed we should just avoid the
 * call to bind entirely.
 */
Ref<Expression>
Lambda::apply(const Ref<Expression>& v) const
{
	std::vector<Name> names(names_.begin() + 1, names_.end());

	if (names.empty()) {
		/* XXX eval?  */
		return (Expression::eval(Expression::bind(expr_, names_.front(), v)));
	}

	return (Lambda(names, expr_).bind(names_.front(), v));
}

#if 0
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
#endif

Ref<Expression>
Lambda::simplify(const Ref<Expression>& self) const
{
	Ref<Expression> expr(Expression::simplify(expr_));
	bool simplified = !expr.null();

	if (!simplified)
		expr = expr_;

	switch (expr->type_) {
	case Expression::EFunction:
		break;
	default:
		if (!simplified)
			return (Ref<Expression>());
		return (new Expression(Lambda(names_, expr)));
	}

	Lambda *nested = dynamic_cast<Lambda *>(expr->function_);
	if (nested == NULL) {
		if (!simplified)
			return (Ref<Expression>());
		return (new Expression(Lambda(names_, expr)));
	}

	std::vector<Name> names(names_);
	names.insert(names.end(), nested->names_.begin(), nested->names_.end());
	return (new Expression(Lambda(names, nested->expr_)));
}

std::ostream&
Lambda::print(std::ostream& os) const
{
	std::vector<Name>::const_iterator it;

	os << "\\";
	for (it = names_.begin(); it != names_.end(); ++it)
		os << *it << " ";
	os << "-> " << *expr_;

	return (os);
}
