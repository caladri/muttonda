#include <ostream>
#include <map>
#include <string>
#include <vector>

#include "expression.h"
#include "function.h"
#include "name.h"

std::wostream&
Function::print(std::wostream& os) const
{
	std::vector<Ref<Expression> >::const_iterator it;

	os << name_;
	for (it = expressions_.begin(); it != expressions_.end(); ++it)
		os << " " << *it;

	return (os);
}
