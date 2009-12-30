#ifndef	CHURCH_H
#define	CHURCH_H

#include <map>

struct ChurchBuiltin {
	static std::string name(void)
	{
		return ("church");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		static std::map<uintmax_t, Ref<Expression> > memoized;
		std::map<uintmax_t, Ref<Expression> >::const_iterator it;
		Ref<Expression> a(expressions[0]);
		uintmax_t i = a->scalar().value();


		it = memoized.find(i);
		if (it != memoized.end())
			return (it->second);

		Ref<Expression> expr(new Expression(Name("x")));
		Ref<Expression> f(new Expression(Name("f")));
		while (i--) {
			expr = new Expression(f, expr);
		}
		expr = new Expression(Lambda("f", new Expression(Lambda("x", expr))));

		memoized[i] = expr;

		return (expr);
	}
};

static struct : Builtin<ChurchBuiltin, 1> { } Church;

#endif /* !CHURCH_H */
