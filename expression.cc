#include <ostream>
#include <vector>

#include "expression.h"
#include "function.h"
#include "lambda.h"
#include "name.h"
#include "scalar.h"

Expression::Expression(const Name& v)
: type_(EVariable),
  name_(v),
  scalar_(),
  expressions_(),
  str_(),
  function_(NULL),
  simplified_(true)
{ }

Expression::Expression(const Scalar& v)
: type_(EValue),
  name_(),
  scalar_(v),
  expressions_(),
  str_(),
  function_(NULL),
  simplified_(true)
{ }

Expression::Expression(const Expression& a, const Expression& b)
: type_(EApply),
  name_(),
  scalar_(),
  expressions_(),
  str_(),
  function_(NULL),
  simplified_(a.simplified_ && b.simplified_)
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
  function_(NULL),
  simplified_(true)
{ }

Expression::Expression(const Function& f)
: type_(EFunction),
  name_(),
  scalar_(),
  expressions_(),
  str_(),
  function_(f.clone()),
  simplified_(false)
{
	Lambda *mine = dynamic_cast<Lambda *>(function_);
	if (mine != NULL) {
		if (mine->expr_.type_ != EFunction) {
			simplified_ = mine->expr_.simplified_;
		} else {
			mine = dynamic_cast<Lambda *>(mine->expr_.function_);
			if (mine != NULL)
				simplified_ = false;
			else
				simplified_ = true;
		}
	} else
		simplified_ = true;
}

Expression::Expression(const Expression& src)
: type_(src.type_),
  name_(src.name_),
  scalar_(src.scalar_),
  expressions_(src.expressions_),
  str_(src.str_),
  function_(NULL),
  simplified_(src.simplified_)
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
	simplified_ = src.simplified_;
	return (*this);
}

void
Expression::bind(const Name& v, const Expression& e)
{
	switch (type_) {
	case EVariable:
		if (name_ == v) {
			*this = e;
			simplified_ = false;
		}
		break;
	case EValue:
	case EString:
		break;
	case EApply:
		expressions_[0].bind(v, e);
		expressions_[1].bind(v, e);
		simplified_ = expressions_[0].simplified_ && expressions_[1].simplified_;
		break;
	case EFunction: {
		function_->bind(v, e);

		Lambda *mine = dynamic_cast<Lambda *>(function_);
		if (mine != NULL)
			simplified_ = mine->expr_.simplified_;
		else
			simplified_ = true;
		break;
	}
	default:
		throw "Invalid type. (bind)";
	}
}

Expression
Expression::eval(void) const
{
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

	if (simplified_)
		return (*this);

	switch (type_) {
	case EApply: {
		Expression a(expressions_[0].simplify());
		Expression b(expressions_[1].simplify());

		if (a.type_ == EFunction) {
			switch (b.type_) {
			case EValue:
			case EString: {
				/* XXX folding isn't working yet?  */
				Expression expr(a.function_->fold(b));
				if (expr.type_ != EApply) {
					expr.simplified_ = false;
					return (expr.simplify());
				}
				return (expr);
			}
			default:
				break;
			}
		}
		Expression expr(a, b);
		expr.simplified_ = true;
		return (expr);
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
							expr.simplified_ = false;
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
	case EFunction: {
		Expression expr(function_->apply(b));
		expr.simplified_ = false;
		return (expr);
	}
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
