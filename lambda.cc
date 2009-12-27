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
Lambda::bind(const Name& name, const Ref<Expression>& e) const
{
	std::vector<Name>::const_iterator it;

	/* Do not rename if the name is shadowed.  */
	for (it = names_.begin(); it != names_.end(); ++it)
		if (*it == name)
			return (Ref<Expression>());

	Ref<Expression> expr(expr_->bind(name, e));
	if (expr.null())
		return (Ref<Expression>());
	return (new Expression(Lambda(names_, expr)));
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
		Ref<Expression> expr(expr_->bind(names_.front(), v));
		if (expr.null())
			expr = expr_;
		Ref<Expression> evaluated(expr->eval());
		if (evaluated.null())
			return (expr);
		return (evaluated);
	}

	Lambda l(names, expr_);
	Ref<Expression> expr(l.bind(names_.front(), v));
	if (expr.null())
		return (new Expression(l));
	return (expr);
}

/*
 * XXX
 * If the name is shadowed we should just avoid the
 * call to bind entirely.
 */
Ref<Expression>
Lambda::fold(const Ref<Expression>& v) const
{
	std::vector<Name> names(names_.begin() + 1, names_.end());

	if (names.empty()) {
		Ref<Expression> expr(expr_->bind(names_.front(), v));
		if (expr.null())
			expr = expr_;
		return (expr);
	}

	Lambda l(names, expr_);
	Ref<Expression> expr(l.bind(names_.front(), v));
	if (expr.null())
		return (new Expression(l));
	return (expr);
}

Ref<Expression>
Lambda::simplify(void) const
{
	Ref<Expression> expr(expr_->simplify());
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
	os << "-> " << expr_;

	return (os);
}
