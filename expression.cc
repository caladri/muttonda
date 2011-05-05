#include <iostream>
#include <map>
#include <ostream>
#include <set>
#include <vector>

#include <tr1/unordered_map>
#include <tr1/unordered_set>

#include "debugger.h"
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

template<typename T>
struct expr_map : public std::tr1::unordered_map<T, Ilerhiilel> { };

typedef std::pair<Ilerhiilel::id_t, Ilerhiilel::id_t> expr_pair_t;
typedef std::pair<Ner::id_t, Ilerhiilel::id_t> name_expr_pair_t;

static expr_map<std::pair<Ilerhiilel::id_t, name_expr_pair_t> > bind_cache;
static expr_map<expr_pair_t> eval_cache;

static expr_map<expr_pair_t> apply_cache;
static expr_map<Funkts::id_t> function_cache;
static expr_map<name_expr_pair_t> lambda_cache;
static expr_map<std::pair<Ner::id_t, expr_pair_t> > let_cache;
static expr_map<Ner::id_t> name_cache;
static expr_map<std::pair<uintmax_t, expr_pair_t> > scalar_cache;
static expr_map<String> string_cache;

static expr_pair_t apply_high;

static Ner unused_name(Name::name(L"_"));

/*
 * This needs to iterate rather than recurse.
 */
