#include <iostream>
#include <ostream>
#include <vector>

#include "expression.h"
#include "function.h"
#include "lambda.h"
#include "name.h"
#include "scalar.h"

/*
 * Do a variable naming pass at some point so we can't
 * leak namespace into lambdas by application.
 */

/*
 * It is possible to track whether an Expression has
 * been simplified, but where that value changes due
 * to binding and a need for resimplification, it can
 * be lost in a copy or a conversion from an EApply to
 * an EApply by hand.
 */

Expression::Expression(const Name& v)
: type_(EVariable),
  name_(v),
  scalar_(),
  expressions_(),
  str_(),
  function_(NULL)
{ }

Expression::Expression(const Scalar& v)
: type_(EValue),
  name_(),
  scalar_(v),
  expressions_(),
  str_(),
  function_(NULL)
{ }

Expression::Expression(const Ref<Expression>& a, const Ref<Expression>& b)
: type_(EApply),
  name_(),
  scalar_(),
  expressions_(),
  str_(),
  function_(NULL)
{
	expressions_.push_back(a);
	expressions_.push_back(b);
}

Expression::Expression(const String& str)
: type_(EString),
  name_(),
  scalar_(),
  expressions_(),
  str_(str),
  function_(NULL)
{ }

Expression::Expression(const Function& f)
: type_(EFunction),
  name_(),
  scalar_(),
  expressions_(),
  str_(),
  function_(f.clone())
{ }

Expression::~Expression()
{
	if (function_ != NULL) {
		delete function_;
		function_ = NULL;
	}
}

Ref<Expression>
Expression::bind(const Ref<Expression>& self, const Name& v, const Ref<Expression>& e)
{
	switch (self->type_) {
	case EVariable:
		if (self->name_ == v)
			return (e);
		return (self);
	case EValue:
	case EString:
		return (self);
	case EApply:
		return (new Expression(bind(self->expressions_[0], v, e),
				       bind(self->expressions_[1], v, e)));
	case EFunction:
		return (self->function_->bind(v, e));
	default:
		throw "Invalid type. (bind)";
	}
}

Ref<Expression>
Expression::eval(const Ref<Expression>& self)
{
	try {
		switch (self->type_) {
		case EVariable:
			throw "Unbound variable.";
		case EFunction:
		case EValue:
		case EString:
			return (self);
		case EApply: {
			Ref<Expression> expr(eval(self->expressions_[0]));

			switch (expr->type_) {
			case EVariable:
				throw "Attempting to apply to free variable.";
			case EValue:
				throw "Attempting to apply to scalar.";
			case EApply:
				return (new Expression(expr, self->expressions_[1]));
			case EFunction:
				return (expr->function_->apply(self->expressions_[1]));
			case EString:
				throw "Attempting to apply to string.";
			default:
				throw "Invalid type. (apply)";
			}
			break;
		}
		default:
			throw "Invalid type. (eval)";
		}
	} catch(...) {
		std::cerr << "From: " << *self << std::endl;
		throw;
	}
}

/*
 * This must not error out ever as it is called when there are still unbound
 * variables, and so we may be partially evaluating something catastrophic
 * which is not intended to work, or which must not be called until needed
 * (like an error routine in the false branch of a conditional.)
 *
 * Would be nice to detect unused variables and dead parameters and to not
 * waste any time on them (i.e. mark them dead, don't simplify, don't
 * evaluate, throw error if they are coerced.)
 */
Ref<Expression>
Expression::simplify(const Ref<Expression>& self)
{
	switch (self->type_) {
	case EApply: {
		Ref<Expression> a(self->expressions_[0]);
		Ref<Expression> b(self->expressions_[1]);

		a = simplify(a);
		b = simplify(b);

		if (a.null() && b.null())
			return (Ref<Expression>());

		if (a.null())
			a = self->expressions_[0];

		if (b.null())
			b = self->expressions_[1];

		/* XXX Folding is temporarily broken.  */
#if 0
		if (a->type_ == EFunction) {
			switch (b->type_) {
			case EValue:
			case EString: {
				Expression expr(a->function_->fold(b));
				if (expr.type_ != EApply)
					return (expr.simplify());
				return (expr);
			}
			default:
				break;
			}
		}
#endif
		return (new Expression(a, b));
	}
	case EFunction:
		return (self->function_->simplify(self));
	default:
		return (Ref<Expression>());
	}
}

Name
Expression::name(void) const
{
	switch (type_) {
	case EVariable:
		return (name_);
	default:
		throw "Expression is not a variable name.";
	}
}

Scalar
Expression::scalar(void) const
{
	switch (type_) {
	case EValue:
		return (scalar_);
	default:
		throw "Expression is not scalar.";
	}
}

String
Expression::string(void) const
{
	switch (type_) {
	case EString:
		return (str_);
	default:
		throw "Expression is not a string.";
	}
}

std::ostream&
operator<< (std::ostream& os, const Expression& e)
{
	switch (e.type_) {
	case Expression::EVariable:
		return (os << e.name_);
	case Expression::EValue:
		return (os << e.scalar());
	case Expression::EApply:
		if (e.expressions_[0]->type_ == Expression::EFunction)
			os << '(' << *(e.expressions_[0]) << ')';
		else
			os << *(e.expressions_[0]);
		os << ' ';
		if (e.expressions_[1]->type_ == Expression::EApply ||
		    e.expressions_[1]->type_ == Expression::EFunction)
			os << '(' << *(e.expressions_[1]) << ')';
		else
			os << *(e.expressions_[1]);
		return (os);
	case Expression::EFunction:
		return (e.function_->print(os));
	case Expression::EString:
		return (os << e.string());
	default:
		throw "Invalid type. (render)";
	}
}
