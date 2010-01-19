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

/*
 * This needs to iterate rather than recurse.
 */
Ref<Expression>
Expression::bind(const Ref<Name>& v, const Ref<Expression>& e) const
{
	if (v.id() == Name::name(L"_").id()) /* Ew.  */
		return (Ref<Expression>());

	if (e->type_ == EVariable && e->name_.id() == v.id())
		return (Ref<Expression>());

	static std::tr1::unordered_set<std::pair<unsigned, unsigned> > null_cache;
	static std::tr1::unordered_map<std::pair<unsigned, std::pair<unsigned, unsigned> >, Ref<Expression> > bind_cache;
	std::tr1::unordered_map<std::pair<unsigned, std::pair<unsigned, unsigned> >, Ref<Expression> >::const_iterator bcit;
	std::pair<unsigned, unsigned> binding(v.id(), e.id());
	std::pair<unsigned, std::pair<unsigned, unsigned> > bind_key;
	std::pair<unsigned, unsigned> null_key;

	bind_key.second = binding;
	null_key.second = v.id();

	switch (type_) {
	case EVariable:
		if (name_.id() == v.id())
			return (e);
		if (e->type_ == EVariable && e->name_.id() == name_.id())
			throw "Name capture.  (Variable.)";
		return (Ref<Expression>());
	case EScalar:
	case EString:
	case EFunction:
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
	case ELambda: {
		if (name_.id() == v.id())
			return (Ref<Expression>());
		if (e->type_ == EVariable && e->name_.id() == name_.id())
			throw "Name capture.  (Lambda.)";

		Ref<Expression> a(expressions_.first);

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

		if (a.null())
			return (Ref<Expression>());
		
		return (lambda(name_, a));
	}
	case ELet: {
		Ref<Expression> a(expressions_.first);

		if (e->type_ == EVariable && e->name_.id() == name_.id())
			throw "Name capture.  (Let.)";

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

		Ref<Expression> b;
		
		if (name_.id() != v.id()) {
			b = expressions_.second;

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
/*
 * This may need to iterate rather than recurse.
 */
Ref<Expression>
Expression::simplify(void) const
{
	switch (type_) {
	case ELet:
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

		switch (type_) {
		case EApply:
			if (a->type_ == ELambda)
				throw "Misspelled let expression.";
			if (null_a && null_b)
				return (Ref<Expression>());
			return (apply(a, b));
		case ELet: {
			/* XXX If we do proper renaming, we can bind variables safely.  We do not prevent name capture properly here.  */
			Ref<Expression> expr(a->bind(name_, b));
			if (expr.null()) {
				if (null_a && null_b)
					return (Ref<Expression>());
				expr = let(name_, a, b);
			}
			return (expr);
		}
		default:
			   throw "Consistency is all I ask.";
		}
	}
	default:
		return (Ref<Expression>());
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

	if (name.id() == Name::name(L"_").id()) /* Ew.  */
		return (b);

	if (a->type_ == EVariable && a->name_.id() == name.id())
		return (b);

	it = cache.find(key);
	if (it != cache.end())
		return (it->second);
	Ref<Expression> expr(new Expression(name, a, b));
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
