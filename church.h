#ifndef	CHURCH_H
#define	CHURCH_H

struct ScalarAddBuiltin {
	static std::string name(void)
	{
		return ("scalar+");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(Expression::eval(expressions[0]));
		Ref<Expression> b(Expression::eval(expressions[1]));

		return (new Expression(a->scalar() + b->scalar()));
	}
};

static struct : Builtin<ScalarAddBuiltin, 2> { } ScalarAdd;

struct ScalarEqualBuiltin {
	static std::string name(void)
	{
		return ("scalar=");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(Expression::eval(expressions[0]));
		Ref<Expression> b(Expression::eval(expressions[1]));

		if (a->scalar() == b->scalar())
			return (new Expression(Scalar(1)));
		return (new Expression(Scalar(0)));
	}
};

static struct : Builtin<ScalarEqualBuiltin, 2> { } ScalarEqual;

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
