#ifndef	CHURCH_H
#define	CHURCH_H

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

#endif /* !CHURCH_H */
