#ifndef	CHURCH_H
#define	CHURCH_H

struct ScalarAddBuiltin {
	static std::string name(void)
	{
		return ("scalar+");
	}

	static Expression function(const std::vector<Expression>& expressions)
	{
		Expression a = expressions[0].eval();
		Expression b = expressions[1].eval();

		return (Expression(a.scalar() + b.scalar()));
	}
};

static struct : Builtin<ScalarAddBuiltin, 2> { } ScalarAdd;

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

static struct _Church : Builtin<ChurchBuiltin, 1> {
	Function *clone(void) const
	{
		return (new _Church(*this));
	}

	virtual Expression fold(const Expression& expr) const
	{
		std::vector<Expression> expressions;
		expressions.push_back(expr);
		return (ChurchBuiltin::function(expressions));
	}
} Church;

#endif /* !CHURCH_H */
