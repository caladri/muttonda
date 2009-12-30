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
	defun(Read);
	defun(Print);
	defun(Show);
	defun(StringLength);

	/* Load main library.  */
	if (!Program::load(L"init.mda")) {
		throw "Unable to find init library.";
	}

	if (!quiet) {
		/* Say hello to the nice user.  */
		std::wcout << "Muttonda, duh!  Here's what's in the standard library:" << std::endl;
		help();
		std::wcout << "Now have at it!  (Tasty mutton...)" << std::endl;
	}
}

void
Program::define(const std::wstring& str, const Ref<Expression>& expr)
{
	if (definitions_.find(str) != definitions_.end())
		definitions_.erase(str);
	Ref<Expression> evaluated = eval(expr, true);
	definitions_.insert(std::map<std::wstring, Ref<Expression> >::value_type(str, evaluated));
}

bool
Program::defined(const std::wstring& str)
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
	std::map<std::wstring, Ref<Expression> >::const_iterator it;

	if (!quiet)
		std::wcout << "eval: " << expr << " =>" << std::endl;

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
		std::wcout << "      " << program << " =>" << std::endl;
#endif

	Ref<Expression> evaluated = program->eval();
	if (evaluated.null())
		evaluated = program;

#if defined(VERBOSE) && defined(BAAAAAAA)
	if (!quiet)
		std::wcout << "      " << evaluated << " =>" << std::endl;
#endif

	Ref<Expression> simplified = evaluated->simplify();
	if (simplified.null())
		simplified = evaluated;

#if defined(VERBOSE) && defined(BAAAAAAA)
	if (!quiet)
		std::wcout << "      " << simplified << " =>" << std::endl;
#endif

	return (simplified);
}

bool
Program::load(const std::wstring& name)
{
	static const wchar_t *paths[] = {
		L"",
		L".",
		L"lib",
		NULL
	};
	const wchar_t **prefixp;
	std::wifstream input;

	for (prefixp = paths; *prefixp != NULL; prefixp++) {
		std::wstring path(*prefixp);

		path += L"/" + name;

		char buf[path.size() + 1];

		wcstombs(buf, path.c_str(), sizeof buf); /* XXX Check!  */

		input.open(buf);
		if (input.is_open())
			break;

		if (name[0] == L'/')
			break;
	}

	if (!input.is_open()) {
		std::wcerr << "Could not open library: " << name << std::endl;
		return (false);
	}

	while (input.good()) {
		std::wstring line;
		std::getline(input, line);

		if (line[0] == '#')
			continue;

		Ref<Expression> expr;

		try {
			expr = parse(line);
			if (expr.null())
				continue;
		} catch (const char *msg) {
			std::wcerr << "Library parse error: " << msg << std::endl;
			return (false);
		}

		try {
			expr = eval(expr, true);
		} catch (const char *msg) {
			std::wcerr << "Library untime error: " << msg << std::endl;
			return (false);
		}
	}
	return (true);
}

void
Program::help(void) const
{
	std::map<std::wstring, Ref<Expression> >::const_iterator it;

	for (it = definitions_.begin(); it != definitions_.end(); ++it)
		std::wcout << "\t" << it->first << " = " << it->second << std::endl;
}
