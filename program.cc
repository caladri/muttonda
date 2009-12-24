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
	Program::instance_.defun(ScalarEqual);
	Program::instance_.defun(Print);
	Program::instance_.defun(Show);
	Program::instance_.defun(StringLength);

	/* SK-calculus.  */
	Program::instance_.defun("S", parse("\\x y z -> x z (y z)"));
	Program::instance_.defun("K", parse("\\x y -> x"));
	Program::instance_.defun("I", parse("\\x -> x"));

	/* Curry's BCKW system.  */
	Program::instance_.defun("B", parse("\\x y z -> x (y z)"));
	Program::instance_.defun("C", parse("\\x y z -> x z y"));
	Program::instance_.defun("W", parse("\\x y -> x y y"));

	/* Call-by-value Y combinator.  */
	Program::instance_.defun("Y", parse("\\f -> (\\x -> f (\\y -> x x y)) (\\x -> f (\\y -> x x y))"));

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

	/* Church numerals.  */
	Program::instance_.defun("unchurch", parse("\\n -> n (\\x -> scalar+ x 1) 0"));
	Program::instance_.defun("+", parse("\\m n f x -> m f (n f x)"));
	Program::instance_.defun("*", parse("\\m n f -> n (m f)"));
	Program::instance_.defun("**", parse("\\m n -> n m"));
	Program::instance_.defun("pred", parse("\\n f x -> n (\\g h -> h (g f)) (\\u -> x) (\\u -> u)"));
	Program::instance_.defun("zero?", parse("\\n -> n (\\x -> F) T"));
	Program::instance_.defun("=", parse("\\x y -> not (zero? (church (scalar= (unchurch x) (unchurch y))))"));
	Program::instance_.defun("fact", parse("\\n -> Y (\\f -> \\n -> zero? n $1 (* n (f (pred n)))) n"));

	/* Lists.  */
	Program::instance_.defun("nil", parse("pair T error"));
	Program::instance_.defun("nil?", parse("fst"));
	Program::instance_.defun("cons", parse("\\h t -> pair F (pair h t)"));
	Program::instance_.defun("car", parse("\\z -> fst (snd z)"));
	Program::instance_.defun("cdr", parse("\\z -> snd (snd z)"));

	/* List-processing.  */
	Program::instance_.defun("foldl", parse("\\b z l -> Y (\\f -> \\b z l -> nil? l z (f b (b z (car l)) (cdr l))) b z l"));
	Program::instance_.defun("apply", parse("foldl I I"));
	Program::instance_.defun("append", parse("\\l m -> Y (\\f -> \\l m -> nil? l m (cons (car l) (f (cdr l) m))) l m"));
	Program::instance_.defun("map", parse("\\g l -> Y (\\f -> \\g l -> nil? l nil (cons (g (car l)) (f g (cdr l)))) g l"));
	Program::instance_.defun("print-list", parse("\\l -> print \"[\" nil? l I (print (car l) (nil? (cdr l) I (apply (map (\\x -> print \", \" print x) (cdr l))))) print \"]\" print \"\n\""));

	/* List creation.  */
	Program::instance_.defun("range", parse("\\x p g -> Y (\\f -> \\x p g -> (cons x (not (p x) nil (f (g x) p g)))) x p g"));
	Program::instance_.defun("up", parse("\\x g -> range x (\\y -> T) g"));
	Program::instance_.defun("from", parse("\\x -> up x (\\y -> + y $1)"));
	Program::instance_.defun("upto", parse("\\x y -> range x (\\z -> not (= y z)) (\\z -> + z $1)"));
	Program::instance_.defun("..", parse("upto"));

	/* Function composition.  */
	Program::instance_.defun(".", parse("B"));
	Program::instance_.defun("compose", parse("foldl . I"));

	if (!quiet) {
		/* Say hello to the nice user.  */
		std::cout << "Muttonda, duh!  Here's what's in the standard library:" << std::endl;
		help();
		std::cout << "Now have at it!  (Tasty mutton...)" << std::endl;
	}
}

void
Program::define(const std::string& str, const Ref<Expression>& expr)
{
	if (definitions_.find(str) != definitions_.end())
		definitions_.erase(str);
	definitions_.insert(std::map<std::string, Ref<Expression> >::value_type(str, eval(expr, true)));
}

bool
Program::defined(const std::string& str)
{
	return (definitions_.find(str) != definitions_.end());
}

void
Program::defun(const std::string& str, const std::vector<Name>& vars, const Ref<Expression>& expr)
{
	Ref<Expression> fun(new Expression(Lambda(vars, expr)));

	define(str, fun);
}

void
Program::defun(const SimpleFunction& fun)
{
	defun(fun.name(), new Expression(fun));
}

void
Program::defun(const std::string& str, const Ref<Expression>& expr)
{
	define(str, expr);
}

Ref<Expression>
Program::eval(const Ref<Expression>& expr, bool quiet) const
{
	std::map<std::string, Ref<Expression> >::const_iterator it;

	if (!quiet)
		std::cout << "eval: " << *expr << " =>" << std::endl;

	Ref<Expression> program(expr);

	if (!definitions_.empty()) {
		std::vector<Name> names;

		names.reserve(definitions_.size());
		for (it = definitions_.begin(); it != definitions_.end(); ++it) {
			names.push_back(it->first);
		}
		program = new Expression(Lambda(names, program));
		for (it = definitions_.begin(); it != definitions_.end(); ++it) {
			program = new Expression(program, it->second);
		}
	}

#if defined(VERBOSE) && defined(BAAAAAAA)
	if (!quiet)
		std::cout << "      " << *program << " =>" << std::endl;
#endif

	program = Expression::eval(program);

#if defined(VERBOSE) && defined(BAAAAAAA)
	if (!quiet)
		std::cout << "      " << *program << " =>" << std::endl;
#endif

	return (Expression::simplify(program));
}

void
Program::help(void) const
{
	std::map<std::string, Ref<Expression> >::const_iterator it;

	for (it = definitions_.begin(); it != definitions_.end(); ++it) {
		const Ref<Expression>& expr = it->second;
		std::cout << "\t" << it->first << " = " << *expr << std::endl;
	}
}
