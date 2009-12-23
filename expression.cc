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

Expression::Expression(const Expression& a, const Expression& b)
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

Expression::Expression(const Expression& src)
: type_(src.type_),
  name_(src.name_),
  scalar_(src.scalar_),
  expressions_(src.expressions_),
  str_(src.str_),
  function_(NULL)
{
	if (src.function_ != NULL)
		function_ = src.function_->clone();
}

Expression::~Expression()
{
	if (function_ != NULL) {
		delete function_;
		function_ = NULL;
	}
}

Expression&
Expression::operator= (const Expression& src)
{
	type_ = src.type_;
	name_ = src.name_;
	scalar_ = src.scalar_;
	expressions_ = src.expressions_;
	str_ = src.str_;
	if (function_ != NULL) {
		delete function_;
		function_ = NULL;
	}
	if (src.function_ != NULL)
		function_ = src.function_->clone();
	return (*this);
}

void
Expression::bind(const Name& v, const Expression& e)
{
	switch (type_) {
	case EVariable:
		if (name_ == v)
			*this = e;
		break;
	case EValue:
	case EString:
		break;
	case EApply:
		expressions_[0].bind(v, e);
		expressions_[1].bind(v, e);
		break;
	case EFunction: {
		function_->bind(v, e);
		break;
	}
	default:
		throw "Invalid type. (bind)";
	}
}

Expression
Expression::eval(void) const
{
	try {
		switch (type_) {
		case EVariable:
			throw "Unbound variable.";
		case EFunction:
		case EValue:
		case EString:
			return (*this);
		case EApply:
			return (expressions_[0](expressions_[1]));
		default:
			throw "Invalid type. (eval)";
		}
	} catch(...) {
		std::cerr << "From: " << *this << std::endl;
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
Expression
Expression::simplify(void) const
{
	Lambda *mine;

	switch (type_) {
	case EApply: {
		Expression a(expressions_[0].simplify());
		Expression b(expressions_[1].simplify());

		if (a.type_ == EFunction) {
			switch (b.type_) {
			case EValue:
			case EString: {
				Expression expr(a.function_->fold(b));
				if (expr.type_ != EApply)
					return (expr.simplify());
				return (expr);
			}
			default:
				break;
			}
		}
		return (Expression(a, b));
	}
	case EFunction:
		mine = dynamic_cast<Lambda *>(function_);
		if (mine != NULL) {
			Expression body(mine->expr_.simplify());
			if (body.type_ == EFunction) {
				Lambda *theirs = dynamic_cast<Lambda *>(body.function_);
				if (theirs != NULL) {
					std::vector<Name> names(mine->names_);
					names.insert(names.end(), theirs->names_.begin(), theirs->names_.end());

					Expression expr(theirs->expr_);
					if (expr.type_ == EFunction) {
						theirs = dynamic_cast<Lambda *>(expr.function_);
						if (theirs != NULL) {
							expr = Expression(Lambda(names, expr));
							return (expr.simplify());
						}
					}
					return (Lambda(names, expr));
				}
			}
			return (Expression(Lambda(mine->names_, body)));
		}
		/* FALLTHROUGH */
	default:
		return (*this);
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

Expression
Expression::operator() (const Expression& b) const
{
	switch (type_) {
	case EVariable:
		throw "Attempting to apply to free variable.";
	case EValue:
		throw "Attempting to apply to scalar.";
	case EApply: {
		Expression a(this->eval());
		return (a(b));
	}
	case EFunction:
		return (function_->apply(b));
	case EString:
		throw "Attempting to apply to string.";
	default:
		throw "Invalid type. (apply)";
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
		if (e.expressions_[0].type_ == Expression::EFunction)
			os << '(' << e.expressions_[0] << ')';
		else
			os << e.expressions_[0];
		os << ' ';
		if (e.expressions_[1].type_ == Expression::EApply ||
		    e.expressions_[1].type_ == Expression::EFunction)
			os << '(' << e.expressions_[1] << ')';
		else
			os << e.expressions_[1];
		return (os);
	case Expression::EFunction:
		return (e.function_->print(os));
	case Expression::EString:
		return (os << e.string());
	default:
		throw "Invalid type. (render)";
	}
}
