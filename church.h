#ifndef	CHURCH_H
#define	CHURCH_H

struct ChurchBuiltin {
	static std::wstring name(void)
	{
		return (L"church");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		static std::map<unsigned, Ref<Expression> > expr_map, scalar_map;
		std::map<unsigned, Ref<Expression> >::const_iterator it;

		Ref<Expression> a(expressions[0]);
		it = expr_map.find(a.id());
		if (it != expr_map.end()) {
			return (it->second);
		}

		uintmax_t i = a->scalar().value();
		it = scalar_map.find(i);
		if (it != scalar_map.end()) {
			return (it->second);
		}

		Ref<Expression> expr(Expression::name(L"x"));
		Ref<Expression> f(Expression::name(L"f"));
		while (i--) {
			expr = Expression::apply(f, expr);
		}
		expr = Expression::lambda(L"f", Expression::lambda(L"x", expr));

		expr_map[a.id()] = expr;
		scalar_map[i] = expr;

		return (expr);
	}
};

static struct : Builtin<ChurchBuiltin, 1> { } Church;

#endif /* !CHURCH_H */
