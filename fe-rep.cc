#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <vector>

#include "expression.h"
#include "function.h"
#include "lambda.h"
#include "name.h"
#include "parse.h"

#include "church.h"

class Program {
	std::map<std::string, Expression> definitions_;

public:
	Program(void)
	: definitions_()
	{ }

	void begin(void) const
	{
		std::map<std::string, Expression>::const_iterator it;

		std::cout << "Muttonda, duh!  Here's what's in the standard library:" << std::endl;
		for (it = definitions_.begin(); it != definitions_.end(); ++it) {
			std::cout << "\t" << it->first << " = " << it->second << std::endl;
		}
		std::cout << "Now have at it!  (Tasty mutton...)" << std::endl;
	}

	void define(const std::string& str, const Expression& expr)
	{
		if (definitions_.find(str) != definitions_.end())
			definitions_.erase(str);
		definitions_.insert(std::map<std::string, Expression>::value_type(str, expr));
	}

	void defun(const std::string& str, const std::vector<Name>& vars, const Expression& expr)
	{
		std::vector<Name>::const_reverse_iterator it;
		Expression fun(expr);

		for (it = vars.rbegin(); it != vars.rend(); ++it) {
			fun = Lambda(*it, fun);
		}
		define(str, fun);
	}

	void defun(const std::string& str, const Expression& expr, const char *var = NULL, ...)
	{
		std::vector<Name> vars;
		va_list ap;

		va_start(ap, var);
		while (var != NULL) {
			vars.push_back(var);
			var = va_arg(ap, const char *);
		}
		va_end(ap);
		defun(str, vars, expr);
	}

	Expression eval(const Expression& expr) const
	{
		std::map<std::string, Expression>::const_iterator it;
		Expression program(expr);

		std::cout << "eval: " << expr << " =>" << std::endl;
		for (it = definitions_.begin(); it != definitions_.end(); ++it)
			program = Expression(Lambda(it->first, program), it->second);
#if defined(VERBOSE) && defined(BAAAAAAA)
		std::cout << "      " << program << " =>" << std::endl;
#endif
		return (program.eval());
	}
};

static Program program;

struct DefineBuiltin {
	static std::string name(void)
	{
		return ("define");
	}

	static Expression function(const std::vector<Expression>& expressions)
	{
		Expression var = expressions[0];
		Name name = var.name();

		program.defun(name.str(), expressions[1]);
		return (expressions[1]);
	}
};

static Builtin<DefineBuiltin> Define(2);

int
main(void)
{
	/* Load some useful library functions.  */
	program.defun("define", Define);

	program.defun("S", S);
	program.defun("K", K);

	program.defun("T", True);
	program.defun("F", False);

	program.defun("nil", nil);
	program.defun("cons", cons);
	program.defun("car", car);
	program.defun("cdr", cdr);
	program.defun("cond", cond);

	program.defun("church", Church);
	program.defun("unchurch", unchurch);
	program.defun("+", plus);
	program.defun("*", mult);
	program.defun("**", expn);

	/* XXX Assumes STDIN_FILENO == std::cin.  Sigh.  */
	if (isatty(STDIN_FILENO))
		program.begin();

	while (std::cin.good()) {
		/* XXX Assumes STDIN_FILENO == std::cin.  Sigh.  */
		if (isatty(STDIN_FILENO))
			std::cout << "? ";

		std::string line;
		std::getline(std::cin, line);
		std::istringstream istr(line);

		try {
			Expression expr(program.eval(parse(istr)));

			std::cout << expr << std::endl;

			program.defun("_", expr);
		} catch (const char *msg) {
			if (msg != NULL)
				std::cerr << "Error: " << msg << std::endl;
		} catch (int status) {
			switch (status) {
			case 0:
				continue;
			case EOF:
				std::cerr << "EOF." << std::endl;
				return (0);
			default:
				std::cerr << "Unexpected status: " << status << std::endl;
			}
		}
	}
}
