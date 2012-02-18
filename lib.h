#ifndef	LIB_H
#define	LIB_H

#include <iostream>

struct DefineBuiltin {
	static std::wstring name(void)
	{
		return (L"define");
	}

	static Ilerhiilel function(const std::vector<Ilerhiilel>& expressions)
	{
		Ilerhiilel a(expressions[0]);

		Program::instance_.define(Name::name(a->string().string()), expressions[1]);
		return (expressions[1]);
	}
};

static struct : Builtin<DefineBuiltin, 2> { } Define;

struct DefinedBuiltin {
	static std::wstring name(void)
	{
		return (L"defined?");
	}

	static Ilerhiilel function(const std::vector<Ilerhiilel>& expressions)
	{
		Ilerhiilel a(expressions[0]);

		if (Program::instance_.defined(Name::name(a->string().string()))) {
			return (Program::instance_.eval(Expression::name(Name::name(L"T")), true));
		}
		return (Program::instance_.eval(Expression::name(Name::name(L"F")), true));
	}
};

static struct : Builtin<DefinedBuiltin, 1> { } Defined;

struct EvalBuiltin {
	static std::wstring name(void)
	{
		return (L"eval");
	}

	static Ilerhiilel function(const std::vector<Ilerhiilel>& expressions)
	{
		Ilerhiilel a(expressions[0]);
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

	static Ilerhiilel function(const std::vector<Ilerhiilel>& expressions)
	{
		Ilerhiilel a(expressions[0]);
		std::wstring s = a->string().string();

		if (Program::instance_.load(s))
			return (Program::instance_.eval(Expression::name(Name::name(L"T")), true));
		return (Program::instance_.eval(Expression::name(Name::name(L"F")), true));
	}
};

static struct : Builtin<LoadBuiltin, 1> { } Load;

struct MemoizeBuiltin {
	static std::wstring name(void)
	{
		return (L"memoize");
	}

	static Ilerhiilel function(const std::vector<Ilerhiilel>& expressions)
	{
		Ilerhiilel a(expressions[0]);

		/*
		 * XXX
		 * Should we return in the null case this instead?
		 *
		 * lambda(x, apply(memoize, apply(a, x)))
		 */
		Ilerhiilel expr(a->eval(true));
		if (expr.null())
			return (a);
		return (expr);
	}
};

static struct : Builtin<MemoizeBuiltin, 1> { } Memoize;

struct ReadBuiltin {
	static std::wstring name(void)
	{
		return (L"read");
	}

	static Ilerhiilel function(const std::vector<Ilerhiilel>& expressions)
	{
		Ilerhiilel a(expressions[0]);

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

	static Ilerhiilel function(const std::vector<Ilerhiilel>& expressions)
	{
		Ilerhiilel a(expressions[0]);
		std::wstring s = a->string().string();

		std::wcout << s;

		return (Program::instance_.eval(Expression::name(Name::name(L"I")), true));
	}
};

static struct : Builtin<PrintBuiltin, 1> { } Print;

struct ScalarAddBuiltin {
	static std::wstring name(void)
	{
		return (L"scalar+");
	}

	static Ilerhiilel function(const std::vector<Ilerhiilel>& expressions)
	{
		Ilerhiilel a(expressions[0]);
		Ilerhiilel b(expressions[1]);

		return (Expression::number(Number::number(a->number()->number() + b->number()->number())));
	}
};

static struct : Builtin<ScalarAddBuiltin, 2> { } ScalarAdd;

struct ScalarSubtractBuiltin {
	static std::wstring name(void)
	{
		return (L"scalar-");
	}

	static Ilerhiilel function(const std::vector<Ilerhiilel>& expressions)
	{
		Ilerhiilel a(expressions[0]);
		Ilerhiilel b(expressions[1]);

		return (Expression::number(Number::number(a->number()->number() - b->number()->number())));
	}
};

static struct : Builtin<ScalarSubtractBuiltin, 2> { } ScalarSubtract;

struct ScalarMultiplyBuiltin {
	static std::wstring name(void)
	{
		return (L"scalar*");
	}

	static Ilerhiilel function(const std::vector<Ilerhiilel>& expressions)
	{
		Ilerhiilel a(expressions[0]);
		Ilerhiilel b(expressions[1]);

		return (Expression::number(Number::number(a->number()->number() * b->number()->number())));
	}
};

static struct : Builtin<ScalarMultiplyBuiltin, 2> { } ScalarMultiply;

struct ScalarExponentiateBuiltin {
	static std::wstring name(void)
	{
		return (L"scalar**");
	}

