#include <fstream>
#include <iostream>
#include <sstream>
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
#include "lib.h"

Program::Program(void)
: definitions_()
{ }

Program::~Program()
{ }

void
Program::begin(bool quiet)
{
	/* Built-in functions.  */
	defun(Church);
	defun(Define);
	defun(Defined);
	defun(Eval);
	defun(Load);
	defun(ScalarAdd);
	defun(ScalarEqual);
	defun(Print);
	defun(Show);
	defun(StringLength);

	/* Load main library.  */
	if (!Program::load("init.mda")) {
		throw "Unable to find init library.";
	}

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
	Ref<Expression> evaluated = eval(expr, true);
	definitions_.insert(std::map<std::string, Ref<Expression> >::value_type(str, evaluated));
}

bool
Program::defined(const std::string& str)
{
	return (definitions_.find(str) != definitions_.end());
}

void
Program::defun(const SimpleFunction& fun)
{
	define(fun.name(), new Expression(fun));
}

Ref<Expression>
Program::eval(const Ref<Expression>& expr, bool quiet) const
{
	std::map<std::string, Ref<Expression> >::const_iterator it;

	if (!quiet)
		std::cout << "eval: " << expr << " =>" << std::endl;

	Ref<Expression> program(expr);

	if (!definitions_.empty()) {
		for (it = definitions_.begin(); it != definitions_.end(); ++it) {
			Ref<Expression> bound(program->bind(it->first, it->second));
			if (bound.null())
				bound = program;
			else
				program = bound;
		}
	}

#if defined(VERBOSE) && defined(BAAAAAAA)
	if (!quiet)
		std::cout << "      " << program << " =>" << std::endl;
#endif

	Ref<Expression> evaluated = program->eval();
	if (evaluated.null())
		evaluated = program;

#if defined(VERBOSE) && defined(BAAAAAAA)
	if (!quiet)
		std::cout << "      " << evaluated << " =>" << std::endl;
#endif

	Ref<Expression> simplified = evaluated->simplify();
	if (simplified.null())
		simplified = evaluated;

#if defined(VERBOSE) && defined(BAAAAAAA)
	if (!quiet)
		std::cout << "      " << simplified << " =>" << std::endl;
#endif

	return (simplified);
}

bool
Program::load(const std::string& name)
{
	std::ifstream input;

	if (name[0] != '/') {
		static const char *paths[] = {
			".",
			"lib",
			NULL
		};
		const char **prefixp;

		for (prefixp = paths; *prefixp != NULL; prefixp++) {
			std::string path(*prefixp);

			path += "/" + name;

			input.open(path.c_str());
			if (input.is_open())
				break;
		}
	} else {
		input.open(name.c_str());
	}

	if (!input.is_open()) {
		std::cerr << "Could not open library: " << name << std::endl;
		return (false);
	}

	while (input.good()) {
		std::string line;
		std::getline(input, line);

		if (line[0] == '#')
			continue;

		Ref<Expression> expr;

		try {
			expr = parse(line);
			if (expr.null())
				continue;
		} catch (const char *msg) {
			std::cerr << "Library parse error: " << msg << std::endl;
			return (false);
		}

		try {
			expr = eval(expr, true);
		} catch (const char *msg) {
			std::cerr << "Library untime error: " << msg << std::endl;
			return (false);
		}
	}
	return (true);
}

void
Program::help(void) const
{
	std::map<std::string, Ref<Expression> >::const_iterator it;

	for (it = definitions_.begin(); it != definitions_.end(); ++it)
		std::cout << "\t" << it->first << " = " << it->second << std::endl;
}
