#ifndef	CHURCH_H
#define	CHURCH_H

/*
 * Some very religious combinators and useful functions for working with them.
 */

Expression S(Lambda("x", Lambda("y", Lambda("z", Expression(Expression(Name("x"), Name("z")), Expression(Name("y"), Name("z")))))));
Expression K(Lambda("x", Lambda("y", Name("x"))));

Expression True(K);
Expression False(Lambda("x", Lambda("y", Name("y"))));

Expression nil(Lambda("x", True));

Expression cons(Lambda("x", Lambda("y", Lambda("m", Expression(Expression(Name("m"), Name("x")), Name("y"))))));
Expression car(Lambda("z", Expression(Name("z"), True)));
Expression cdr(Lambda("z", Expression(Name("z"), False)));

Expression cond(Lambda("p", Lambda("t", Lambda("f", Expression(Expression(Name("p"), Name("t")), Name("f"))))));

struct AddBuiltin {
	static std::string name(void)
	{
		return ("+");
	}

	static Expression function(const std::vector<Expression>& expressions)
	{
		Expression a = expressions[0].eval();
		Expression b = expressions[1].eval();

		return (Expression(a.scalar() + b.scalar()));
	}
};

static Builtin<AddBuiltin> Add(2);

struct ChurchBuiltin {
	static std::string name(void)
	{
		return ("church");
	}

	static Expression function(const std::vector<Expression>& expressions)
	{
		Expression a = expressions[0].eval();
		unsigned i = a.scalar().value();

		Expression expr(Name("x"));
		while (i--) {
			expr = Expression(Name("f"), expr);
		}
		return (Expression(Lambda("f", Lambda("x", expr))));
	}
};

static Builtin<ChurchBuiltin> Church(1);

Lambda unchurch("n", Expression(Expression(Name("n"), Expression(Lambda("x", Expression(Expression(Add, Name("x")), Scalar(1))))), Scalar(0)));
Lambda plus("m", Lambda("n", Lambda("f", Lambda("x", Expression(Expression(Name("m"), Name("f")), Expression(Expression(Name("n"), Name("f")), Name("x")))))));
Lambda mult("m", Lambda("n", Lambda("f", Expression(Name("n"), Expression(Name("m"), Name("f"))))));
Lambda expn("m", Lambda("n", Expression(Name("n"), Name("m"))));

#endif /* !CHURCH_H */
