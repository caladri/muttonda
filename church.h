#ifndef	CHURCH_H
#define	CHURCH_H

struct ChurchBuiltin {
	static std::string name(void)
	{
		return ("church");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(expressions[0]);
		unsigned i = Expression::scalar(a).value();

		Ref<Expression> expr(new Expression(Name("x")));
		Ref<Expression> f(new Expression(Name("f")));
		while (i--) {
			expr = new Expression(f, expr);
		}
		return (new Expression(Lambda("f", new Expression(Lambda("x", expr)))));
	}
};

static struct : Builtin<ChurchBuiltin, 1> { } Church;

#endif /* !CHURCH_H */
