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
	Program::instance_.defun(Defined);
	Program::instance_.defun(Eval);
	Program::instance_.defun(ScalarAdd);
	Program::instance_.defun(StringLength);

	/* SK-calculus.  */
	Program::instance_.defun("S", parse("\\x y z -> x z (y z)"));
	Program::instance_.defun("K", parse("\\x y -> x"));
	Program::instance_.defun("I", parse("\\x -> x"));

	/* Curry's BCKW system.  */
	Program::instance_.defun("B", parse("\\x y z -> x (y z)"));
	Program::instance_.defun("C", parse("\\x y z -> x z y"));
	Program::instance_.defun("W", parse("\\x y -> x y y"));

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

	/* List-processing.  */
	Program::instance_.defun("foldl", parse("\\b z l -> (\\f -> f f) (\\f -> \\b z l -> nil? l z (f f b (b z (car l)) (cdr l))) b z l"));
	Program::instance_.defun("apply", parse("foldl I I"));

	/* Function composition.  */
	Program::instance_.defun(".", parse("B"));
	Program::instance_.defun("compose", parse("foldl . I"));

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

bool
Program::defined(const std::string& str)
{
	return (definitions_.find(str) != definitions_.end());
}

void
Program::defun(const std::string& str, const std::vector<Name>& vars, const Expression& expr)
{
	Expression fun(Lambda(vars, expr));

	define(str, fun);
}

void
Program::defun(const SimpleFunction& fun)
{
	defun(fun.name(), fun);
}

void
Program::defun(const std::string& str, const Expression& expr)
{
	define(str, expr);
}

Expression
Program::eval(const Expression& expr, bool quiet) const
{
	std::map<std::string, Expression>::const_iterator it;
	std::vector<Name> names;

	if (!quiet)
		std::cout << "eval: " << expr << " =>" << std::endl;

	Expression program(expr);

	if (!definitions_.empty()) {
		names.reserve(definitions_.size());
		for (it = definitions_.begin(); it != definitions_.end(); ++it) {
			names.push_back(it->first);
		}
		program = Expression(Lambda(names, program));
		for (it = definitions_.begin(); it != definitions_.end(); ++it) {
			program = Expression(program, it->second);
		}
	}

#if defined(VERBOSE) && defined(BAAAAAAA)
	if (!quiet)
		std::cout << "      " << program << " =>" << std::endl;
#endif

	program = program.eval();

#if defined(VERBOSE) && defined(BAAAAAAA)
	if (!quiet)
		std::cout << "      " << program << " =>" << std::endl;
#endif

	return (program.simplify());
}

void
Program::help(void) const
{
	std::map<std::string, Expression>::const_iterator it;

	for (it = definitions_.begin(); it != definitions_.end(); ++it) {
		std::cout << "\t" << it->first << " = " << it->second << std::endl;
	}
}