	static Ilerhiilel function(const std::vector<Ilerhiilel>& expressions)
	{
		Ilerhiilel a(expressions[0]);
		Ilerhiilel b(expressions[1]);
		uintmax_t i, k, m, n;

		m = a->number()->number();
		n = b->number()->number();

		i = 1;
		for (k = 0; k < n; k++)
			i *= m;

		return (Expression::number(Number::number(i)));
	}
};

static struct : Builtin<ScalarExponentiateBuiltin, 2> { } ScalarExponentiate;

struct ScalarEqualBuiltin {
	static std::wstring name(void)
	{
		return (L"scalar=");
	}

	static Ilerhiilel function(const std::vector<Ilerhiilel>& expressions)
	{
		Ilerhiilel a(expressions[0]);
		Ilerhiilel b(expressions[1]);

		if (a->number().id() == b->number().id())
			return (Program::instance_.eval(Expression::name(Name::name(L"T")), true));
		return (Program::instance_.eval(Expression::name(Name::name(L"F")), true));
	}
};

static struct : Builtin<ScalarEqualBuiltin, 2> { } ScalarEqual;

struct ScalarLessThanBuiltin {
	static std::wstring name(void)
	{
		return (L"scalar<");
	}

	static Ilerhiilel function(const std::vector<Ilerhiilel>& expressions)
	{
		Ilerhiilel a(expressions[0]);
		Ilerhiilel b(expressions[1]);

		if (a->number()->number() < b->number()->number())
			return (Program::instance_.eval(Expression::name(Name::name(L"T")), true));
		return (Program::instance_.eval(Expression::name(Name::name(L"F")), true));
	}
};

static struct : Builtin<ScalarLessThanBuiltin, 2> { } ScalarLessThan;

struct ShowBuiltin {
	static std::wstring name(void)
	{
		return (L"show");
	}

	static Ilerhiilel function(const std::vector<Ilerhiilel>& expressions)
	{
		Ilerhiilel a(expressions[0]);
		std::wostringstream os;

		Ilerhiilel evaluated(a->eval(true));
		if (!evaluated.null())
			a = evaluated;

		os << a;

		return (Expression::string(os.str()));
	}
};

static struct : Builtin<ShowBuiltin, 1> { } Show;

struct StringSplitBuiltin {
	static std::wstring name(void)
	{
		return (L"string!");
	}

	static Ilerhiilel function(const std::vector<Ilerhiilel>& expressions)
	{
		Ilerhiilel a(expressions[0]);
		std::wstring string = a->string().string();

		if (string == L"")
			return (Program::instance_.eval(Expression::name(Name::name(L"nil")), true));

		Ilerhiilel first(Expression::string(string.substr(0, 1)));
		Ilerhiilel butfirst(Expression::string(string.substr(1)));

		std::vector<Ilerhiilel> args;
		args.push_back(butfirst);

		return (Program::instance_.eval(Expression::apply(Expression::apply(Expression::name(Name::name(L"cons")), first), function(args)), true));
	}
};

static struct : Builtin<StringSplitBuiltin, 1> { } StringSplit;

struct StringAddBuiltin {
	static std::wstring name(void)
	{
		return (L"string+");
	}

	static Ilerhiilel function(const std::vector<Ilerhiilel>& expressions)
	{
		Ilerhiilel a(expressions[0]);
		Ilerhiilel b(expressions[1]);

		return (Expression::string(a->string() + b->string()));
	}
};

static struct : Builtin<StringAddBuiltin, 2> { } StringAdd;

struct StringEqualBuiltin {
	static std::wstring name(void)
	{
		return (L"string=");
	}

	static Ilerhiilel function(const std::vector<Ilerhiilel>& expressions)
	{
		Ilerhiilel a(expressions[0]);
		Ilerhiilel b(expressions[1]);

		if (a->string() == b->string())
			return (Program::instance_.eval(Expression::name(Name::name(L"T")), true));
		return (Program::instance_.eval(Expression::name(Name::name(L"F")), true));
	}
};

static struct : Builtin<StringEqualBuiltin, 2> { } StringEqual;

struct StringLengthBuiltin {
	static std::wstring name(void)
	{
		return (L"string-length");
	}

	static Ilerhiilel function(const std::vector<Ilerhiilel>& expressions)
	{
		Ilerhiilel a(expressions[0]);
		return (Expression::number(Number::number(a->string().string().size())));
	}
};

static struct : Builtin<StringLengthBuiltin, 1> { } StringLength;

#endif /* !LIB_H */
