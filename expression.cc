#include <iostream>
#include <map>
#include <ostream>
#include <set>
#include <vector>

#include "expression.h"
#include "function.h"
#include "lambda.h"
#include "name.h"
#include "scalar.h"

/*
 * Do a variable renaming pass at some point.
 */

/*
 * We can do memoization on the cheap with some help from
 * Ref<>, but it doesn't work for things with side-effects
 * and while we can guess where those might be, it's hard
 * to make that information cascade up, so while we might
 * be able to force non-memoization of a single apply to
 * a function, we can't force non-memoization of the outer
 * apply it is inside.
 *
 * Performance-wise, though, it is pretty impressive.
 */

/*
 * Need to work towards iteration rather than recursion in
 * evaluation, simplification and binding.
 */

Ref<Expression>
Expression::bind(const Name& v, const Ref<Expression>& e) const
{
	static std::set<std::pair<unsigned, Name> > null_cache;
	static std::map<std::pair<unsigned, std::pair<Name, unsigned> >, Ref<Expression> > bind_cache;
	std::map<std::pair<unsigned, std::pair<Name, unsigned> >, Ref<Expression> >::const_iterator bcit;
	std::pair<Name, unsigned> binding(v, e.id());
	std::pair<unsigned, std::pair<Name, unsigned> > bind_key;
	std::pair<unsigned, Name> null_key;

	bind_key.second = binding;
	null_key.second = v;

	switch (type_) {
	case EVariable:
		if (name_ == v)
			return (e);
		return (Ref<Expression>());
	case EScalar:
	case EString:
		return (Ref<Expression>());
	case EApply: {
		Ref<Expression> a(expressions_.first);
		Ref<Expression> b(expressions_.second);

		null_key.first = a.id();
		if (null_cache.find(null_key) != null_cache.end()) {
			a = Ref<Expression>();
		} else {
			bind_key.first = a.id();
			bcit = bind_cache.find(bind_key);
			if (bcit == bind_cache.end()) {
				a = a->bind(v, e);

				if (a.null()) {
					null_cache.insert(null_key);
				} else {
					bind_cache[bind_key] = a;
				}
			} else {
				a = bcit->second;
			}
		}

		null_key.first = b.id();
		if (null_cache.find(null_key) != null_cache.end()) {
			b = Ref<Expression>();
		} else {
			bind_key.first = b.id();
			bcit = bind_cache.find(bind_key);
			if (bcit == bind_cache.end()) {
				b = b->bind(v, e);

				if (b.null()) {
					null_cache.insert(null_key);
				} else {
					bind_cache[bind_key] = b;
				}
			} else {
				b = bcit->second;
			}
		}

		if (a.null() && b.null())
			return (Ref<Expression>());

		if (a.null())
			a = expressions_.first;

		if (b.null())
			b = expressions_.second;

		return (apply(a, b));
	}
	case EFunction:
		return (function_->bind(v, e));
	default:
		throw "Invalid type. (bind)";
	}
}

Ref<Expression>
Expression::eval(bool memoize) const
{
	try {
		switch (type_) {
		case EVariable:
			throw "Unbound variable.";
		case EFunction:
		case EScalar:
		case EString:
			return (Ref<Expression>());
		case EApply: {
			static std::map<std::pair<unsigned, unsigned>, Ref<Expression> > memoized;
			std::map<std::pair<unsigned, unsigned>, Ref<Expression> >::const_iterator it;

			std::pair<unsigned, unsigned> ids(expressions_.first.id(),
							  expressions_.second.id());

			if (memoize) {
				it = memoized.find(ids);
				if (it != memoized.end())
					return (it->second);
			}

			Ref<Expression> expr(expressions_.first);
			Ref<Expression> evaluated = expr->eval(memoize);

			if (!evaluated.null())
				expr = evaluated;

			switch (expr->type_) {
			case EVariable:
				throw "Attempting to apply to free variable.";
			case EScalar:
				throw "Attempting to apply to scalar.";
			case EApply:
				if (evaluated.null()) {
					if (memoize) {
						memoized[ids] = Ref<Expression>();
					}
					return (Ref<Expression>());
				}
				expr = apply(expr, expressions_.second);

				if (memoize) {
					memoized[ids] = expr;

					ids.first = expr.id();
					memoized[ids] = Ref<Expression>();
				}

				return (expr);
			case EFunction:
				expr = expr->function_->apply(expressions_.second, memoize);

				if (memoize) {
					memoized[ids] = expr;
				}

				return (expr);
			case EString:
				throw "Attempting to apply to string.";
			default:
				throw "Invalid type. (apply)";
			}
			break;
		}
		default:
			throw "Invalid type. (eval)";
		}
	} catch(...) {
		std::wcerr << "From: " << *this << std::endl;
		throw;
	}
}

