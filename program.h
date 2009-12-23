#ifndef	PROGRAM_H
#define	PROGRAM_H

#include <sstream> /* XXX If we had a lib.h, this could go there with the Show builtin.  */

class Program {
	std::map<std::string, Expression> definitions_;

public:
	Program(void);

	~Program();

	void begin(bool) const;

	void define(const std::string&, const Expression&);

	bool defined(const std::string&);

	void defun(const std::string&, const std::vector<Name>&, const Expression&);

	void defun(const std::string&, const Expression&);

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

static struct : Builtin<DefineBuiltin, 2> { } Define;

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

static struct : Builtin<DefinedBuiltin, 1> { } Defined;

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

static struct : Builtin<EvalBuiltin, 1> { } Eval;

struct ShowBuiltin {
	static std::string name(void)
	{
		return ("show");
	}

	static Expression function(const std::vector<Expression>& expressions)
	{
		Expression a = expressions[0].eval();
		std::ostringstream os;

		os << a;

		return (Expression(String(os.str())));
	}
};

static struct : Builtin<ShowBuiltin, 1> { } Show;

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

static struct : Builtin<StringLengthBuiltin, 1> { } StringLength;

#endif /* !PROGRAM_H */
