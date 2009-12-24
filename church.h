#ifndef	CHURCH_H
#define	CHURCH_H

struct ChurchBuiltin {
	static std::string name(void)
	{
		return ("church");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(Expression::eval(expressions[0]));
		unsigned i = a->scalar().value();

		Ref<Expression> expr(new Expression(Name("x")));
		while (i--) {
			expr = new Expression(new Expression(Name("f")), expr);
		}
		return (new Expression(Lambda("f", new Expression(Lambda("x", expr)))));
	}
};

static struct _Church : Builtin<ChurchBuiltin, 1> {
#if 0
	Function *clone(void) const
	{
		return (new _Church(*this));
	}

	virtual Expression fold(const Expression& expr) const
	{
		std::vector<Ref<Expression> > expressions;
		expressions.push_back(expr);
		return (ChurchBuiltin::function(expressions));
	}
#endif
} Church;

#endif /* !CHURCH_H */
