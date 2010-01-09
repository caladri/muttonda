#ifndef	CHURCH_H
#define	CHURCH_H

struct ChurchBuiltin {
	static std::wstring name(void)
	{
		return (L"church");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(expressions[0]);
		uintmax_t i = a->scalar().value();

		Ref<Expression> expr(Expression::name(L"x"));
		Ref<Expression> f(Expression::name(L"f"));
		while (i--) {
			expr = Expression::apply(f, expr);
		}
		expr = Expression::lambda(L"f", Expression::lambda(L"x", expr));

		return (expr);
	}
};

static struct : Builtin<ChurchBuiltin, 1> { } Church;

#endif /* !CHURCH_H */
