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
#include "number.h"

#ifdef __clang__
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#endif

/*
 * Do a variable renaming pass at some point.
 *
 * Throw a fit about free variables.
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
static expr_map<Too::id_t> number_cache;
static expr_map<String> string_cache;

static expr_pair_t apply_cache_high;

static Ner unused_name(Name::name(L"_"));
Ilerhiilel Expression::identity(new Expression(Expression::EIdentity));

/*
 * This needs to iterate rather than recurse.
 */
Ilerhiilel
Expression::bind(const Ner& v, const Ilerhiilel& e) const
{
	if(v.id() == unused_name.id())
		throw "Bind of _.";

	if (free_.find(v) == free_.end())
		throw "Refusing to bind non-free variable.";

	expr_map<std::pair<Ilerhiilel::id_t, name_expr_pair_t> >::const_iterator bcit;
	name_expr_pair_t binding(v.id(), e.id());
	std::pair<Ilerhiilel::id_t, name_expr_pair_t> bind_key;

	bind_key.second = binding;

	switch (type_) {
	case EVariable:
		return (e);
	case ENumber:
		throw "Bind called for number.";
	case EString:
		throw "Bind called for string.";
	case EFunction:
		throw "Bind called for function.";
	case EIdentity:
		throw "Bind called for identity.";
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
	case ECurriedNumber: {
		if (e->type_ == EVariable && e->name_.id() == name_.id())
			throw "Name capture.  (Curried number.)";

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
			throw "Curried number had free variable but its parameter did not.";

		return (curried_number(number(number_), a));
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
	bool reduced;
	uintmax_t n, k;

	switch (type_) {
	case EVariable:
		throw "Evaluating free variable.";
	case ECurriedNumber:
		if (expressions_.first->type_ == EVariable)
			throw "Free variable applied to number.";
	case ELambda:
	case EFunction:
	case ENumber:
	case EString:
	case EIdentity:
		return (Ilerhiilel());
	case EApply:
		expr = expressions_.first;
		right_queue.push_back(expressions_.second);
		ids.first = expressions_.first.id();
		ids.second = expressions_.second.id();
		apply_queue.push_back(ids);
		reduced = false;
		break;
	case ELet:
		expr = expressions_.second->bind(name_, expressions_.first);
		if (expr.null())
			throw "Mysterious bind.";
		reduced = true;
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
				reduced = true;

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
		case ENumber:
		case ECurriedNumber:
		case EIdentity:
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
			reduced = true;
			continue;
		default:
			throw "Invalid type.";
		}

		if (right_queue.empty()) {
			if (reduced)
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
				reduced = true;

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
			reduced = true;
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
			reduced = true;
			continue;
		case ENumber:
			if (right_queue.back()->type_ == EVariable)
				throw "Application of free variable to number.";

			expr = curried_number(expr, right_queue.back());

			if (memoize) {
				eval_cache[ids] = expr;

				ids = apply_queue.back();

				eval_cache[ids] = expr;
			}

			right_queue.pop_back();
			apply_queue.pop_back();
			reduced = true;
			continue;
		case ECurriedNumber:
			k = expr->number_->number();

			if (k != 0) {
				Ilerhiilel f = expr->expressions_.first;
				expr = right_queue.back();

				for (n = 0; n < k; n++) {
					expr = apply(f, expr);
					Ilerhiilel t = expr->eval(memoize);
					if (!t.null())
						expr = t;
				}
			} else {
				expr = right_queue.back();
			}

			if (memoize) {
				eval_cache[ids] = expr;

				ids = apply_queue.back();

				eval_cache[ids] = expr;
			}

			right_queue.pop_back();
			apply_queue.pop_back();
			reduced = true;
			continue;
		case EIdentity:
			expr = right_queue.back();

			right_queue.pop_back();
			apply_queue.pop_back();
			reduced = true;
			continue;
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

Too
Expression::number(void) const
{
	if (type_ == EApply || type_ == ELet ||
	    (!expressions_.first.null() && !expressions_.second.null())) {
		Ilerhiilel me = eval(true);
		if (!me.null())
			return (me->number());
	}
	switch (type_) {
	case ENumber:
		if (!expressions_.first.null())
			throw "Lemon curry?";
		return (number_);
	default:
		throw "Expression is not a number.";
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

	if (a->type_ == EIdentity)
		return (b);

	/*
	 * Optimizations of the case where a is a lambda should
	 * be handled in ::let().
	 */
	if (a->type_ == ELambda)
		return (let(a->name_, b, a->expressions_.first));

	/*
	 * This is special-cased as ::curried_number().
	 */
	if (a->type_ == ENumber)
		return (curried_number(a, b));

	if (key.first <= apply_cache_high.first && key.second <= apply_cache_high.second) {
		if (a->pure_ && b->pure_) {
			it = eval_cache.find(key);
			if (it != eval_cache.end())
				return (it->second);
		}

		it = apply_cache.find(key);
		if (it != apply_cache.end())
			return (it->second);
	}
	Ilerhiilel expr(new Expression(a, b));
	apply_cache[key] = expr;

	if (key.first > apply_cache_high.first) {
		apply_cache_high.first = key.first;
	}
	if (key.second > apply_cache_high.second) {
		apply_cache_high.second = key.second;
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

	if (name.id() != unused_name.id()) {
		if (body->free_.find(name) == body->free_.end())
			return (lambda(unused_name, body));
		if (body->type_ == EVariable &&
		    body->name_.id() == name.id())
			return (identity);
		if (body->type_ == ECurriedNumber) {
			/*
			 * This turns:
			 * 	\z -> (21 z)
			 * Into:
			 * 	21
			 */
			if (body->expressions_.first->type_ == EVariable &&
			    body->expressions_.first->name_.id() == name.id())
				return (number(body->number_));
		}
		if (body->type_ == EApply) {
			/*
			 * If this is an expression in the form of:
			 * 	\x -> a x
			 * Then convert it to simply:
			 * 	a
			 * XXX This transformation is invalid for things with side-effects.
			 * Consider:
			 * 	(\a b -> error a b) I
			 * Should be:
			 * 	\b -> error (\x -> x) b
			 * And must not be:
			 * 	error (\x -> x)
			 */
			if (body->expressions_.second->type_ == EVariable &&
			    body->expressions_.second->name_.id() == name.id() &&
			    body->expressions_.first->free_.find(name) ==
			    body->expressions_.first->free_.end() &&
			    body->expressions_.first->pure_) {
				return (body->expressions_.first);
			}
		}
	}

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

	/*
	 * Turns:
	 * 	let _ a b
	 * Into:
	 * 	b
	 * 
	 * This is explicit so as to handle:
	 * 	let _ a _
	 * Which must not actually bind _, or treat it like an identity.
	 */
	if (name.id() == unused_name.id())
		return (b);

	if (b->type_ == EVariable) {
		/*
		 * Turns:
		 * 	let x y x
		 * Into:
		 * 	y
		 */
		if (b->name_.id() == name.id())
			return (a);
		/*
		 * Turns:
		 * 	let x y a
		 * Into:
		 * 	a
		 */
		return (b);
	}

	if (a->type_ == EVariable) {
		/*
		 * Turns:
		 * 	let x x y
		 * Into:
		 * 	y
		 */
		if (a->name_.id() == name.id())
			return (b);
	}

#if 0
	/*
	 * Turns:
	 * 	let f a f z
	 * Into:
	 * 	a z
	 *
	 * Likewise:
	 * 	let f a z f
	 * Into:
	 * 	z a
	 *
	 * Likewise:
	 * 	let f a f f
	 * Into:
	 * 	a a
	 */
	if (b->type_ == EApply) {
		Ilerhiilel l, r;

		l = b->expressions_.first;
		r = b->expressions_.second;

		if (l->name_.id() == name.id() && r->name_.id() == name.id())
			return (apply(a, a));
		if (l->name_.id() == name.id() && !r->free(name))
			return (apply(a, r));
		if (r->name_.id() == name.id() && !l->free(name))
			return (apply(l, a));
	}
#endif

	/*
	 * XXX
	 * This could be broader.  Turns
	 * 	let x y \_ -> a
	 * into:
	 * 	\_ -> let x y a
	 * Which then trivially reduces to:
	 * 	\_ -> a
	 * If we had better variable renaming and general avoidance of name
	 * capture, this would be much easier to get right in the general
	 * case.
	 */
	if (b->type_ == ELambda && b->name_.id() == unused_name.id())
		return (lambda(b->name_, let(name, a, b->expressions_.first)));

	/*
	 * Constant propagation.
	 */
	switch (a->type_) {
	case ECurriedNumber:
		/*
		 * A curried number with no free variables is a constant and
		 * can be propagated.
		 */
		if (!a->free_.empty())
			break;
	case ENumber:
	case EFunction:
	case EString:
	case EIdentity:
		return (b->bind(name, a));
	default:
		break;
	}

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
Expression::number(const Too& n)
{
	expr_map<Too::id_t>::const_iterator it;

	it = number_cache.find(n.id());
	if (it != number_cache.end())
		return (it->second);
	Ilerhiilel expr(new Expression(n));
	number_cache[n.id()] = expr;
	return (expr);
}

/*
 * This is just a special case of ::apply().
 */
Ilerhiilel
Expression::curried_number(const Ilerhiilel& xnumber, const Ilerhiilel& f)
{
	expr_map<expr_pair_t>::const_iterator it;
	expr_pair_t key(xnumber.id(), f.id());
	uintmax_t m;

	m = xnumber->number_->number();

	switch (m) {
	case 0:
		/*
		 * Turns:
		 * 	0 _
		 * Into:
		 * 	I
		 */
		return (lambda(Name::name(L"x"), name(Name::name(L"x"))));
	case 1:
		/*
		 * Turns:
		 * 	1 f x
		 * Into:
		 * 	f
		 */
		return (f);
	default:
		break;
	}

	/*
	 * Turns:
	 * 	(number) (I) [x]
	 * Into:
	 * 	(I) [x]
	 */
	if (f.id() == identity.id())
		return (identity);

	if (key.first <= apply_cache_high.first && key.second <= apply_cache_high.second) {
		it = apply_cache.find(key);
		if (it != apply_cache.end())
			return (it->second);
	}
	Ilerhiilel expr;
	if (f->type_ == ENumber) {
		uintmax_t i, j, n;

		n = f->number_->number();

		j = 1;
		for (i = 0; i < m; i++) {
			j *= n;
		}
		expr = number(Number::number(j));
	} else if (f->type_ == ECurriedNumber) {
		uintmax_t n;

		n = f->number_->number();

		expr = curried_number(number(Number::number(m * n)), f->expressions_.first);
	} else {
		expr = new Expression(xnumber->number_, f);
	}
	apply_cache[key] = expr;

	if (key.first > apply_cache_high.first) {
		apply_cache_high.first = key.first;
	}
	if (key.second > apply_cache_high.second) {
		apply_cache_high.second = key.second;
	}

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
	case Expression::ENumber:
		return (os << e.number_);
	case Expression::ECurriedNumber:
		return (os << '(' << e.number_ << ' ' << e.expressions_.first << ')');
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
	case Expression::EIdentity:
		return (os << "I");
	default:
		throw "Invalid type. (render)";
	}
}
