#include <ostream>
#include <string>
#include <vector>

#include "expression.h"
#include "function.h"
#include "name.h"

Ref<Expression>
SimpleFunction::bind(const Name& v, const Ref<Expression>& e) const
{
	std::vector<Ref<Expression> >::const_iterator it;
	std::vector<Ref<Expression> > expressions;
	bool all_null = true;

	for (it = expressions_.begin(); it != expressions_.end(); ++it) {
		Ref<Expression> expr(*it);

		expr = expr->bind(v, e);
		if (expr.null()) {
			expressions.push_back(*it);
		} else {
			expressions.push_back(expr);
			all_null = false;
		}
	}
	
	if (all_null)
		return (Ref<Expression>());

	Function *f = this->clone();
	SimpleFunction *sf = dynamic_cast<SimpleFunction *>(f);
	if (sf == NULL)
		throw "Could not clone SimpleFunction for bind.";

	sf->expressions_ = expressions;
	Ref<Expression> expr(new Expression(f));

	return (expr);
}

std::wostream&
SimpleFunction::print(std::wostream& os) const
{
	std::vector<Ref<Expression> >::const_iterator it;

	os << name_;
	for (it = expressions_.begin(); it != expressions_.end(); ++it)
		os << " " << *it;

	return (os);
}
