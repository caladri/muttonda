#include <iostream>
#include <map>
#include <ostream>
#include <set>
#include <vector>

#include <tr1/unordered_map>
#include <tr1/unordered_set>

#include "expression.h"
#include "function.h"
#include "name.h"

/*
 * Do a variable renaming pass at some point.
 *
 * Throw a fit about free variables.
 */

/*
 * XXX
 * Check all EScalar.
 */

namespace std {
	namespace tr1 {
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
	};
};

static Ref<Name> unused_name(Name::name(L"_"));

static std::tr1::unordered_map<std::pair<unsigned, std::pair<unsigned, unsigned> >, Ref<Expression> > bind_cache;
static std::tr1::unordered_map<std::pair<unsigned, unsigned>, Ref<Expression> > eval_cache;

static std::tr1::unordered_map<std::pair<unsigned, unsigned>, Ref<Expression> > apply_cache;
static std::tr1::unordered_map<unsigned, Ref<Expression> > function_cache;
static std::tr1::unordered_map<std::pair<unsigned, unsigned>, Ref<Expression> > lambda_cache;
static std::tr1::unordered_map<std::pair<unsigned, std::pair<unsigned, unsigned> >, Ref<Expression> > let_cache;
static std::tr1::unordered_map<unsigned, Ref<Expression> > name_cache;
static std::tr1::unordered_map<std::pair<uintmax_t, std::pair<unsigned, unsigned> >, Ref<Expression> > scalar_cache;
static std::tr1::unordered_map<String, Ref<Expression> > string_cache;

static std::pair<unsigned, unsigned> apply_high;

/*
 * This needs to iterate rather than recurse.
 */
Ref<Expression>
Expression::bind(const Ref<Name>& v, const Ref<Expression>& e) const
{
	if (free_.find(v) == free_.end())
		throw "Refusing to bind non-free variable.";

	std::tr1::unordered_map<std::pair<unsigned, std::pair<unsigned, unsigned> >, Ref<Expression> >::const_iterator bcit;
	std::pair<unsigned, unsigned> binding(v.id(), e.id());
	std::pair<unsigned, std::pair<unsigned, unsigned> > bind_key;

	bind_key.second = binding;

	switch (type_) {
	case EVariable:
		return (e);
	case EScalar:
		throw "Bind called for scalar.";
	case EString:
		throw "Bind called for string.";
	case EFunction:
		throw "Bind called for function.";
	case EApply: {
		Ref<Expression> a(expressions_.first);
		Ref<Expression> b(expressions_.second);

		if (a->free_.find(v) == a->free_.end()) {
			a = Ref<Expression>();
		} else {
			bind_key.first = a.id();
			bcit = bind_cache.find(bind_key);
			if (bcit == bind_cache.end()) {
				a = a->bind(v, e);

				bind_cache[bind_key] = a;
			} else {
				a = bcit->second;
			}
		}

		if (b->free_.find(v) == b->free_.end()) {
			b = Ref<Expression>();
		} else {
			bind_key.first = b.id();
			bcit = bind_cache.find(bind_key);
			if (bcit == bind_cache.end()) {
				b = b->bind(v, e);

				bind_cache[bind_key] = b;
			} else {
				b = bcit->second;
			}
		}

		if (a.null() && b.null())
			throw "Apply had free variable but neither subexpression did.";

		if (a.null())
			a = expressions_.first;

		if (b.null())
			b = expressions_.second;

		return (apply(a, b));
	}
	case ELambda: {
		if (e->type_ == EVariable && e->name_.id() == name_.id())
			throw "Name capture.  (Lambda.)";

		Ref<Expression> a(expressions_.first);

		if (a->free_.find(v) == a->free_.end()) {
			a = Ref<Expression>();
		} else {
			bind_key.first = a.id();
			bcit = bind_cache.find(bind_key);
			if (bcit == bind_cache.end()) {
				a = a->bind(v, e);

				bind_cache[bind_key] = a;
			} else {
				a = bcit->second;
			}
		}

		if (a.null())
			throw "Lambda had free variable but its body did not.";

		return (lambda(name_, a));
	}
	case ELet: {
		Ref<Expression> a(expressions_.first);
		Ref<Expression> b(expressions_.second);

		if (e->type_ == EVariable && e->name_.id() == name_.id())
			throw "Name capture.  (Let.)";

		if (a->free_.find(v) == a->free_.end()) {
			a = Ref<Expression>();
		} else {
			bind_key.first = a.id();
			bcit = bind_cache.find(bind_key);
			if (bcit == bind_cache.end()) {
				a = a->bind(v, e);

				bind_cache[bind_key] = a;
			} else {
				a = bcit->second;
			}
		}

		if (name_.id() != v.id()) {
			if (b->free_.find(v) == b->free_.end()) {
				b = Ref<Expression>();
			} else {
				bind_key.first = b.id();
				bcit = bind_cache.find(bind_key);
				if (bcit == bind_cache.end()) {
					b = b->bind(v, e);

					bind_cache[bind_key] = b;
				} else {
					b = bcit->second;
				}
			}
		} else {
			b = Ref<Expression>();
		}

		if (a.null() && b.null())
			throw "Let had free variable but neither subexpression did.";

		if (a.null())
			a = expressions_.first;

		if (b.null())
			b = expressions_.second;

		return (let(name_, a, b));
	}
	default:
		throw "Invalid type. (bind)";
	}
}

