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
	/* XXX Assumes STDIN_FILENO == std::cin.  Sigh.  */
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
				std::cerr << "Unable to increase stack size limit." << std::endl;
			}
		}
	} else {
		std::cerr << "Unable to get stack size limit." << std::endl;
	}

	try {
		Program::instance_.begin(quiet);
	} catch (const char *msg) {
		std::cerr << "Startup error: " << msg << std::endl;
		exit(1);
	}

	while (std::cin.good()) {
		if (!quiet)
			std::cout << "? ";

		std::string line;
		std::getline(std::cin, line);

		if (line[0] == '#')
			continue;

		if (line == "?" || line == "help") {
			Program::instance_.help();
			continue;
		}

		Ref<Expression> expr;

		try {
			expr = parse(line);
			if (expr.null())
				continue;
		} catch (const char *msg) {
			std::cerr << "Parse error: " << msg << std::endl;
		}
	
		try {
			expr = Program::instance_.eval(expr, quiet);

			std::cout << expr << std::endl;

			Program::instance_.defun("_", expr);
		} catch (const char *msg) {
			std::cerr << "Runtime error: " << msg << std::endl;
		}
	}
}
