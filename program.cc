#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "expression.h"
#include "function.h"
#include "name.h"
#include "number.h"
#include "parse.h"
#include "program.h"

#include "lib.h"

Program Program::instance_;

void
Program::begin(const std::wstring& initlib, bool quiet)
{
	/* Built-in functions.  */
	defun(Define);
	defun(Defined);
	defun(Eval);
	defun(Load);
	defun(Memoize);
	defun(Scalar);
	defun(ScalarAdd);
	defun(ScalarSubtract);
	defun(ScalarMultiply);
	defun(ScalarEqual);
	defun(ScalarLessThan);
	defun(ScalarMultiplyDivide);
	defun(Read);
	defun(Print);
	defun(Show);
	defun(StringAdd);
	defun(StringEqual);
	defun(StringLength);
	defun(StringSplit);

	/* Load main library.  */
	if (!Program::load(initlib)) {
		throw "Unable to find init library.";
	}

	if (!quiet) {
		/* Say hello to the nice user.  */
		std::wcout << "Muttonda, duh!  Here's what's in the standard library:" << std::endl;
		help(false);
		std::wcout << "Now have at it!  (Tasty mutton...)" << std::endl;
	}
}

void
Program::define(const Ner& name, const Ilerhiilel& expr)
{
	if (definitions_.find(name) != definitions_.end())
		definitions_.erase(name);
	Ilerhiilel evaluated = eval(expr, true);
	definitions_.insert(std::map<Ner, Ilerhiilel>::value_type(name, evaluated));
}

bool
Program::defined(const Ner& name)
{
	return (definitions_.find(name) != definitions_.end());
}

void
Program::defun(const Function& fun)
{
	define(fun.name(), Expression::function(fun.clone()));
}

Ilerhiilel
Program::eval(const Ilerhiilel& expr, bool quiet) const
{
	std::map<Ner, Ilerhiilel>::const_iterator it;

	if (!quiet)
		std::wcout << "eval: " << expr << " =>" << std::endl;

	Ilerhiilel program(expr);

	Ilerhiilel evaluated;
	if (!program->free()) {
		/*
		 * No free variables, not using any builtins, is pure, can be
		 * memoized.
		 */
		evaluated = program->eval(true);
		if (evaluated.null())
			evaluated = program;
	} else {
		if (!definitions_.empty() && program->free()) {
			for (it = definitions_.begin(); it != definitions_.end(); ++it) {
				if (!program->free(it->first))
					continue;
				program = program->bind(it->first, it->second);
				if (program.null())
					throw "Failed to bind free variable.";
				if (!program->free())
					break;
			}
		}

#if defined(VERBOSE) && defined(BAAAAAAA)
		if (!quiet)
			std::wcout << "      " << program << " =>" << std::endl;
#endif

		evaluated = program->eval(program->pure());
		if (evaluated.null())
			evaluated = program;
	}

#if defined(VERBOSE) && defined(BAAAAAAA)
	if (!quiet)
		std::wcout << "      " << evaluated << " =>" << std::endl;
#endif

	return (evaluated);
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

		Ilerhiilel expr;

		try {
			expr = parse(line);
			if (expr.null())
				continue;
		} catch (const char *msg) {
			std::wcerr << "Library parse error: " << msg << std::endl;
			std::wcerr << "At line: " << line << std::endl;
			return (false);
		}

		try {
			expr = eval(expr, true);
		} catch (const char *msg) {
			std::wcerr << "Library runtime error: " << msg << std::endl;
			std::wcerr << "In expression: " << expr << std::endl;
			return (false);
		}
	}
	return (true);
}

void
Program::help(bool verbose) const
{
	std::map<Ner, Ilerhiilel>::const_iterator it;

	for (it = definitions_.begin(); it != definitions_.end(); ++it) {
		if (verbose) {
			std::wcout << "\t" << it->first << " = " << it->second << std::endl;
		} else {
			if (it != definitions_.begin())
				std::wcout << " ";
			std::wcout << it->first;
		}
	}
	if (!verbose)
		std::wcout << std::endl;
}
