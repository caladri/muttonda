#include <iostream>
#include <map>
#include <ostream>
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

Expression::Expression(const Name& v)
: type_(EVariable),
  name_(v),
  scalar_(),
  expressions_(),
  str_(),
  function_(NULL)
{ }

Expression::Expression(const Scalar& v)
: type_(EScalar),
  name_(),
  scalar_(v),
  expressions_(),
  str_(),
  function_(NULL)
{ }

Expression::Expression(const Ref<Expression>& a, const Ref<Expression>& b)
: type_(EApply),
  name_(),
  scalar_(),
  expressions_(),
  str_(),
  function_(NULL)
{
	expressions_.push_back(a);
	expressions_.push_back(b);
}

Expression::Expression(const String& str)
: type_(EString),
  name_(),
  scalar_(),
  expressions_(),
  str_(str),
  function_(NULL)
{ }

Expression::Expression(const Function& f)
: type_(EFunction),
  name_(),
  scalar_(),
  expressions_(),
  str_(),
  function_(f.clone())
{ }

Expression::~Expression()
{
	if (function_ != NULL) {
		delete function_;
		function_ = NULL;
	}
}

Ref<Expression>
Expression::bind(const Name& v, const Ref<Expression>& e) const
{
	switch (type_) {
	case EVariable:
		if (name_ == v)
			return (e);
		return (Ref<Expression>());
	case EScalar:
	case EString:
		return (Ref<Expression>());
	case EApply: {
		static std::map<std::pair<unsigned, unsigned>, Ref<Expression> > apply_cache;

		Ref<Expression> a(expressions_[0]);
		Ref<Expression> b(expressions_[1]);

		a = a->bind(v, e);
		b = b->bind(v, e);

		if (a.null() && b.null())
			return (Ref<Expression>());

		if (a.null())
			a = expressions_[0];

		if (b.null())
			b = expressions_[1];

		std::pair<unsigned, unsigned> ids(a.id(), b.id());
		std::map<std::pair<unsigned, unsigned>, Ref<Expression> >::const_iterator it;

		it = apply_cache.find(ids);
		if (it != apply_cache.end())
			return (it->second);

		Ref<Expression> expr(new Expression(a, b));

		apply_cache[ids] = expr;

		return (expr);
	}
	case EFunction:
		return (function_->bind(v, e));
	default:
		throw "Invalid type. (bind)";
	}
}

Ref<Expression>
Expression::eval(void) const
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
#ifdef MEMOIZE
			static std::map<std::pair<unsigned, unsigned>, Ref<Expression> > memoized;

			std::pair<unsigned, unsigned> ids(expressions_[0].id(),
							  expressions_[1].id());
			std::map<std::pair<unsigned, unsigned>, Ref<Expression> >::const_iterator it;

			it = memoized.find(ids);
			if (it != memoized.end())
				return (it->second);
#endif

			Ref<Expression> expr(expressions_[0]);
			Ref<Expression> evaluated = expr->eval();

			if (!evaluated.null())
				expr = evaluated;

			switch (expr->type_) {
			case EVariable:
				throw "Attempting to apply to free variable.";
			case EScalar:
				throw "Attempting to apply to scalar.";
			case EApply:
#ifdef MEMOIZE
				if (evaluated.null()) {
					memoized[ids] = Ref<Expression>();
					return (Ref<Expression>());
				}
#endif
				expr = new Expression(expr, expressions_[1]);

#ifdef MEMOIZE
				memoized[ids] = expr;

				ids = std::pair<unsigned, unsigned>(expr.id(), expressions_[1].id());
				memoized[ids] = Ref<Expression>();
#endif

				return (expr);
			case EFunction:
				expr = expr->function_->apply(expressions_[1]);

#ifdef MEMOIZE
				memoized[ids] = expr;
#endif

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
		std::cerr << "From: " << *this << std::endl;
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
		Ref<Expression> a(expressions_[0]);
		Ref<Expression> b(expressions_[1]);

		a = a->simplify();
		b = b->simplify();

		if (a.null() && b.null())
			return (Ref<Expression>());

		if (a.null())
			a = expressions_[0];

		if (b.null())
			b = expressions_[1];

		if (a->type_ == EFunction) {
			switch (b->type_) {
			/* XXX If we do proper renaming, we can fold in variables, too.  */
			case EScalar:
			case EString: {
				Ref<Expression> expr(a->function_->fold(b));
				if (expr.null())
					break;
				return (expr);
			}
			default:
				break;
			}
		}
		return (new Expression(a, b));
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
		Ref<Expression> me = eval();
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
		Ref<Expression> me = eval();
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

std::ostream&
operator<< (std::ostream& os, const Ref<Expression>& e)
{
	if (e.null())
		throw "Cowardly refusing to print a null Expression.";
	return (os << *e);
}

std::ostream&
operator<< (std::ostream& os, const Expression& e)
{
	switch (e.type_) {
	case Expression::EVariable:
		return (os << e.name_);
	case Expression::EScalar:
		return (os << e.scalar());
	case Expression::EApply:
		if (e.expressions_[0]->type_ == Expression::EFunction)
			os << '(' << e.expressions_[0] << ')';
		else
			os << e.expressions_[0];
		os << ' ';
		if (e.expressions_[1]->type_ == Expression::EApply ||
		    e.expressions_[1]->type_ == Expression::EFunction)
			os << '(' << e.expressions_[1] << ')';
		else
			os << e.expressions_[1];
		return (os);
	case Expression::EFunction:
		return (e.function_->print(os));
	case Expression::EString:
		return (os << e.string());
	default:
		throw "Invalid type. (render)";
	}
}
