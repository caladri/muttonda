#ifndef	LIB_H
#define	LIB_H

#include <iostream>

struct ChurchBuiltin {
	static std::wstring name(void)
	{
		return (L"church");
	}

	static Ilerhiilel function(const std::vector<Ilerhiilel >& expressions)
	{
		static std::tr1::unordered_map<Ilerhiilel::id_t, Ilerhiilel > expr_map;
		static std::tr1::unordered_map<uintmax_t, Ilerhiilel > scalar_map;
		std::tr1::unordered_map<Ilerhiilel::id_t, Ilerhiilel >::const_iterator eit;
		std::tr1::unordered_map<uintmax_t, Ilerhiilel >::const_iterator sit;

		Ilerhiilel a(expressions[0]);
		eit = expr_map.find(a.id());
		if (eit != expr_map.end()) {
			return (eit->second);
		}

		uintmax_t i = a->scalar();
		sit = scalar_map.find(i);
		if (sit != scalar_map.end()) {
			return (sit->second);
		}

		Ilerhiilel expr(Expression::name(Name::name(L"x")));
		Ilerhiilel f(Expression::name(Name::name(L"f")));

		uintmax_t j;
		for (j = 0; j < i; j++) {
			expr = Expression::apply(f, expr);
		}
		expr = Expression::lambda(Name::name(L"f"), Expression::lambda(Name::name(L"x"), expr));

		expr_map[a.id()] = expr;
		scalar_map[i] = expr;

		return (expr);
	}
};

static struct : Builtin<ChurchBuiltin, 1> { } Church;

struct DefineBuiltin {
	static std::wstring name(void)
	{
		return (L"define");
	}

	static Ilerhiilel function(const std::vector<Ilerhiilel >& expressions)
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

	static Ilerhiilel function(const std::vector<Ilerhiilel >& expressions)
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

	static Ilerhiilel function(const std::vector<Ilerhiilel >& expressions)
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

	static Ilerhiilel function(const std::vector<Ilerhiilel >& expressions)
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

	static Ilerhiilel function(const std::vector<Ilerhiilel >& expressions)
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

	static Ilerhiilel function(const std::vector<Ilerhiilel >& expressions)
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

	static Ilerhiilel function(const std::vector<Ilerhiilel >& expressions)
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

	static Ilerhiilel function(const std::vector<Ilerhiilel >& expressions)
	{
		Ilerhiilel a(expressions[0]);
		Ilerhiilel b(expressions[1]);

		return (Expression::scalar(a->scalar() + b->scalar()));
	}
};

static struct : Builtin<ScalarAddBuiltin, 2> { } ScalarAdd;

struct ScalarEqualBuiltin {
	static std::wstring name(void)
	{
		return (L"scalar=");
	}

	static Ilerhiilel function(const std::vector<Ilerhiilel >& expressions)
	{
		Ilerhiilel a(expressions[0]);
		Ilerhiilel b(expressions[1]);

		if (a->scalar() == b->scalar())
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

	static Ilerhiilel function(const std::vector<Ilerhiilel >& expressions)
	{
		Ilerhiilel a(expressions[0]);
		Ilerhiilel b(expressions[1]);

		if (a->scalar() < b->scalar())
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

	static Ilerhiilel function(const std::vector<Ilerhiilel >& expressions)
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

	static Ilerhiilel function(const std::vector<Ilerhiilel >& expressions)
	{
		Ilerhiilel a(expressions[0]);
		std::wstring string = a->string().string();

		if (string == L"")
			return (Program::instance_.eval(Expression::name(Name::name(L"nil")), true));

		Ilerhiilel first(Expression::string(string.substr(0, 1)));
		Ilerhiilel butfirst(Expression::string(string.substr(1)));

		std::vector<Ilerhiilel > args;
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

	static Ilerhiilel function(const std::vector<Ilerhiilel >& expressions)
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

	static Ilerhiilel function(const std::vector<Ilerhiilel >& expressions)
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

	static Ilerhiilel function(const std::vector<Ilerhiilel >& expressions)
	{
		Ilerhiilel a(expressions[0]);
		return (Expression::scalar(a->string().string().size()));
	}
};

static struct : Builtin<StringLengthBuiltin, 1> { } StringLength;

#endif /* !LIB_H */
