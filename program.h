#ifndef	PROGRAM_H
#define	PROGRAM_H

class Program {
	std::map<std::string, Expression> definitions_;

public:
	Program(void);

	~Program();

	void begin(bool) const;

	void define(const std::string&, const Expression&);

	bool defined(const std::string&);

	void defun(const std::string&, const std::vector<Name>&, const Expression&);

	void defun(const std::string&, const Expression&, const char * = NULL, ...);

	void defun(const SimpleFunction&);

	Expression eval(const Expression&, bool) const;

	void help(void) const;

	static Program instance_;
};

struct DefineBuiltin {
	static std::string name(void)
	{
		return ("define");
	}

	static Expression function(const std::vector<Expression>& expressions)
	{
		Expression var = expressions[0].eval();

		Program::instance_.defun(var.string().string(), expressions[1]);
		return (expressions[1]);
	}
};

static Builtin<DefineBuiltin> Define(2);

struct DefinedBuiltin {
	static std::string name(void)
	{
		return ("defined?");
	}

	static Expression function(const std::vector<Expression>& expressions)
	{
		Expression var = expressions[0].eval();

		if (Program::instance_.defined(var.string().string())) {
			return (Program::instance_.eval(Name("T"), true));
		}
		return (Program::instance_.eval(Name("F"), true));
	}
};

static Builtin<DefinedBuiltin> Defined(1);

struct EvalBuiltin {
	static std::string name(void)
	{
		return ("eval");
	}

	static Expression function(const std::vector<Expression>& expressions)
	{
		Expression a = expressions[0].eval();
		std::string s = a.string().string();

		return (Program::instance_.eval(parse(s), true));
	}
};

static Builtin<EvalBuiltin> Eval(1);

struct StringLengthBuiltin {
	static std::string name(void)
	{
		return ("string-length");
	}

	static Expression function(const std::vector<Expression>& expressions)
	{
		Expression a = expressions[0].eval();
		return (Expression(Scalar(a.string().string().size())));
	}
};

static Builtin<StringLengthBuiltin> StringLength(1);

#endif /* !PROGRAM_H */
