#include <iostream>
#include <string>
#include <map>
#include <vector>

#include "expression.h"
#include "function.h"
#include "lambda.h"
#include "name.h"
#include "parse.h"
#include "program.h"

Program::Program(void)
: definitions_()
{ }

Program::~Program()
{ }

void
Program::begin(void) const
{
	std::cout << "Muttonda, duh!  Here's what's in the standard library:" << std::endl;
	help();
	std::cout << "Now have at it!  (Tasty mutton...)" << std::endl;
}

void
Program::define(const std::string& str, const Expression& expr)
{
	if (definitions_.find(str) != definitions_.end())
		definitions_.erase(str);
	definitions_.insert(std::map<std::string, Expression>::value_type(str, eval(expr, true)));
}

void
Program::defun(const std::string& str, const std::vector<Name>& vars, const Expression& expr)
{
	std::vector<Name>::const_reverse_iterator it;
	Expression fun(expr);

	for (it = vars.rbegin(); it != vars.rend(); ++it) {
		fun = Lambda(*it, fun);
	}
	define(str, fun);
}

void
Program::defun(const SimpleFunction& fun)
{
	defun(fun.name(), fun);
}

void
Program::defun(const std::string& str, const Expression& expr, const char *var, ...)
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

Expression
Program::eval(const Expression& expr, bool quiet) const
{
	std::map<std::string, Expression>::const_iterator it;
	Expression program(expr);

	if (!quiet)
		std::cout << "eval: " << expr << " =>" << std::endl;
	for (it = definitions_.begin(); it != definitions_.end(); ++it)
		program = Expression(Lambda(it->first, program), it->second);
#if defined(VERBOSE) && defined(BAAAAAAA)
	if (!quiet)
		std::cout << "      " << program << " =>" << std::endl;
#endif
	return (program.eval());
}

void
Program::help(void) const
{
	std::map<std::string, Expression>::const_iterator it;

	for (it = definitions_.begin(); it != definitions_.end(); ++it) {
		std::cout << "\t" << it->first << " = " << it->second << std::endl;
	}
}