/*
 * XXX
 * Put this all in a try block and dump the expression context in catch.
 *
 * Should the ELet binds really set reduced_?
 */
Ref<Expression>
Expression::eval(bool memoize) const
{
	std::tr1::unordered_map<std::pair<unsigned, unsigned>, Ref<Expression> >::const_iterator it;
	std::vector<std::pair<unsigned, unsigned> > apply_queue;
	std::vector<Ref<Expression> > right_queue;
	std::pair<unsigned, unsigned> ids;
	Ref<Expression> expr;
	bool reduced_;

	switch (type_) {
	case EVariable:
		throw "Evaluating free variable.";
	case ELambda:
	case EFunction:
	case EScalar:
	case EString:
		return (Ref<Expression>());
	case EApply:
		expr = expressions_.first;
		right_queue.push_back(expressions_.second);
		ids = std::pair<unsigned, unsigned>(expressions_.first.id(),
						    expressions_.second.id());
		apply_queue.push_back(ids);
		break;
	case ELet:
		expr = expressions_.second->bind(name_, expressions_.first);
		if (expr.null())
			throw "Mysterious bind.";
		reduced_ = true;
		break;
	default:
		throw "Invalid type.";
	}


	for (;;) {
		if (expr.null())
			throw "Null reference in reduction pass.";

		if (memoize && !right_queue.empty()) {
			ids = apply_queue.back();

			it = eval_cache.find(ids);
			if (it == eval_cache.end()) {
				ids.first = expr.id();
				ids.second = right_queue.back().id();

				it = eval_cache.find(ids);
			}

			if (it != eval_cache.end()) {
				right_queue.pop_back();
				apply_queue.pop_back();
				expr = it->second;
				continue;
			}
		}

		/*
		 * Depth-first evaluation.
		 */
		switch (expr->type_) {
		case EVariable:
			throw "Refusing to reduce free variable.";
		case ELambda:
		case EFunction:
		case EScalar:
		case EString:
			break;
		case EApply:
			if (memoize) {
				ids = std::pair<unsigned, unsigned>(expr->expressions_.first.id(),
								    expr->expressions_.second.id());
				apply_queue.push_back(ids);
			}
			right_queue.push_back(expr->expressions_.second);
			expr = expr->expressions_.first;
			continue;
		case ELet:
			expr = expr->expressions_.second->bind(expr->name_, expr->expressions_.first);
			if (expr.null())
				throw "Failed to reduce let.";
			reduced_ = true;
			continue;
		default:
			throw "Invalid type.";
		}

		if (right_queue.empty()) {
			if (reduced_)
				return (expr);
			return (Ref<Expression>());
		}

		/*
		 * Application.
		 */
		switch (expr->type_) {
		case EVariable:
			throw "Somehow a free variable slipped by.";
		case ELambda:
			if (expr->name_.id() == unused_name.id()) {
				expr = expr->expressions_.first;

				if (memoize) {
					eval_cache[ids] = expr;

					ids = apply_queue.back();

					eval_cache[ids] = expr;
				}

				right_queue.pop_back();
				apply_queue.pop_back();
				reduced_ = true;

				continue;
			}

			expr = expr->expressions_.first->bind(expr->name_, right_queue.back());
			if (expr.null())
				throw "Apply to non-_ parameter must not be null.";

			if (memoize) {
				eval_cache[ids] = expr;

				ids = apply_queue.back();

				eval_cache[ids] = expr;
			}

			right_queue.pop_back();
			apply_queue.pop_back();
			reduced_ = true;
			continue;
		case EFunction:
			expr = expr->function_->apply(right_queue.back(), memoize);
			if (expr.null())
				throw "Builtin function must not return null.";

			if (memoize) {
				eval_cache[ids] = expr;

				ids = apply_queue.back();

				eval_cache[ids] = expr;
			}

			right_queue.pop_back();
			apply_queue.pop_back();
			reduced_ = true;
			continue;
		case EScalar:
			throw "Refusing to apply to scalar.";
		case EString:
			throw "Refusing to apply to string.";
		case EApply:
			throw "Somehow an application slipped by.";
		case ELet:
			throw "Somehow a let slipped by.";
		default:
			throw "Invalid type.";
		}
	}
}

