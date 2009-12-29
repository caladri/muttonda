#ifndef	LIB_H
#define	LIB_H

#include <iostream>

struct DefineBuiltin {
	static std::string name(void)
	{
		return ("define");
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
	static std::string name(void)
	{
		return ("defined?");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(expressions[0]);

		if (Program::instance_.defined(a->string().string())) {
			return (Program::instance_.eval(new Expression(Name("T")), true));
		}
		return (Program::instance_.eval(new Expression(Name("F")), true));
	}
};

static struct : Builtin<DefinedBuiltin, 1> { } Defined;

struct EvalBuiltin {
	static std::string name(void)
	{
		return ("eval");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(expressions[0]);
		std::string s = a->string().string();

		return (Program::instance_.eval(parse(s), true));
	}
};

static struct : Builtin<EvalBuiltin, 1> { } Eval;

struct LoadBuiltin {
	static std::string name(void)
	{
		return ("load");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(expressions[0]);
		std::string s = a->string().string();

		if (Program::instance_.load(s))
			return (Program::instance_.eval(new Expression(Name("T")), true));
		return (Program::instance_.eval(new Expression(Name("F")), true));
	}
};

static struct : Builtin<LoadBuiltin, 1> { } Load;

struct ReadBuiltin {
	static std::string name(void)
	{
		return ("read");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(expressions[0]);
		
		std::string line;
		std::getline(std::cin, line);

		return (Program::instance_.eval(new Expression(a, new Expression(String(line))), true));
	}
};

static struct : Builtin<ReadBuiltin, 1> { } Read;

struct PrintBuiltin {
	static std::string name(void)
	{
		return ("print");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(expressions[0]);
		std::string s = a->string().string();

		if (s == "\\n")
			std::cout << std::endl;
		else
			std::cout << s;

		return (new Expression(Lambda(Name("x"), new Expression(Name("x")))));
	}
};

static struct : Builtin<PrintBuiltin, 1> { } Print;

struct ScalarAddBuiltin {
	static std::string name(void)
	{
		return ("scalar+");
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
	static std::string name(void)
	{
		return ("scalar=");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(expressions[0]);
		Ref<Expression> b(expressions[1]);

		if (a->scalar() == b->scalar())
			return (Program::instance_.eval(new Expression(Name("T")), true));
		return (Program::instance_.eval(new Expression(Name("F")), true));
	}
};

static struct : Builtin<ScalarEqualBuiltin, 2> { } ScalarEqual;

struct ShowBuiltin {
	static std::string name(void)
	{
		return ("show");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(expressions[0]);
		std::ostringstream os;

		a = a->eval();
		if (!a.null())
			os << a;
		else
			os << expressions[0];

		return (new Expression(String(os.str())));
	}
};

static struct : Builtin<ShowBuiltin, 1> { } Show;

struct StringLengthBuiltin {
	static std::string name(void)
	{
		return ("string-length");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(expressions[0]);
		return (new Expression(Scalar(a->string().string().size())));
	}
};

static struct : Builtin<StringLengthBuiltin, 1> { } StringLength;

#endif /* !LIB_H */
