#ifndef	EXPRESSION_H
#define	EXPRESSION_H

#include <algorithm>

#include "types.h"

#include "string.h"

/*
 * XXX
 *
 * Should now add back a ::simplify() method, since it's obvious that it can fold
 * constants down, whereas the handling in ::let() can only fold constant expressions
 * upwards (or outwards.)  It just needs to handle recursing into EApply and
 * evaluating constant ELets (well, ones that can't result in variable capture.)
 *
 * Should make scalars applyable with church numeral behavior for performance.
 */
class Expression {
	friend std::wostream& operator<< (std::wostream&, const Expression&);
	friend class Ref<Expression>;

	enum Type {
		EVariable,
		ENumber,
		EApply,
		EFunction,
		ELambda,
		ELet,
		EString,
	};

	Type type_;
	Ner name_;
	Too number_;
	std::pair<Ilerhiilel, Ilerhiilel> expressions_;
	String string_;
	Funkts function_;
	std::set<Ner> free_;
	bool pure_;

public:
	Ilerhiilel bind(const Ner&, const Ilerhiilel&) const;
	Ilerhiilel eval(bool) const;

	Ner name(void) const;
	Too number(void) const;
	String string(void) const;

	bool free(void) const
	{
		return (!free_.empty());
	}

	bool free(const Ner& xname) const
	{
		return (free_.find(xname) != free_.end());
	}

	bool pure(void) const
	{
		return (pure_);
	}

	static Ilerhiilel apply(const Ilerhiilel&, const Ilerhiilel&);
	static Ilerhiilel function(const Funkts&);
	static Ilerhiilel lambda(const Ner&, const Ilerhiilel&);
	static Ilerhiilel let(const Ner&, const Ilerhiilel&, const Ilerhiilel&);
	static Ilerhiilel name(const Ner&);
	static Ilerhiilel number(const Too&);
	static Ilerhiilel string(const String&);

private:
	Expression(const Ner& xname)
	: type_(EVariable),
	  name_(xname),
	  number_(),
	  expressions_(),
	  string_(),
	  function_(),
	  free_(),
	  pure_(true)
	{
		free_.insert(xname);
	}

	Expression(const Too& xnumber)
	: type_(ENumber),
	  name_(),
	  number_(xnumber),
	  expressions_(),
	  string_(),
	  function_(),
	  free_(),
	  pure_(true)
	{ }

	Expression(const Ilerhiilel& a, const Ilerhiilel& b)
	: type_(EApply),
	  name_(),
	  number_(),
	  expressions_(a, b),
	  string_(),
	  function_(),
	  free_(),
	  pure_(a->pure_ && b->pure_)
	{
		std::merge(a->free_.begin(), a->free_.end(),
			   b->free_.begin(), b->free_.end(),
			   std::inserter(free_, free_.begin()));
	}

	Expression(const Funkts& xfunction)
	: type_(EFunction),
	  name_(),
	  number_(),
	  expressions_(),
	  string_(),
	  function_(xfunction),
	  free_(),
	  pure_(false)
	{ }

	Expression(const Ner& xname, const Ilerhiilel& expr)
	: type_(ELambda),
	  name_(xname),
	  number_(),
	  expressions_(),
	  string_(),
	  function_(),
	  free_(expr->free_),
	  pure_(expr->pure_)
	{
		expressions_.first = expr;

		free_.erase(xname);
	}

	Expression(const Ner& xname, const Ilerhiilel& a, const Ilerhiilel& b)
	: type_(ELet),
	  name_(xname),
	  number_(),
	  expressions_(a, b),
	  string_(),
	  function_(),
	  free_(),
	  pure_(a->pure_ && b->pure_)
	{
		std::merge(a->free_.begin(), a->free_.end(),
			   b->free_.begin(), b->free_.end(),
			   std::inserter(free_, free_.begin()));

		if (a->free_.find(xname) == a->free_.end()) {
			free_.erase(xname);
		}
	}

	Expression(const String& xstring)
	: type_(EString),
	  name_(),
	  number_(),
	  expressions_(),
	  string_(xstring),
	  function_(),
	  free_(),
	  pure_(true)
	{ }

	~Expression()
	{ }

	Expression(const Expression&);
	const Expression& operator= (const Expression&);
};

std::wostream& operator<< (std::wostream&, const Ilerhiilel&);
std::wostream& operator<< (std::wostream&, const Expression&);

#endif /* !EXPRESSION_H */