Ref<Name>
Expression::name(void) const
{
	if (type_ == EApply || type_ == ELet) {
		Ref<Expression> me = eval(true);
		if (!me.null())
			return (me->name());
	}
	switch (type_) {
	case EVariable:
		return (name_);
	default:
		throw "Expression is not variable.";
	}
}

uintmax_t
Expression::scalar(void) const
{
	if (type_ == EApply || type_ == ELet ||
	    (!expressions_.first.null() && !expressions_.second.null())) {
		Ref<Expression> me = eval(true);
		if (!me.null())
			return (me->scalar());
	}
	switch (type_) {
	case EScalar:
		if (!expressions_.first.null())
			throw "Lemon curry?";
		return (number_);
	default:
		throw "Expression is not scalar.";
	}
}

String
Expression::string(void) const
{
	if (type_ == EApply || type_ == ELet) {
		Ref<Expression> me = eval(true);
		if (!me.null())
			return (me->string());
	}
	switch (type_) {
	case EString:
		return (string_);
	default:
		throw "Expression is not a string.";
	}
}

Ref<Expression>
Expression::apply(const Ref<Expression>& a, const Ref<Expression>& b)
{
	std::tr1::unordered_map<std::pair<unsigned, unsigned>, Ref<Expression> >::const_iterator it;
	std::pair<unsigned, unsigned> key(a.id(), b.id());

	if (a->type_ == ELambda) {
		return (let(a->name_, b, a->expressions_.first));
	}

	if (key.first <= apply_high.first && key.second <= apply_high.second) {
		it = apply_cache.find(key);
		if (it != apply_cache.end())
			return (it->second);
	}
	Ref<Expression> expr(new Expression(a, b));
	apply_cache[key] = expr;

	if (key.first > apply_high.first) {
		apply_high.first = key.first;
	}
	if (key.second > apply_high.second) {
		apply_high.second = key.second;
	}

	return (expr);
}

Ref<Expression>
Expression::function(const Ref<Function>& function)
{
	std::tr1::unordered_map<unsigned, Ref<Expression> >::const_iterator it;

	it = function_cache.find(function.id());
	if (it != function_cache.end())
		return (it->second);
	Ref<Expression> expr(new Expression(function));
	function_cache[function.id()];
	return (expr);
}

Ref<Expression>
Expression::lambda(const Ref<Name>& name, const Ref<Expression>& body)
{
	std::tr1::unordered_map<std::pair<unsigned, unsigned>, Ref<Expression> >::const_iterator it;
	std::pair<unsigned, unsigned> key(name.id(), body.id());

	if (name.id() != unused_name.id() &&
	    body->free_.find(name) == body->free_.end())
		return (lambda(unused_name, body));

	it = lambda_cache.find(key);
	if (it != lambda_cache.end())
		return (it->second);
	Ref<Expression> expr(new Expression(name, body));
	lambda_cache[key] = expr;
	return (expr);
}

