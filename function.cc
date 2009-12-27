#include <ostream>
#include <string>
#include <vector>

#include "expression.h"
#include "function.h"
#include "name.h"

SimpleFunction::SimpleFunction(const std::string& n)
: Function(),
  name_(n),
  expressions_()
{ }

SimpleFunction::SimpleFunction(const SimpleFunction& src)
: Function(src),
  name_(src.name_),
  expressions_(src.expressions_)
{ }

SimpleFunction::~SimpleFunction()
{ }

Ref<Expression>
SimpleFunction::bind(const Name& v, const Ref<Expression>& e)
{
	std::vector<Ref<Expression> >::iterator it;
	std::vector<Ref<Expression> > expressions;

	for (it = expressions_.begin(); it != expressions_.end(); ++it)
		expressions.push_back(Expression::bind(*it, v, e));

	Function *f = this->clone();
	SimpleFunction *sf = dynamic_cast<SimpleFunction *>(f);
	if (sf == NULL)
		throw "Could not clone SimpleFunction for bind.";

	sf->expressions_ = expressions;
	Ref<Expression> expr(new Expression(*sf));

	delete f;

	return (expr);
}

std::string
SimpleFunction::name(void) const
{
	return (name_);
}

std::ostream&
SimpleFunction::print(std::ostream& os) const
{
	std::vector<Ref<Expression> >::const_iterator it;

	os << name_;
	for (it = expressions_.begin(); it != expressions_.end(); ++it)
		os << " " << *it;

	return (os);
}
