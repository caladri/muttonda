#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <vector>

#include "expression.h"
#include "function.h"
#include "lambda.h"
#include "name.h"

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

static Expression apply(const std::vector<Expression>&);
static Expression read(std::istream&, bool);
static std::string read_token(std::istream&);

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
			Expression expr(read(istr, false));

			expr = program.eval(expr);

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

static Expression
apply(const std::vector<Expression>& expressions)
{
	std::vector<Expression>::const_iterator it;

	if (expressions.empty())
		throw 0;

	Expression expr(expressions[0]);
	unsigned i;

	for (i = 1; i < expressions.size(); i++)
		expr = Expression(expr, expressions[i]);

	return (expr);
}

static Expression
read(std::istream& is, bool in_parens)
{
	std::vector<Expression> expressions;
	std::string token;

	while (is.good()) {
		token = read_token(is);

		if (token == "(") {
			expressions.push_back(read(is, true));
		} else if (token == ")") {
			if (in_parens)
				return (apply(expressions));
			throw "Expected token, got parenthesis.";
		} else if (token == "\\") {
			token = read_token(is);

			if (token == "(" || token == ")" || token == "\\" || token == "->")
				throw "Expected variable for lambda.";
			if (read_token(is) != "->")
				throw "Expected arrow for lambda.";

			expressions.push_back(Lambda(token, read(is, in_parens)));

			return (apply(expressions));
		} else if (token == "\n") {
			break;
		} else if (token != "") {
			if (token == "->")
				throw "Unexpected arrow outside of lambda.";
			std::string::iterator it;
			for (it = token.begin(); it != token.end(); ++it) {
				if (std::isdigit(*it))
					continue;
				expressions.push_back(Name(token));
				token = "";
				break;
			}
			if (token != "")
				expressions.push_back(Scalar(atoi(token.c_str())));
		}
	}
	if (in_parens)
		throw "EOL before end of expression in parentheses.";
	if (expressions.empty())
		throw 0;
	return (apply(expressions));
}

static std::string
read_token(std::istream& is)
{
	std::string token;
	int ch;

	while (is.good()) {
		ch = is.get();

		switch (ch) {
		case EOF:
			return (token);
		case ' ':
		case '\t':
			if (token != "")
				return (token);
			break;
		case '(':
		case ')':
		case '\\':
		case '\n':
			if (token != "") {
				is.putback(ch);
				return (token);
			}
			return (std::string() + (char)ch);
		case '-':
			ch = is.get();

			switch (ch) {
			case EOF:
				return (token + '-');
			case '>':
				if (token != "") {
					is.putback('>');
					is.putback('-');
					return (token);
				}
				return ("->");
			default:
				token += '-';
				token += (char)ch;
				break;
			}
			break;
		default:
			token += (char)ch;
			break;
		}
	}
	return (token);
}