Ref<Expression>
Expression::let(const Ref<Name>& name, const Ref<Expression>& a, const Ref<Expression>& b)
{
	std::tr1::unordered_map<std::pair<unsigned, std::pair<unsigned, unsigned> >, Ref<Expression> >::const_iterator it;
	std::pair<unsigned, std::pair<unsigned, unsigned> > key(name.id(), std::pair<unsigned, unsigned>(a.id(), b.id()));

	if (b->type_ == EVariable && b->name_.id() == name.id())
		return (a);

	if (a->type_ == EVariable && a->name_.id() == name.id())
		return (b);

	it = let_cache.find(key);
	if (it != let_cache.end())
		return (it->second);

	Ref<Expression> expr;

	if (b->free_.find(name) == b->free_.end()) {
		expr = b;
	} else {
		expr = new Expression(name, a, b);
	}
	let_cache[key] = expr;
	return (expr);
}

Ref<Expression>
Expression::name(const Ref<Name>& n)
{
	std::tr1::unordered_map<unsigned, Ref<Expression> >::const_iterator it;

	it = name_cache.find(n.id());
	if (it != name_cache.end())
		return (it->second);
	Ref<Expression> expr(new Expression(n));
	name_cache[n.id()] = expr;
	return (expr);
}

Ref<Expression>
Expression::scalar(const uintmax_t& number, const Ref<Expression>& f, const Ref<Expression>& x)
{
	std::tr1::unordered_map<std::pair<uintmax_t, std::pair<unsigned, unsigned> >, Ref<Expression> >::const_iterator it;
	std::pair<uintmax_t, std::pair<unsigned, unsigned> > key(number, std::pair<unsigned, unsigned>(f.id(), x.id()));

	it = scalar_cache.find(key);
	if (it != scalar_cache.end())
		return (it->second);
	Ref<Expression> expr(new Expression(number, f, x));
	scalar_cache[key] = expr;
	return (expr);
}

Ref<Expression>
Expression::string(const String& s)
{
	std::tr1::unordered_map<String, Ref<Expression> >::const_iterator it;

	it = string_cache.find(s);
	if (it != string_cache.end())
		return (it->second);
	Ref<Expression> expr(new Expression(s));
	string_cache[s] = expr;
	return (expr);
}

std::wostream&
operator<< (std::wostream& os, const Ref<Expression>& e)
{
	if (e.null())
		throw "Cowardly refusing to print a null Expression.";
	return (os << *e);
}

std::wostream&
operator<< (std::wostream& os, const Expression& e)
{
	switch (e.type_) {
	case Expression::EVariable:
		return (os << e.name_);
	case Expression::EScalar:
		return (os << e.scalar());
	case Expression::EApply:
		if (e.expressions_.first->type_ == Expression::EFunction ||
		    e.expressions_.first->type_ == Expression::ELambda ||
		    e.expressions_.first->type_ == Expression::ELet)
			os << '(' << e.expressions_.first << ')';
		else
			os << e.expressions_.first;
		os << ' ';
		if (e.expressions_.second->type_ == Expression::EApply ||
		    e.expressions_.second->type_ == Expression::EFunction ||
		    e.expressions_.second->type_ == Expression::ELambda ||
		    e.expressions_.second->type_ == Expression::ELet)
			os << '(' << e.expressions_.second << ')';
		else
			os << e.expressions_.second;
		return (os);
	case Expression::EFunction:
		return (e.function_->print(os));
	case Expression::ELambda: {
		os << "\\" << e.name_;

		Ref<Expression> next(e.expressions_.first);
		while (next->type_ == Expression::ELambda) {
			os << " " << next->name_;
			next = next->expressions_.first;
		}

		os << " -> " << next;

		return (os);
	}
	case Expression::ELet:
		os << "let " << e.name_ << " ";
		if (e.expressions_.first->type_ == Expression::EApply ||
		    e.expressions_.first->type_ == Expression::EFunction ||
		    e.expressions_.first->type_ == Expression::ELambda ||
		    e.expressions_.first->type_ == Expression::ELet)
			os << '(' << e.expressions_.first << ')';
		else
			os << e.expressions_.first;
		os << " " << e.expressions_.second;
		return (os);
	case Expression::EString:
		return (os << e.string());
	default:
		throw "Invalid type. (render)";
	}
}
