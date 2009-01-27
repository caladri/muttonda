#include <ostream>
#include <string>
#include <vector>

#include "expression.h"
#include "function.h"
#include "name.h"

SimpleFunction::SimpleFunction(const std::string& name)
: Function(),
  name_(name),
  expressions_()
{ }

SimpleFunction::SimpleFunction(const SimpleFunction& src)
: Function(),
  name_(src.name_),
  expressions_(src.expressions_)
{ }

SimpleFunction::~SimpleFunction()
{ }

void
SimpleFunction::bind(const Name& v, const Expression& e)
{
	std::vector<Expression>::iterator it;

	for (it = expressions_.begin(); it != expressions_.end(); ++it)
		it->bind(v, e);
}

std::ostream&
SimpleFunction::print(std::ostream& os) const
{
	std::vector<Expression>::const_iterator it;

	os << "(" << name_;
	for (it = expressions_.begin(); it != expressions_.end(); ++it) {
		os << " " << *it;
	}
	os << ")";

	return (os);
}
