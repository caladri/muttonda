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

	try {
		Program::instance_.begin(quiet);
	} catch (const char *msg) {
		if (msg != NULL)
			std::cerr << "Error: " << msg << std::endl;
		exit(1);
	}

	while (std::cin.good()) {
		if (!quiet)
			std::cout << "? ";

		std::string line;
		std::getline(std::cin, line);

		if (line == "" || line[0] == '#')
			continue;

		if (line == "?" || line == "help") {
			Program::instance_.help();
			continue;
		}

		try {
			Ref<Expression> expr(parse(line));
			
			expr = Program::instance_.eval(expr, quiet);

			std::cout << expr << std::endl;

			Program::instance_.defun("_", expr);
		} catch (const char *msg) {
			if (msg != NULL)
				std::cerr << "Error: " << msg << std::endl;
		} catch (int status) {
			switch (status) {
			case 0:
				continue;
			default:
				std::cerr << "Unexpected status: " << status << std::endl;
			}
		}
	}
}
