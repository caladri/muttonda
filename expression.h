#ifndef	EXPRESSION_H
#define	EXPRESSION_H

#include <algorithm>

#include "types.h"

#include "string.h"

namespace std {
	template<>
	struct hash<std::pair<unsigned, unsigned> > {
		size_t operator() (const std::pair<unsigned, unsigned>& p) const
		{
			return (hash<unsigned>()(p.first) + hash<unsigned>()(p.second));
		}
	};

	template<>
	struct hash<std::pair<unsigned, std::pair<unsigned, unsigned> > > {
		size_t operator() (const std::pair<unsigned, std::pair<unsigned, unsigned> >& p) const
		{
			return (hash<unsigned>()(p.first) + hash<std::pair<unsigned, unsigned> >()(p.second));
		}
	};

	template<>
	struct hash<std::pair<uintmax_t, std::pair<unsigned, unsigned> > > {
		size_t operator() (const std::pair<uintmax_t, std::pair<unsigned, unsigned> >& p) const
		{
			return (hash<uintmax_t>()(p.first) + hash<std::pair<unsigned, unsigned> >()(p.second));
		}
	};
}

class Expression {
	friend std::wostream& operator<< (std::wostream&, const Expression&);
	friend class Ref<Expression, ExpressionMeta>;
	friend ExpressionMeta;

public:
	template<typename T>
	struct expr_map : public std::unordered_map<T, Ilerhiilel> { };

private:
	enum Type {
		EVariable,
		ENumber,
		ECurriedNumber,
		EApply,
		EFunction,
		ELambda,
		EString,
		EIdentity,
		ESelfApply,
	};

	Type type_;
	Ner name_;
	Too number_;
	std::pair<Ilerhiilel, Ilerhiilel> expressions_;
	String string_;
	Funkts function_;
	std::unordered_set<Ner::id_t> free_;
	bool pure_;

public:
	Ilerhiilel bind(const Ner&, const Ilerhiilel&) const;
	Ilerhiilel eval(bool) const;

	Ner name(void) const;
	Too number(void) const;
	String string(void) const;

	bool constant(void) const
	{
		switch (type_) {
		case ENumber:
		case EString:
			return (true);
		default:
			return (false);
		}
	}

	bool free(void) const
	{
		return (!free_.empty());
	}

	bool free(const Ner& xname) const
	{
		return (free_.find(xname.id()) != free_.end());
	}

	bool pure(void) const
	{
		return (pure_);
	}

	static bool match(const Ilerhiilel&, const char *);
	static bool match(const Expression&, const char *);
	static size_t match(const Ilerhiilel&, const char *, const std::map<char, Ner>&);
	static size_t match(const Expression&, const char *, const std::map<char, Ner>&);

	static Ilerhiilel apply(const Ilerhiilel&, const Ilerhiilel&);
	static Ilerhiilel function(const Funkts&);
	static Ilerhiilel lambda(const Ner&, const Ilerhiilel&);
	static Ilerhiilel let(const Ner&, const Ilerhiilel&, const Ilerhiilel&);
	static Ilerhiilel name(const Ner&);
	static Ilerhiilel number(const Too&);
	static Ilerhiilel curried_number(const Ilerhiilel&, const Ilerhiilel&);
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
		free_.insert(xname.id());
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

	Expression(const Too& xnumber, const Ilerhiilel expr)
	: type_(ECurriedNumber),
	  name_(),
	  number_(xnumber),
	  expressions_(),
	  string_(),
	  function_(),
	  free_(expr->free_),
	  pure_(expr->pure_)
	{
		expressions_.first = expr;
	}

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

		free_.erase(xname.id());
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

	Expression(const Type& type)
	: type_(type),
	  name_(),
	  number_(),
	  expressions_(),
	  string_(),
	  function_(),
	  free_(),
	  pure_(true)
	{
		if (type != EIdentity)
			throw "Attempt to instantiate wrong type of expression.";
	}

	Expression(const Type& type, const Ilerhiilel& expr)
	: type_(type),
	  name_(),
	  number_(),
	  expressions_(),
	  string_(),
	  function_(),
	  free_(expr->free_),
	  pure_(expr->pure_)
	{
		if (type != ESelfApply)
			throw "Attempt to instantiate wrong type of expression.";

		expressions_.first = expr;
	}

	~Expression()
	{ }

	Expression(const Expression&);
	const Expression& operator= (const Expression&);

public:
	static Ilerhiilel identity;
};

class ExpressionMeta {
	typedef std::pair<Ner::id_t, Ilerhiilel::id_t> name_expr_pair_t;

	Expression *expr_;
	Expression::expr_map<name_expr_pair_t> bind_cache_;
	Expression::expr_map<Ner::id_t> lambda_cache_;
	Ilerhiilel self_apply_;
public:
	ExpressionMeta(Expression *expr)
	: expr_(expr),
	  bind_cache_(),
	  lambda_cache_(),
	  self_apply_()
	{ }

	~ExpressionMeta()
	{ }

	Ilerhiilel bind_cache(const Ner& v, const Ilerhiilel& e)
	{
		const name_expr_pair_t binding(v.id(), e.id());
		Expression::expr_map<name_expr_pair_t>::const_iterator it;
		it = bind_cache_.find(binding);
		if (it == bind_cache_.end()) {
			Ilerhiilel expr = expr_->bind(v, e);
			bind_cache_[binding] = expr;
			return expr;
		}
		return it->second;
	}

	Ilerhiilel lambda_cache(const Ner& name, const Ilerhiilel& body)
	{
		Expression::expr_map<Ner::id_t>::const_iterator it;
		it = lambda_cache_.find(name.id());
		if (it != lambda_cache_.end())
			return (it->second);
		Ilerhiilel expr(new Expression(name, body));
		lambda_cache_[name.id()] = expr;
		return (expr);
	}

	Ilerhiilel self_apply(const Ilerhiilel& self)
	{
		if (self_apply_.null())
			self_apply_ = new Expression(Expression::ESelfApply, self);
		return (self_apply_);
	}
};

std::wostream& operator<< (std::wostream&, const Ilerhiilel&);
std::wostream& operator<< (std::wostream&, const Expression&);

#endif /* !EXPRESSION_H */
