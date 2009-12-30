#include <sys/resource.h>

#include <iostream>
#include <string>
#include <map>
#include <vector>

#include "expression.h"
#include "function.h"
#include "lambda.h"
#include "parse.h"
#include "program.h"

Program Program::instance_;

int
main(void)
{
	/* XXX Assumes STDIN_FILENO == std::wcin.  Sigh.  */
	bool quiet = !isatty(STDIN_FILENO);

	/*
	 * Up the stack size.
	 */
	struct rlimit rlim;
	int rv = ::getrlimit(RLIMIT_STACK, &rlim);
	if (rv == 0) {
		if (rlim.rlim_cur < rlim.rlim_max) {
			rlim.rlim_cur = rlim.rlim_max;

			rv = ::setrlimit(RLIMIT_STACK, &rlim);
			if (rv == -1) {
				std::wcerr << "Unable to increase stack size limit." << std::endl;
			}
		}
	} else {
		std::wcerr << "Unable to get stack size limit." << std::endl;
	}

	try {
		Program::instance_.begin(quiet);
	} catch (const char *msg) {
		std::wcerr << "Startup error: " << msg << std::endl;
		exit(1);
	}

	while (std::wcin.good()) {
		if (!quiet)
			std::wcout << "? ";

		std::wstring line;
		std::getline(std::wcin, line);

		if (line[0] == '#')
			continue;

		if (line == L"?" || line == L"help") {
			Program::instance_.help();
			continue;
		}

		Ref<Expression> expr;

		try {
			expr = parse(line);
			if (expr.null())
				continue;
		} catch (const char *msg) {
			std::wcerr << "Parse error: " << msg << std::endl;
		}
	
		try {
			expr = Program::instance_.eval(expr, quiet);

			std::wcout << expr << std::endl;

			Program::instance_.define(L"_", expr);
		} catch (const char *msg) {
			std::wcerr << "Runtime error: " << msg << std::endl;
		}
	}
}
