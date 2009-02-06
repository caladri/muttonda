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

#include "church.h"

Program::Program(void)
: definitions_()
{ }

Program::~Program()
{ }

void
Program::begin(bool quiet) const
{
	/* Built-in functions.  */
	Program::instance_.defun(Church);
	Program::instance_.defun(Define);
	Program::instance_.defun(Eval);
	Program::instance_.defun(ScalarAdd);

	/* SK-calculus.  */
	Program::instance_.defun("S", parse("\\x y z -> x z (y z)"));
	Program::instance_.defun("K", parse("\\x y -> x"));

	/* Booleans.  */
	Program::instance_.defun("T", parse("\\x y -> x"));
	Program::instance_.defun("F", parse("\\x y -> y"));

	/* Boolean logic.  */
	Program::instance_.defun("and", parse("\\m n -> m n m"));
	Program::instance_.defun("or", parse("\\m n -> m m n"));
	Program::instance_.defun("not", parse("\\m -> m F T"));
	Program::instance_.defun("xor", parse("\\m n -> m (n F T) n"));

	/* Church pairs.  */
	Program::instance_.defun("pair", parse("\\x y z -> z x y"));
	Program::instance_.defun("fst", parse("\\p -> p \\x y -> x"));
	Program::instance_.defun("snd", parse("\\p -> p \\x y -> y"));

	/* Lists.  */
	Program::instance_.defun("nil", parse("pair T error"));
	Program::instance_.defun("nil?", parse("fst"));
	Program::instance_.defun("cons", parse("\\h t -> pair F (pair h t)"));
	Program::instance_.defun("car", parse("\\z -> fst (snd z)"));
	Program::instance_.defun("cdr", parse("\\z -> snd (snd z)"));

	/* Church numerals.  */
	Program::instance_.defun("unchurch", parse("\\n -> n (\\x -> scalar+ x 1) 0"));
	Program::instance_.defun("+", parse("\\m n f x -> m f (n f x)"));
	Program::instance_.defun("*", parse("\\m n f -> n (m f)"));
	Program::instance_.defun("**", parse("\\m n -> n m"));
	Program::instance_.defun("pred", parse("\\n f x -> n (\\g h -> h (g f)) (\\u -> x) (\\u -> u)"));
	Program::instance_.defun("zero?", parse("\\n -> n (\\x -> F) T"));

	if (!quiet) {
		/* Say hello to the nice user.  */
		std::cout << "Muttonda, duh!  Here's what's in the standard library:" << std::endl;
		help();
		std::cout << "Now have at it!  (Tasty mutton...)" << std::endl;
	}
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
