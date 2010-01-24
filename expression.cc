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

/*
 * XXX
 * Not sure if ELet handling is safe wrt name binding.  Pretty
 * sure it's wrong, actually!
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
	};
};

static Ref<Name> unused_name(Name::name(L"_"));

/*
 * This needs to iterate rather than recurse.
 */
Ref<Expression>
Expression::bind(const Ref<Name>& v, const Ref<Expression>& e) const
{
	if (free_.find(v.id()) == free_.end())
		return (Ref<Expression>());

	static std::tr1::unordered_map<std::pair<unsigned, std::pair<unsigned, unsigned> >, Ref<Expression> > bind_cache;
	std::tr1::unordered_map<std::pair<unsigned, std::pair<unsigned, unsigned> >, Ref<Expression> >::const_iterator bcit;
	std::pair<unsigned, unsigned> binding(v.id(), e.id());
	std::pair<unsigned, std::pair<unsigned, unsigned> > bind_key;

	bind_key.second = binding;

	switch (type_) {
	case EVariable:
		return (e);
	case EScalar:
	case EString:
	case EFunction:
		return (Ref<Expression>());
	case EApply: {
		Ref<Expression> a(expressions_.first);
		Ref<Expression> b(expressions_.second);

		if (a->free_.find(v.id()) == a->free_.end()) {
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

		if (b->free_.find(v.id()) == b->free_.end()) {
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
			return (Ref<Expression>());

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

		if (a->free_.find(v.id()) == a->free_.end()) {
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
			return (Ref<Expression>());
		
		return (lambda(name_, a));
	}
	case ELet: {
		Ref<Expression> a(expressions_.first);
		Ref<Expression> b(expressions_.second);

		if (e->type_ == EVariable && e->name_.id() == name_.id())
			throw "Name capture.  (Let.)";

		if (a->free_.find(v.id()) == a->free_.end()) {
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
			if (b->free_.find(v.id()) == b->free_.end()) {
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
			return (Ref<Expression>());

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
 * This needs to iterate rather than recurse.
 */
Ref<Expression>
Expression::eval(bool memoize) const
{
	try {
		switch (type_) {
		case EVariable:
			throw "Unbound variable.";
		case ELambda:
		case EFunction:
		case EScalar:
		case EString:
			return (Ref<Expression>());
		case EApply: {
			static std::tr1::unordered_map<std::pair<unsigned, unsigned>, Ref<Expression> > memoized;
			std::tr1::unordered_map<std::pair<unsigned, unsigned>, Ref<Expression> >::const_iterator it;

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
			case ELambda: {
				Ref<Expression> bound(expr->expressions_.first->bind(expr->name_, expressions_.second));
				if (bound.null()) {
					expr = expr->expressions_.first;
				} else {
					expr = bound;
				}

				Ref<Expression> evaluated(expr->eval(memoize));
				if (!evaluated.null()) {
					expr = evaluated;
				}

				if (memoize) {
					memoized[ids] = expr;
				}

				return (expr);
			}
			case EString:
				throw "Attempting to apply to string.";
			case ELet:
				throw "Attempting to apply to unreduced let.";
			default:
				throw "Invalid type. (apply)";
			}
			break;
		}
		case ELet: {
			Ref<Expression> val(expressions_.first);
			Ref<Expression> body(expressions_.second);

			if (val->type_ == EVariable)
				throw "Cowardly refusing to let a variable to a free variable.";

			Ref<Expression> expr(body->bind(name_, val));
			if (expr.null()) {
				expr = body->eval(memoize);
				if (expr.null())
					return (body);
				return (expr);
			}

			Ref<Expression> evaluated(expr->eval(memoize));
			if (evaluated.null())
				return (expr);
			return (evaluated);
		} 
		default:
			throw "Invalid type. (eval)";
		}
	} catch(...) {
		std::wcerr << "From: " << *this << std::endl;
		throw;
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

Scalar
Expression::scalar(void) const
{
	if (type_ == EApply || type_ == ELet) {
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
	if (type_ == EApply || type_ == ELet) {
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
	static std::tr1::unordered_map<std::pair<unsigned, unsigned>, Ref<Expression> > cache;
	static std::pair<unsigned, unsigned> high;
	std::tr1::unordered_map<std::pair<unsigned, unsigned>, Ref<Expression> >::const_iterator it;
	std::pair<unsigned, unsigned> key(a.id(), b.id());

	if (a->type_ == ELambda) {
		return (let(a->name_, b, a->expressions_.first));
	}

	if (key.first <= high.first && key.second <= high.second) {
		it = cache.find(key);
		if (it != cache.end())
			return (it->second);
	}
	Ref<Expression> expr(new Expression(a, b));
	cache[key] = expr;

	if (key.first > high.first) {
		high.first = key.first;
	}
	if (key.second > high.second) {
		high.second = key.second;
	}

	return (expr);
}

Ref<Expression>
Expression::lambda(const Ref<Name>& name, const Ref<Expression>& body)
{
	static std::tr1::unordered_map<std::pair<unsigned, unsigned>, Ref<Expression> > cache;
	std::tr1::unordered_map<std::pair<unsigned, unsigned>, Ref<Expression> >::const_iterator it;
	std::pair<unsigned, unsigned> key(name.id(), body.id());

	if (name.id() != unused_name.id() &&
	    body->free_.find(name.id()) == body->free_.end())
		return (lambda(unused_name, body));

	it = cache.find(key);
	if (it != cache.end())
		return (it->second);
	Ref<Expression> expr(new Expression(name, body));
	cache[key] = expr;
	return (expr);
}

Ref<Expression>
Expression::let(const Ref<Name>& name, const Ref<Expression>& a, const Ref<Expression>& b)
{
	static std::tr1::unordered_map<std::pair<unsigned, std::pair<unsigned, unsigned> >, Ref<Expression> > cache;
	std::tr1::unordered_map<std::pair<unsigned, std::pair<unsigned, unsigned> >, Ref<Expression> >::const_iterator it;
	std::pair<unsigned, std::pair<unsigned, unsigned> > key(name.id(), std::pair<unsigned, unsigned>(a.id(), b.id()));

	if (a->type_ == EVariable && a->name_.id() == name.id())
		return (b);

	it = cache.find(key);
	if (it != cache.end())
		return (it->second);

	Ref<Expression> expr;

	if (b->free_.find(name.id()) == b->free_.end()) {
		expr = b;
	} else if (a->type_ == EScalar || a->type_ == EFunction || a->type_ == EString) {
		expr = b->bind(name, a);
		if (expr.null())
			expr = b;
	} else {
		expr = new Expression(name, a, b);
	}
	cache[key] = expr;
	return (expr);
}

Ref<Expression>
Expression::name(const Ref<Name>& n)
{
	static std::tr1::unordered_map<unsigned, Ref<Expression> > cache;
	std::tr1::unordered_map<unsigned, Ref<Expression> >::const_iterator it;

	it = cache.find(n.id());
	if (it != cache.end())
		return (it->second);
	Ref<Expression> expr(new Expression(n));
	cache[n.id()] = expr;
	return (expr);
}

Ref<Expression>
Expression::scalar(const Scalar& s)
{
	static std::tr1::unordered_map<Scalar, Ref<Expression> > cache;
	std::tr1::unordered_map<Scalar, Ref<Expression> >::const_iterator it;

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
	static std::tr1::unordered_map<String, Ref<Expression> > cache;
	std::tr1::unordered_map<String, Ref<Expression> >::const_iterator it;

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
