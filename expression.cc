#include <iostream>
#include <map>
#include <ostream>
#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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

typedef std::pair<Ilerhiilel::id_t, Ilerhiilel::id_t> expr_pair_t;

static Expression::expr_map<expr_pair_t> eval_cache;

static Expression::expr_map<expr_pair_t> apply_cache;
static Expression::expr_map<Funkts::id_t> function_cache;
static Expression::expr_map<std::pair<Ner::id_t, expr_pair_t> > let_cache;
static Expression::expr_map<Ner::id_t> name_cache;
static Expression::expr_map<Too::id_t> number_cache;
static Expression::expr_map<String> string_cache;

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
			a = a.meta()->bind_cache(v, e);
		}

		if (b->free_.find(v) == b->free_.end()) {
			b = Ilerhiilel();
		} else {
			b = b.meta()->bind_cache(v, e);
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
			a = a.meta()->bind_cache(v, e);
		}

		if (a.null())
			throw "Lambda had free variable but its body did not.";

		return (lambda(name_, a));
	}
	case ECurriedNumber: {
		if (e->type_ == EVariable && e->name_.id() == name_.id())
			throw "Name capture.  (Curried number.)";

		Ilerhiilel a(expressions_.first);

		if (a->free_.find(v) == a->free_.end()) {
			a = Ilerhiilel();
		} else {
			a = a.meta()->bind_cache(v, e);
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
	std::stack<expr_pair_t> apply_stack;
	std::stack<Ilerhiilel> right_stack;
	expr_pair_t ids;
	Ilerhiilel expr;
	bool reduced;
	uintmax_t n, k;

	switch (type_) {
	case EVariable:
		Debugger::instance()->set(Expression::name(name_));
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
		right_stack.push(expressions_.second);
		ids.first = expressions_.first.id();
		ids.second = expressions_.second.id();
		if (memoize)
			apply_stack.push(ids);
		reduced = false;
		break;
	default:
		throw "Invalid type.";
	}

	for (;;) {
		if (expr.null())
			throw "Null reference in reduction pass.";

		if (memoize && !right_stack.empty()) {
			ids = apply_stack.top();

			it = eval_cache.find(ids);
			if (it == eval_cache.end()) {
				ids.first = expr.id();
				ids.second = right_stack.top().id();

				it = eval_cache.find(ids);
			}

			if (it != eval_cache.end()) {
				expr = it->second;

				right_stack.pop();
				apply_stack.pop();
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

				apply_stack.push(ids);
			}
			right_stack.push(expr->expressions_.second);
			expr = expr->expressions_.first;
			continue;
		default:
			throw "Invalid type.";
		}

		if (right_stack.empty()) {
			if (reduced)
				return (expr);
			return (Ilerhiilel());
		}

		/*
		 * Rewrite the application if it's not to a builtin,
		 * since they do nasty things with argument capture.
		 */
		if (expr->type_ != EFunction) {
			Ilerhiilel b = right_stack.top();
			if (expr->type_ != ELambda && b->type_ == EVariable)
				throw "Application of free variable during evaluation.";

			Ilerhiilel rexpr = apply(expr, b);
			if (rexpr->type_ != EApply ||
			    rexpr->expressions_.first.id() != expr.id() ||
			    rexpr->expressions_.second.id() != b.id()) {
				/*
				 * The rewritten expression was different.
				 */
				expr = rexpr;

				if (memoize) {
					eval_cache[ids] = expr;

					ids = apply_stack.top();
					apply_stack.pop();

					eval_cache[ids] = expr;
				}

				right_stack.pop();
				reduced = true;
				continue;
			}
		}

		/*
		 * Application.
		 */
		switch (expr->type_) {
		case EVariable:
			throw "Somehow a free variable slipped by.";
		case ELambda:
			if (expr->name_.id() == unused_name.id())
				throw "Application to lambda _ parameter should have been rewritten.";

			expr = expr->expressions_.first->bind(expr->name_, right_stack.top());
			if (expr.null())
				throw "Apply to non-_ parameter must not be null.";

			if (memoize) {
				eval_cache[ids] = expr;

				ids = apply_stack.top();
				apply_stack.pop();

				eval_cache[ids] = expr;
			}

			right_stack.pop();
			reduced = true;
			continue;
		case EFunction:
			expr = expr->function_->apply(right_stack.top(), memoize);
			if (expr.null())
				throw "Builtin function must not return null.";

			if (memoize) {
				eval_cache[ids] = expr;

				ids = apply_stack.top();
				apply_stack.pop();

				eval_cache[ids] = expr;
			}

			right_stack.pop();
			reduced = true;
			continue;
		case ENumber:
			throw "Somehow a number went without curry.";
		case ECurriedNumber:
			k = expr->number_->number();
			if (k == 0 || k == 1)
				throw "Somehow an application to 0 or one was not rewritten.";

			if (k != 0) {
				Ilerhiilel f = expr->expressions_.first;
				expr = right_stack.top();

				for (n = 0; n < k; n++) {
					expr = apply(f, expr);
					Ilerhiilel t = expr->eval(memoize);
					if (!t.null())
						expr = t;
				}
			} else {
				expr = right_stack.top();
			}

			if (memoize) {
				eval_cache[ids] = expr;

				ids = apply_stack.top();
				apply_stack.pop();

				eval_cache[ids] = expr;
			}

			right_stack.pop();
			reduced = true;
			continue;
		case EIdentity:
			throw "Somehow application to an identity was not rewritten.";
		case EString:
			Debugger::instance()->set(expr);
			throw "Refusing to apply to string.";
		case EApply:
			Debugger::instance()->set(expr);
			throw "Somehow an application slipped by.";
		default:
			throw "Invalid type.";
		}
	}
}

Ner
Expression::name(void) const
{
	if (type_ == EApply) {
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
	if (type_ == EApply) {
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
	if (type_ == EApply) {
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

bool
Expression::match(const Ilerhiilel& expr, const char *patt)
{
	size_t matched = match(expr, patt, std::map<char, Ner>());
	if (matched < strlen(patt))
		return (false);
	return (true);
}

/*
 * This doesn't have some error handling that'd be useful, mostly
 * invovling malformed patterns.
 *
 * Returns the number of characters of pattern consumed, or 0 if
 * no match.
 */
size_t
Expression::match(const Ilerhiilel& expr, const char *patt, const std::map<char, Ner>& penv)
{
	if (*patt == '\0')
		throw "Premature end of pattern?";
	if (*patt == '*') /* Wildcard.  */
		return (1);
	switch (expr->type_) {
	case ELambda:
		if (patt[0] == 'L') {
			if (penv.find(patt[1]) != penv.end())
				return (0); /* XXX */
			std::map<char, Ner> cenv(penv);
			if (patt[1] != '_')
				cenv[patt[1]] = expr->name_;
			size_t matched = match(expr->expressions_.first, patt + 2, cenv);
			if (matched == 0)
				return (0);
			return (2 + matched);
		}
		return (0);
	case EApply:
		if (patt[0] == 'A') {
			size_t matched1 = match(expr->expressions_.first, patt + 1, penv);
			if (matched1 == 0)
				return (0);
			size_t matched2 = match(expr->expressions_.second, patt + 1 + matched1, penv);
			if (matched2 == 0)
				return (0);
			return (1 + matched1 + matched2);
		}
		return (0);
	case ECurriedNumber:
		if (patt[0] == 'C') {
			size_t matched = match(expr->expressions_.first, patt + 1, penv);
			if (matched == 0)
				return (0);
			return (1 + matched);
		}
		return (0);
	case EVariable:
		if (patt[0] >= 'a' && patt[0] <= 'z') {
			std::map<char, Ner>::const_iterator it;
			it = penv.find(patt[0]);
			if (it == penv.end())
				return (0); /* XXX */
			if (expr->name_.id() != it->second.id())
				return (0);
			return (1);
		}
		return (0);
	case EIdentity:
		if (patt[0] == 'I')
			return (1);
		return (0);
	default:
		return (0);
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
	 * This is special-cased as ::curried_number().
	 */
	if (a->type_ == ENumber)
		return (curried_number(a, b));

	if (a->type_ == ELambda) {
		const Ner& name = a->name_;
		const Ilerhiilel& body = a->expressions_.first;

		/*
		 * Turns:
		 * 	(\_ -> b) a
		 * Into:
		 * 	b
		 */
		if (name.id() == unused_name.id())
			return (body);

		/*
		 * Turns:
		 * 	(\a -> f a) a
		 * Into:
		 * 	f a
		 */
		if (b->type_ == EVariable && b->name_.id() == name.id())
			return (body);

		/*
		 * Turns:
		 * 	(\n f x -> 4 f (n f x)) 3
		 * Into:
		 * 	7
		 */
		if (b->type_ == ENumber && match(a, "LnLfLxACfAAnfx"))
			return (number(Number::number(body->expressions_.first->expressions_.first->expressions_.first->number_->number() + b->number_->number())));

		/*
		 * Turns:
		 * 	(\n f x -> f (n f x)) 4
		 * Into:
		 * 	5
		 */
		if (b->type_ == ENumber && match(a, "LnLfLxAfAAnfx"))
			return (number(Number::number(1 + b->number_->number())));

		/*
		 * Turns:
		 * 	(\n f x -> n f x) 3
		 * Into:
		 * 	3
		 */
		if (b->type_ == ENumber && match(a, "LnLfLxAAnfx"))
			return (b);

		/*
		 * Turns:
		 * 	(\n f x -> n (\g h -> h (g f)) (\_ -> x) I) 9
		 * Into:
		 * 	8
		 */
		if (b->type_ == ENumber && b->number_->number() > 0 &&
		    match(a, "LnLfLxAAAnLgLhAhAgfL_xI")) {
			return (number(Number::number(b->number_->number() - 1)));
		}

		/*
		 * Turns:
		 * 	(\n -> n (\_ -> f) t) 0
		 * Into:
		 * 	t
		 *
		 * Turns:
		 * 	(\n -> n (\_ -> f) t) 2
		 * Into:
		 * 	f
		 */
		if (b->type_ == ENumber && match(a, "LnAAnL_**")) {
			if (b->number_->number() == 0) {
				return (body->expressions_.second);
			} else {
				return (body->expressions_.first->expressions_.second->expressions_.first);
			}
		}

		/*
		 * Constant propagation.
		 */
		switch (b->type_) {
		case ECurriedNumber:
			/*
			 * A curried number with no free variables is a constant and
			 * can be propagated.
			 */
			if (!b->free_.empty())
				break;
		case ENumber:
		case EFunction:
		case EString:
		case EIdentity:
			return (body->bind(name, b));
		default:
			break;
		}

		/*
		 * Turns:
		 * 	(\f -> f z) a
		 * Into:
		 * 	a z
		 *
		 * Likewise:
		 * 	(\f -> z f) a
		 * Into:
		 * 	a z
		 *
		 * Likewise:
		 * 	(\f -> f f) a
		 * Into:
		 * 	a a
		 *
		 * XXX This could be a general bind/rename,
		 *     if and only if we could ensure that a
		 *     was free everywhere f is used.
		 */
		if (body->type_ == EApply && b->type_ != EApply && b->type_ != ELambda) {
			Ilerhiilel l, r;

			l = body->expressions_.first;
			r = body->expressions_.second;

			if (l->type_ == EVariable) {
				if (r->type_ == EVariable) {
					if (l->name_.id() == name.id() && r->name_.id() == name.id())
						return (apply(b, b));
				}
				if (l->name_.id() == name.id() && !r->free(name))
					return (apply(b, r));
			} else if (r->type_ == EVariable) {
				if (r->name_.id() == name.id() && !l->free(name))
					return (apply(l, b));
			}
		}
	}

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
		std::map<char, Ner> env;
		const char *pattern = "LxACfx";
		env['f'] = name;
		if (match(body, pattern, env) == strlen(pattern)) {
			/*
			 * This turns:
			 * 	\f x -> 4 f x
			 * Into:
			 * 	4
			 */
			return (number(body->expressions_.first->expressions_.first->number_));
		}
		/*
		 * Eta-reduction is pointless.
		 */
#if 0
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
#endif
	}

	return (body.meta()->lambda_cache(name, body));
}

Ilerhiilel
Expression::let(const Ner& name, const Ilerhiilel& a, const Ilerhiilel& b)
{
	return (apply(lambda(name, b), a));
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
		return (identity);
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

	/*
	 * Turns:
	 * 	(number) (\_ a -> a foo)
	 * Into:
	 * 	\_ a -> a foo
	 */
	if (f->type_ == ELambda && f->name_.id() == unused_name.id())
		return (f);

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
		return (os << e.number_ << ' ' << e.expressions_.first);
	case Expression::EApply:
		if (e.expressions_.first->type_ == Expression::EFunction ||
		    e.expressions_.first->type_ == Expression::ELambda)
			os << '(' << e.expressions_.first << ')';
		else
			os << e.expressions_.first;
		os << ' ';
		if (e.expressions_.second->type_ == Expression::EApply ||
		    e.expressions_.second->type_ == Expression::EFunction ||
		    e.expressions_.second->type_ == Expression::ELambda ||
		    e.expressions_.second->type_ == Expression::ECurriedNumber)
			os << '(' << e.expressions_.second << ')';
		else
			os << e.expressions_.second;
		return (os);
	case Expression::EFunction:
		return (e.function_->print(os));
	case Expression::ELambda: {
#if 1 /* Experimenting with some odd ways of printing things.  */
		if (e.name_.id() == unused_name.id() &&
		    e.expressions_.first->type_ == Expression::EIdentity) {
			os << "F";
			return (os);
		}
		if (e.expressions_.first->type_ == Expression::ELambda &&
		    e.expressions_.first->expressions_.first->type_ == Expression::EVariable &&
		    e.expressions_.first->expressions_.first->name_.id() == e.name_.id()) {
			os << "T";
			return (os);
		}
		if (e.expressions_.first->type_ == Expression::EApply &&
		    e.expressions_.first->expressions_.first->type_ == Expression::EApply &&
		    e.expressions_.first->expressions_.first->expressions_.first->type_ == Expression::EVariable &&
		    e.expressions_.first->expressions_.first->expressions_.first->name_.id() == e.name_.id() &&
		    !e.expressions_.first->expressions_.first->expressions_.second->free(e.name_) &&
		    !e.expressions_.first->expressions_.second->free(e.name_)) {
			if (e.expressions_.first->expressions_.first->expressions_.second->type_ == Expression::EFunction ||
			    e.expressions_.first->expressions_.first->expressions_.second->type_ == Expression::ELambda)
				os << '(' << e.expressions_.first->expressions_.first->expressions_.second << ')';
			else
				os << e.expressions_.first->expressions_.first->expressions_.second;
			os << ',';
			if (e.expressions_.first->expressions_.second->type_ == Expression::EApply ||
			    e.expressions_.first->expressions_.second->type_ == Expression::EFunction ||
			    e.expressions_.first->expressions_.second->type_ == Expression::ELambda ||
			    e.expressions_.first->expressions_.second->type_ == Expression::ECurriedNumber)
				os << '(' << e.expressions_.first->expressions_.second << ')';
			else
				os << e.expressions_.first->expressions_.second;
			return (os);
		}
#endif

		os << "\\" << e.name_;

		Ilerhiilel next(e.expressions_.first);
		while (next->type_ == Expression::ELambda) {
			os << " " << next->name_;
			next = next->expressions_.first;
		}

		os << " -> " << next;

		return (os);
	}
	case Expression::EString:
		return (os << e.string());
	case Expression::EIdentity:
		return (os << "I");
	default:
		throw "Invalid type. (render)";
	}
}