/*
 * This must not error out ever as it is called when there are still unbound
 * variables, and so we may be partially evaluating something catastrophic
 * which is not intended to work, or which must not be called until needed
 * (like an error routine in the false branch of a conditional.)
 *
 * Would be nice to detect unused variables and dead parameters and to not
 * waste any time on them (i.e. mark them dead, don't simplify, don't
 * evaluate, throw error if they are coerced.)
 */
Ref<Expression>
Expression::simplify(void) const
{
	switch (type_) {
	case EApply: {
		Ref<Expression> a(expressions_.first);
		Ref<Expression> b(expressions_.second);
		bool null_a = false, null_b = false;

		a = a->simplify();
		b = b->simplify();

		if (a.null()) {
			a = expressions_.first;
			null_a = true;
		}

		if (b.null()) {
			b = expressions_.second;
			null_b = true;
		}

		if (a->type_ == EFunction) {
			/* XXX If we do proper renaming, we can fold in variables like constants, too.  */
			bool constant;
			switch (b->type_) {
			case EScalar:
			case EString:
				constant = true;
				break;
			default:
				constant = false;
				break;
			}
			Ref<Expression> expr(a->function_->fold(b, constant));
			if (!expr.null())
				return (expr);
		}
		if (null_a && null_b)
			return (Ref<Expression>());
		return (apply(a, b));
	}
	case EFunction:
		return (function_->simplify());
	default:
		return (Ref<Expression>());
	}
}

Scalar
Expression::scalar(void) const
{
	if (type_ == EApply) {
		Ref<Expression> me = eval(true);
		if (!me.null())
			return (me->scalar());
	}
	switch (type_) {
	case EScalar:
		return (scalar_);
	default:
		throw "Expression is not scalar.";
	}
}

String
Expression::string(void) const
{
	if (type_ == EApply) {
		Ref<Expression> me = eval(true);
		if (!me.null())
			return (me->string());
	}
	switch (type_) {
	case EString:
		return (str_);
	default:
		throw "Expression is not a string.";
	}
}

Ref<Expression>
Expression::apply(const Ref<Expression>& a, const Ref<Expression>& b)
{
	static std::map<std::pair<unsigned, unsigned>, Ref<Expression> > cache;
	std::map<std::pair<unsigned, unsigned>, Ref<Expression> >::const_iterator it;
	std::pair<unsigned, unsigned> key(a.id(), b.id());

	it = cache.find(key);
	if (it != cache.end())
		return (it->second);
	Ref<Expression> expr(new Expression(a, b));
	cache[key] = expr;
	return (expr);
}

Ref<Expression>
Expression::lambda(const Name& name, const Ref<Expression>& body)
{
	static std::map<std::pair<Name, unsigned>, Ref<Expression> > cache;
	std::map<std::pair<Name, unsigned>, Ref<Expression> >::const_iterator it;
	std::pair<Name, unsigned> key(name, body.id());

	it = cache.find(key);
	if (it != cache.end())
		return (it->second);
	Ref<Expression> expr(new Expression(new Lambda(name, body)));
	cache[key] = expr;
	return (expr);
}

Ref<Expression>
Expression::name(const Name& n)
{
	static std::map<Name, Ref<Expression> > cache;
	std::map<Name, Ref<Expression> >::const_iterator it;

	it = cache.find(n);
	if (it != cache.end())
		return (it->second);
	Ref<Expression> expr(new Expression(n));
	cache[n] = expr;
	return (expr);
}

Ref<Expression>
Expression::scalar(const Scalar& s)
{
	static std::map<Scalar, Ref<Expression> > cache;
	std::map<Scalar, Ref<Expression> >::const_iterator it;

	it = cache.find(s);
	if (it != cache.end())
		return (it->second);
	Ref<Expression> expr(new Expression(s));
	cache[s] = expr;
	return (expr);
}

Ref<Expression>
Expression::string(const String& s)
{
	static std::map<String, Ref<Expression> > cache;
	std::map<String, Ref<Expression> >::const_iterator it;

	it = cache.find(s);
	if (it != cache.end())
		return (it->second);
	Ref<Expression> expr(new Expression(s));
	cache[s] = expr;
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
		if (e.expressions_.first->type_ == Expression::EFunction)
			os << '(' << e.expressions_.first << ')';
		else
			os << e.expressions_.first;
		os << ' ';
		if (e.expressions_.second->type_ == Expression::EApply ||
		    e.expressions_.second->type_ == Expression::EFunction)
			os << '(' << e.expressions_.second << ')';
		else
			os << e.expressions_.second;
		return (os);
	case Expression::EFunction:
		return (e.function_->print(os));
	case Expression::EString:
		return (os << e.string());
	default:
		throw "Invalid type. (render)";
	}
}
