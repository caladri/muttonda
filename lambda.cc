#include <ostream>
#include <string>
#include <vector>

#include "expression.h"
#include "function.h"
#include "lambda.h"
#include "name.h"

Lambda::Lambda(const Name& name, const Ref<Expression>& expr)
: Function(),
  name_(name),
  expr_(expr)
{ }

Lambda::Lambda(const Lambda& src)
: Function(src),
  name_(src.name_),
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
	/* Do not rename if the name is shadowed.  */
	if (name_ == name)
		return (Ref<Expression>());

	Ref<Expression> expr(expr_->bind(name, e));
	if (expr.null())
		return (Ref<Expression>());
	return (Expression::lambda(name_, expr));
}

/*
 * XXX
 * If the name is shadowed we should just avoid the
 * call to bind entirely.
 */
Ref<Expression>
Lambda::apply(const Ref<Expression>& v, bool memoize) const
{
	Ref<Expression> expr(expr_->bind(name_, v));
	if (expr.null())
		expr = expr_;
	Ref<Expression> evaluated(expr->eval(memoize));
	if (evaluated.null())
		return (expr);
	return (evaluated);
}

/*
 * XXX
 * If the name is shadowed we should just avoid the
 * call to bind entirely.
 */
Ref<Expression>
Lambda::fold(const Ref<Expression>& v) const
{
	Ref<Expression> expr(expr_->bind(name_, v));
	if (expr.null())
		return (expr_->simplify());
	Ref<Expression> simplified(expr->simplify());
	if (simplified.null())
		return (expr);
	return (simplified);
}

/*
 * TODO:
 * If our expression is an Apply and the right side of the
 * apply is name_ and name_ is not used in the left side of
 * the apply (which can be determined using bind()) then we
 * can simplify ourselves to being just the left side of the
 * apply.
 */
Ref<Expression>
Lambda::simplify(void) const
{
	Ref<Expression> expr(expr_->simplify());

	if (expr.null())
		return (Ref<Expression>());
	return (Expression::lambda(name_, expr));
}

std::wostream&
Lambda::print(std::wostream& os) const
{
	os << "\\" << name_;

	Ref<Expression> expr(expr_);
	for (;;) {
		if (expr->type_ != Expression::EFunction) {
			break;
		}
		Lambda *nested = dynamic_cast<Lambda *>(expr->function_);
		if (nested == NULL) {
			break;
		}
		os << " " << nested->name_;
		expr = nested->expr_;
	}

	os << " -> " << expr;
	return (os);
}
