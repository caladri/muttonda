#include <ostream>
#include <string>
#include <vector>

#include "expression.h"
#include "function.h"
#include "lambda.h"
#include "name.h"

Lambda::Lambda(const Name& v, const Expression& e)
: Function(),
  v_(v),
  e_(e)
{ }

Lambda::Lambda(const Lambda& src)
: Function(),
  v_(src.v_),
  e_(src.e_)
{ }

Lambda::~Lambda()
{ }

Function *
Lambda::clone(void) const
{
	return (new Lambda(*this));
}

void
Lambda::bind(const Name& v, const Expression& e)
{
	/* Do not rename if the name is shadowed.  */
	if (v_ == v)
		return;
	e_.bind(v, e);
}

Expression
Lambda::apply(const Expression& v) const
{
	Expression e(e_);

	e.bind(v_, v);

	return (e.eval());
}

std::ostream&
Lambda::print(std::ostream& os) const
{
	return (os << "\\" << v_ << " -> " << e_);
}
