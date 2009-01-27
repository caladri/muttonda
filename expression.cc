#include <ostream>
#include <vector>

#include "expression.h"
#include "function.h"
#include "name.h"
#include "scalar.h"

Expression::Expression(const Name& v)
: type_(EVariable),
  name_(v),
  scalar_(),
  expressions_(),
  function_(NULL)
{ }

Expression::Expression(const Scalar& v)
: type_(EValue),
  name_(),
  scalar_(v),
  expressions_(),
  function_(NULL)
{ }

Expression::Expression(const Expression& a, const Expression& b)
: type_(EApply),
  name_(),
  scalar_(),
  expressions_(),
  function_(NULL)
{
	expressions_.push_back(a);
	expressions_.push_back(b);
}

Expression::Expression(const Function& f)
: type_(EFunction),
  name_(),
  scalar_(),
  expressions_(),
  function_(f.clone())
{ }

Expression::Expression(const Expression& src)
: type_(src.type_),
  name_(src.name_),
  scalar_(src.scalar_),
  expressions_(src.expressions_),
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
	std::vector<Expression>::iterator it;

	switch (type_) {
	case EVariable:
		if (name_ == v)
			*this = e;
		break;
	case EValue:
		break;
	case EApply:
		for (it = expressions_.begin();
		     it != expressions_.end(); ++it) {
			it->bind(v, e);
		}
		break;
	case EFunction:
		function_->bind(v, e);
		break;
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
	case EValue:
	case EFunction:
		return (*this);
	case EApply:
		return (expressions_[0](expressions_[1]));
	default:
		throw "Invalid type. (eval)";
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
		return (os << '(' << e.expressions_[0] << ' ' <<
			e.expressions_[1] << ')' );
	case Expression::EFunction:
		return (e.function_->print(os));
	default:
		throw "Invalid type. (render)";
	}
}
