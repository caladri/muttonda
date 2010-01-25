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
	if (free_.find(v) == free_.end())
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
			return (Ref<Expression>());
		
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
 * XXX
 * Put this all in a try block and dump the expression context in catch.
 */
Ref<Expression>
Expression::eval(bool memoize) const
{
	std::vector<Ref<Expression> > right_queue;
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

	static std::tr1::unordered_map<std::pair<unsigned, unsigned>, Ref<Expression> > memoized;
	std::tr1::unordered_map<std::pair<unsigned, unsigned>, Ref<Expression> >::const_iterator it;
	std::pair<unsigned, unsigned> ids;

	for (;;) {
		if (expr.null())
			throw "Null reference in reduction pass.";

		if (memoize && !right_queue.empty()) {
			ids.first = expr.id();
			ids.second = right_queue.back().id();

			it = memoized.find(ids);
			if (it != memoized.end()) {
				right_queue.pop_back();
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
			right_queue.push_back(expr->expressions_.second);
			expr = expr->expressions_.first;
			continue;
		case ELet:
			expr = expr->expressions_.second->bind(expr->name_, expr->expressions_.first);
			if (expr.null())
				throw "Failed to reduce let.";
			reduced_ = true;

			if (!right_queue.empty()) {
				expr = apply(expr, right_queue.back());
				right_queue.pop_back();
			}
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
					memoized[ids] = expr;
				}

				right_queue.pop_back();
				reduced_ = true;

				if (!right_queue.empty()) {
					expr = apply(expr, right_queue.back());
					right_queue.pop_back();
				}
				continue;
			}

			expr = expr->expressions_.first->bind(expr->name_, right_queue.back());
			if (expr.null())
				throw "Apply to non-_ parameter must not be null.";

			if (memoize) {
				memoized[ids] = expr;
			}

			right_queue.pop_back();
			reduced_ = true;

			if (!right_queue.empty()) {
				expr = apply(expr, right_queue.back());
				right_queue.pop_back();
			}
			continue;
		case EFunction:
			expr = expr->function_->apply(right_queue.back(), memoize);
			if (expr.null())
				throw "Builtin function must not return null.";

			if (memoize) {
				memoized[ids] = expr;
			}

			right_queue.pop_back();
			reduced_ = true;

			if (!right_queue.empty()) {
				expr = apply(expr, right_queue.back());
				right_queue.pop_back();
			}
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
	    body->free_.find(name) == body->free_.end())
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

	if (b->type_ == EVariable && b->name_.id() == name.id())
		return (a);

	if (a->type_ == EVariable && a->name_.id() == name.id())
		return (b);

	it = cache.find(key);
	if (it != cache.end())
		return (it->second);

	Ref<Expression> expr;

	if (b->free_.find(name) == b->free_.end()) {
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