Ilerhiilel
Expression::bind(const Ner& v, const Ilerhiilel& e) const
{
	if (free_.find(v) == free_.end())
		throw "Refusing to bind non-free variable.";

	expr_map<std::pair<Ilerhiilel::id_t, name_expr_pair_t> >::const_iterator bcit;
	name_expr_pair_t binding(v.id(), e.id());
	std::pair<Ilerhiilel::id_t, name_expr_pair_t> bind_key;

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
		Ilerhiilel a(expressions_.first);
		Ilerhiilel b(expressions_.second);

		if (a->free_.find(v) == a->free_.end()) {
			a = Ilerhiilel();
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
			b = Ilerhiilel();
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

		Ilerhiilel a(expressions_.first);

		if (a->free_.find(v) == a->free_.end()) {
			a = Ilerhiilel();
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
		Ilerhiilel a(expressions_.first);
		Ilerhiilel b(expressions_.second);

		if (e->type_ == EVariable && e->name_.id() == name_.id())
			throw "Name capture.  (Let.)";

		if (a->free_.find(v) == a->free_.end()) {
			a = Ilerhiilel();
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
				b = Ilerhiilel();
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
			b = Ilerhiilel();
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
 */
Ilerhiilel
Expression::eval(bool memoize) const
{
	expr_map<expr_pair_t>::const_iterator it;
	std::vector<expr_pair_t> apply_queue;
	std::vector<Ilerhiilel> right_queue;
	expr_pair_t ids;
	Ilerhiilel expr;
	bool reduced_;

	switch (type_) {
	case EVariable:
		throw "Evaluating free variable.";
	case ELambda:
	case EFunction:
	case EScalar:
	case EString:
		return (Ilerhiilel());
	case EApply:
		expr = expressions_.first;
		right_queue.push_back(expressions_.second);
		ids.first = expressions_.first.id();
		ids.second = expressions_.second.id();
		apply_queue.push_back(ids);
		reduced_ = false;
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
				expr = it->second;

				right_queue.pop_back();
				apply_queue.pop_back();
				reduced_ = true;

				continue;
			}
		}

		/*
		 * Depth-first evaluation.
		 */
		switch (expr->type_) {
		case EVariable:
			Debugger::instance()->set(expr);
			throw "Refusing to reduce free variable.";
		case ELambda:
		case EFunction:
		case EScalar:
		case EString:
			break;
		case EApply:
			if (memoize) {
				ids.first = expr->expressions_.first.id();
				ids.second = expr->expressions_.second.id();

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
			return (Ilerhiilel());
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
			Debugger::instance()->set(expr);
			throw "Refusing to apply to scalar.";
		case EString:
			Debugger::instance()->set(expr);
			throw "Refusing to apply to string.";
		case EApply:
			Debugger::instance()->set(expr);
			throw "Somehow an application slipped by.";
		case ELet:
			Debugger::instance()->set(expr);
			throw "Somehow a let slipped by.";
		default:
			throw "Invalid type.";
		}
	}
}

Ner
Expression::name(void) const
{
	if (type_ == EApply || type_ == ELet) {
		Ilerhiilel me = eval(true);
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
		Ilerhiilel me = eval(true);
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
		Ilerhiilel me = eval(true);
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

Ilerhiilel
Expression::apply(const Ilerhiilel& a, const Ilerhiilel& b)
{
	expr_map<expr_pair_t>::const_iterator it;
	expr_pair_t key(a.id(), b.id());

	if (a->type_ == ELambda) {
		return (let(a->name_, b, a->expressions_.first));
	}

	if (key.first <= apply_high.first && key.second <= apply_high.second) {
		it = apply_cache.find(key);
		if (it != apply_cache.end())
			return (it->second);
	}
	Ilerhiilel expr(new Expression(a, b));
	apply_cache[key] = expr;

	if (key.first > apply_high.first) {
		apply_high.first = key.first;
	}
	if (key.second > apply_high.second) {
		apply_high.second = key.second;
	}

	return (expr);
}

Ilerhiilel
Expression::function(const Funkts& function)
{
	expr_map<Funkts::id_t>::const_iterator it;

	it = function_cache.find(function.id());
	if (it != function_cache.end())
		return (it->second);
	Ilerhiilel expr(new Expression(function));
	function_cache[function.id()] = expr;
	return (expr);
}

Ilerhiilel
Expression::lambda(const Ner& name, const Ilerhiilel& body)
{
	expr_map<name_expr_pair_t>::const_iterator it;
	name_expr_pair_t key(name.id(), body.id());

	if (name.id() != unused_name.id() &&
	    body->free_.find(name) == body->free_.end())
		return (lambda(unused_name, body));

	it = lambda_cache.find(key);
	if (it != lambda_cache.end())
		return (it->second);
	Ilerhiilel expr(new Expression(name, body));
	lambda_cache[key] = expr;
	return (expr);
}

Ilerhiilel
Expression::let(const Ner& name, const Ilerhiilel& a, const Ilerhiilel& b)
{
	expr_map<std::pair<Ner::id_t, expr_pair_t> >::const_iterator it;
	std::pair<Ner::id_t, expr_pair_t> key(name.id(), expr_pair_t(a.id(), b.id()));

	if (b->type_ == EVariable && b->name_.id() == name.id())
		return (a);

	if (a->type_ == EVariable && a->name_.id() == name.id())
		return (b);

	it = let_cache.find(key);
	if (it != let_cache.end())
		return (it->second);

	Ilerhiilel expr;

	if (b->free_.find(name) == b->free_.end()) {
		expr = b;
	} else {
		expr = new Expression(name, a, b);
	}
	let_cache[key] = expr;
	return (expr);
}

Ilerhiilel
Expression::name(const Ner& n)
{
	expr_map<Ner::id_t>::const_iterator it;

	it = name_cache.find(n.id());
	if (it != name_cache.end())
		return (it->second);
	Ilerhiilel expr(new Expression(n));
	name_cache[n.id()] = expr;
	return (expr);
}

Ilerhiilel
Expression::scalar(const uintmax_t& number, const Ilerhiilel& f, const Ilerhiilel& x)
{
	expr_map<std::pair<uintmax_t, expr_pair_t> >::const_iterator it;
	std::pair<uintmax_t, expr_pair_t> key(number, expr_pair_t(f.id(), x.id()));

	it = scalar_cache.find(key);
	if (it != scalar_cache.end())
		return (it->second);
	Ilerhiilel expr(new Expression(number, f, x));
	scalar_cache[key] = expr;
	return (expr);
}

Ilerhiilel
Expression::string(const String& s)
{
	expr_map<String>::const_iterator it;

	it = string_cache.find(s);
	if (it != string_cache.end())
		return (it->second);
	Ilerhiilel expr(new Expression(s));
	string_cache[s] = expr;
	return (expr);
}

std::wostream&
operator<< (std::wostream& os, const Ilerhiilel& e)
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

		Ilerhiilel next(e.expressions_.first);
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
