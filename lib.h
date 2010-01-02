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
			return (Program::instance_.eval(new Expression(Name(L"T")), true));
		}
		return (Program::instance_.eval(new Expression(Name(L"F")), true));
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
			return (Program::instance_.eval(new Expression(Name(L"T")), true));
		return (Program::instance_.eval(new Expression(Name(L"F")), true));
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

		return (Program::instance_.eval(new Expression(a, new Expression(String(line))), true));
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

		return (new Expression(Lambda(Name(L"x"), new Expression(Name(L"x")))));
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

		return (new Expression(a->scalar() + b->scalar()));
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
			return (Program::instance_.eval(new Expression(Name(L"T")), true));
		return (Program::instance_.eval(new Expression(Name(L"F")), true));
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
			return (Program::instance_.eval(new Expression(Name(L"T")), true));
		return (Program::instance_.eval(new Expression(Name(L"F")), true));
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

		a = a->eval(true);
		if (!a.null())
			os << a;
		else
			os << expressions[0];

		return (new Expression(String(os.str())));
	}
};

static struct : Builtin<ShowBuiltin, 1> { } Show;

struct StringLengthBuiltin {
	static std::wstring name(void)
	{
		return (L"string-length");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(expressions[0]);
		return (new Expression(Scalar(a->string().string().size())));
	}
};

static struct : Builtin<StringLengthBuiltin, 1> { } StringLength;

#endif /* !LIB_H */
