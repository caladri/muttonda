#include <ostream>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "expression.h"
#include "function.h"
#include "name.h"
#include "number.h"

std::wostream&
Function::print(std::wostream& os) const
{
	std::vector<Ilerhiilel>::const_iterator it;

	os << name_;
	for (it = expressions_.begin(); it != expressions_.end(); ++it)
		os << " " << *it;

	return (os);
}
