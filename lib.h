#ifndef	LIB_H
#define	LIB_H

#include <iostream>

struct DefineBuiltin {
	static std::wstring name(void)
	{
		return (L"define");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(expressions[0]);

		Program::instance_.define(a->string().string(), expressions[1]);
		return (expressions[1]);
	}
};

static struct : Builtin<DefineBuiltin, 2> { } Define;

struct DefinedBuiltin {
	static std::wstring name(void)
	{
		return (L"defined?");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(expressions[0]);

		if (Program::instance_.defined(a->string().string())) {
			return (Program::instance_.eval(Expression::name(L"T"), true));
		}
		return (Program::instance_.eval(Expression::name(L"F"), true));
	}
};

static struct : Builtin<DefinedBuiltin, 1> { } Defined;

struct EvalBuiltin {
	static std::wstring name(void)
	{
		return (L"eval");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(expressions[0]);
		std::wstring s = a->string().string();

		return (Program::instance_.eval(parse(s), true));
	}
};

static struct : Builtin<EvalBuiltin, 1> { } Eval;

struct LetBuiltin {
	static std::wstring name(void)
	{
		return (L"let");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(expressions[0]);
		Name n = a->name();
		Ref<Expression> b(expressions[1]);
		Ref<Expression> expr(expressions[2]);

		Ref<Expression> bound(expr->bind(n, b));
		if (!bound.null())
			expr = bound;

		return (Program::instance_.eval(expr, true));
	}
};

static struct : Builtin<LetBuiltin, 3> { } Let;

struct LoadBuiltin {
	static std::wstring name(void)
	{
		return (L"load");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(expressions[0]);
		std::wstring s = a->string().string();

		if (Program::instance_.load(s))
			return (Program::instance_.eval(Expression::name(L"T"), true));
		return (Program::instance_.eval(Expression::name(L"F"), true));
	}
};

static struct : Builtin<LoadBuiltin, 1> { } Load;

struct MemoizeBuiltin {
	static std::wstring name(void)
	{
		return (L"memoize");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(expressions[0]);

		return (a->eval(true));
	}
};

static struct : Builtin<MemoizeBuiltin, 1> { } Memoize;

struct ReadBuiltin {
	static std::wstring name(void)
	{
		return (L"read");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(expressions[0]);
		
		std::wstring line;
		std::getline(std::wcin, line);

		return (Program::instance_.eval(Expression::apply(a, Expression::string(line)), true));
	}
};

static struct : Builtin<ReadBuiltin, 1> { } Read;

struct PrintBuiltin {
	static std::wstring name(void)
	{
		return (L"print");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(expressions[0]);
		std::wstring s = a->string().string();

		if (s == L"\\n")
			std::wcout << std::endl;
		else
			std::wcout << s;

		return (Program::instance_.eval(Expression::name(L"I"), true));
	}
};

static struct : Builtin<PrintBuiltin, 1> { } Print;

struct ScalarAddBuiltin {
	static std::wstring name(void)
	{
		return (L"scalar+");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(expressions[0]);
		Ref<Expression> b(expressions[1]);

		return (Expression::scalar(a->scalar() + b->scalar()));
	}
};

static struct : Builtin<ScalarAddBuiltin, 2> { } ScalarAdd;

struct ScalarEqualBuiltin {
	static std::wstring name(void)
	{
		return (L"scalar=");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(expressions[0]);
		Ref<Expression> b(expressions[1]);

		if (a->scalar() == b->scalar())
			return (Program::instance_.eval(Expression::name(L"T"), true));
		return (Program::instance_.eval(Expression::name(L"F"), true));
	}
};

static struct : Builtin<ScalarEqualBuiltin, 2> { } ScalarEqual;

struct ScalarLessThanBuiltin {
	static std::wstring name(void)
	{
		return (L"scalar<");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(expressions[0]);
		Ref<Expression> b(expressions[1]);

		if (a->scalar() < b->scalar())
			return (Program::instance_.eval(Expression::name(L"T"), true));
		return (Program::instance_.eval(Expression::name(L"F"), true));
	}
};

static struct : Builtin<ScalarLessThanBuiltin, 2> { } ScalarLessThan;

struct ShowBuiltin {
	static std::wstring name(void)
	{
		return (L"show");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(expressions[0]);
		std::wostringstream os;

		Ref<Expression> evaluated(a->eval(true));
		if (!evaluated.null())
			a = evaluated;
		Ref<Expression> simplified(a->simplify());
		if (!simplified.null())
			a = simplified;

		os << a;

		return (Expression::string(os.str()));
	}
};

static struct : Builtin<ShowBuiltin, 1> { } Show;

struct StringAddBuiltin {
	static std::wstring name(void)
	{
		return (L"string+");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(expressions[0]);
		Ref<Expression> b(expressions[1]);

		return (Expression::string(a->string() + b->string()));
	}
};

static struct : Builtin<StringAddBuiltin, 2> { } StringAdd;

struct StringLengthBuiltin {
	static std::wstring name(void)
	{
		return (L"string-length");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(expressions[0]);
		return (Expression::scalar(a->string().string().size()));
	}
};

static struct : Builtin<StringLengthBuiltin, 1> { } StringLength;

#endif /* !LIB_H */
