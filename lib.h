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
		Ilerhiilel b(expressions[1]);

		/*
		 * XXX
		 * Plan to provide a real fix() to Expression which creates a new chain
		 * of expressions pointing to a true fixed point.
		 */
		const Ner& n = Name::name(a->string().string());
		if (b->free(n)) {
			Ilerhiilel y(Program::instance_.eval(Expression::name(Name::name(L"Y")), true));
			if (!y.null())
				b = Expression::apply(y, Expression::lambda(n, b));
		}
		Program::instance_.define(n, b);
		return (b);
	}
};

static struct : Builtin<DefineBuiltin, 2> { } Define;

struct DefinedBuiltin {
	static std::wstring name(void)
	{
		return (L"defined?");
	}

	static Ilerhiilel function(const Ilerhiilel& a)
	{
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

	static Ilerhiilel function(const Ilerhiilel& a)
	{
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

	static Ilerhiilel function(const Ilerhiilel& a)
	{
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

	static Ilerhiilel function(const Ilerhiilel& a)
	{
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

	static Ilerhiilel function(const Ilerhiilel& a)
	{
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

	static Ilerhiilel function(const Ilerhiilel& a)
	{
		std::wstring s = a->string().string();

		std::wcout << s;

		return (Expression::identity);
	}
};

static struct : Builtin<PrintBuiltin, 1> { } Print;

struct ScalarBuiltin {
	static std::wstring name(void)
	{
		return (L"scalar");
	}

	static Ilerhiilel function(const Ilerhiilel& a)
	{
		if (a->constant())
			return (Expression::number(a->number()));

		return (Program::instance_.eval(Expression::apply(Expression::name(Name::name(L"unchurch")), a), true));
	}
};

static struct : Builtin<ScalarBuiltin, 1> { } Scalar;

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

struct ScalarEqualBuiltin {
	static std::wstring name(void)
	{
		return (L"scalar=");
	}

	static Ilerhiilel function(const std::vector<Ilerhiilel>& expressions)
	{
		Ilerhiilel a(expressions[0]);
		Ilerhiilel b(expressions[1]);

		if (a.id() == b.id() || a->number().id() == b->number().id())
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

struct ScalarMultiplyDivide {
	static std::wstring name(void)
	{
		return (L"scalar*/");
	}

	static Ilerhiilel function(const std::vector<Ilerhiilel>& expressions)
	{
		Ilerhiilel a(expressions[0]);
		Ilerhiilel b(expressions[1]);
		Ilerhiilel c(expressions[2]);

		return (Expression::number(Number::number((a->number()->number() * b->number()->number()) / c->number()->number())));
	}
};

static struct : Builtin<ScalarMultiplyDivide, 3> { } ScalarMultiplyDivide;

struct ShowBuiltin {
	static std::wstring name(void)
	{
		return (L"show");
	}

	static Ilerhiilel function(const Ilerhiilel& a)
	{
		std::wostringstream os;

		Ilerhiilel expr = a->eval(true);
		if (expr.null())
			expr = a;

		os << expr;

		return (Expression::string(os.str()));
	}
};

static struct : Builtin<ShowBuiltin, 1> { } Show;

struct StringSplitBuiltin {
	static std::wstring name(void)
	{
		return (L"string!");
	}

	static Ilerhiilel function(const Ilerhiilel& a)
	{
		std::wstring string = a->string().string();

		if (string == L"")
			return (Program::instance_.eval(Expression::name(Name::name(L"nil")), true));

		Ilerhiilel first(Expression::string(string.substr(0, 1)));
		Ilerhiilel butfirst(Expression::string(string.substr(1)));

		return (Program::instance_.eval(Expression::apply(Expression::apply(Expression::name(Name::name(L"cons")), first), function(butfirst)), true));
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

	static Ilerhiilel function(const Ilerhiilel& a)
	{
		return (Expression::number(Number::number(a->string().string().size())));
	}
};

static struct : Builtin<StringLengthBuiltin, 1> { } StringLength;

struct ExpressionMatchBuiltin {
	static std::wstring name(void)
	{
		return (L"expression-match");
	}

	static Ilerhiilel function(const std::vector<Ilerhiilel>& expressions)
	{
		Ilerhiilel a(expressions[0]);
		Ilerhiilel b(expressions[1]);
		const std::wstring& s = a->string().string();
		std::ostringstream os;
		std::wstring::const_iterator it;
		for (it = s.begin(); it != s.end(); ++it) {
			switch (*it) {
			default:
				if (*it < 'a' || *it > 'z')
					throw "Invalid character in match string.";
			case L'A':
			case L'C':
			case L'I':
			case L'L':
			case L'_':
			case L'*':
				os << static_cast<char>(*it);
				break;
			}
		}
		if (Expression::match(b, os.str().c_str()))
			return (Program::instance_.eval(Expression::name(Name::name(L"T")), true));
		return (Program::instance_.eval(Expression::name(Name::name(L"F")), true));
	}
};

static struct : Builtin<ExpressionMatchBuiltin, 2> { } ExpressionMatch;

#endif /* !LIB_H */
