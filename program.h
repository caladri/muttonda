#ifndef	PROGRAM_H
#define	PROGRAM_H

#include <sstream> /* XXX If we had a lib.h, this could go there with the Show builtin.  */

class Program {
	std::map<std::string, Ref<Expression> > definitions_;

public:
	Program(void);

	~Program();

	void begin(bool) const;

	void define(const std::string&, const Ref<Expression>&);

	bool defined(const std::string&);

	void defun(const std::string&, const std::vector<Name>&, const Ref<Expression>&);

	void defun(const std::string&, const Ref<Expression>&);

	void defun(const SimpleFunction&);

	Ref<Expression> eval(const Ref<Expression>&, bool) const;

	void help(void) const;

	static Program instance_;
};

struct DefineBuiltin {
	static std::string name(void)
	{
		return ("define");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> var(Expression::eval(expressions[0]));

		Program::instance_.defun(var->string().string(), expressions[1]);
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
		Ref<Expression> var(Expression::eval(expressions[0]));

		if (Program::instance_.defined(var->string().string())) {
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
		Ref<Expression> a(Expression::eval(expressions[0]));
		std::string s = a->string().string();

		return (Program::instance_.eval(parse(s), true));
	}
};

static struct : Builtin<EvalBuiltin, 1> { } Eval;

struct PrintBuiltin {
	static std::string name(void)
	{
		return ("print");
	}

	static Ref<Expression> function(const std::vector<Ref<Expression> >& expressions)
	{
		Ref<Expression> a(Expression::eval(expressions[0]));
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
		Ref<Expression> a(Expression::eval(expressions[0]));
		std::ostringstream os;

		os << *a;

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
		Ref<Expression> a(Expression::eval(expressions[0]));
		return (new Expression(Scalar(a->string().string().size())));
	}
};

static struct : Builtin<StringLengthBuiltin, 1> { } StringLength;

#endif /* !PROGRAM_H */
