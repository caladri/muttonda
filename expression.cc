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
	case EString:
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
	case EString:
		return (*this);
	case EApply:
		return (expressions_[0](expressions_[1]));
	default:
		throw "Invalid type. (eval)";
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
