#include <iostream>
#include <string>
#include <map>
#include <vector>

#include "expression.h"
#include "function.h"
#include "parse.h"
#include "program.h"

Program Program::instance_;

int
main(void)
{
	/* XXX Assumes STDIN_FILENO == std::cin.  Sigh.  */
	try {
		Program::instance_.begin(!isatty(STDIN_FILENO));
	} catch (const char *msg) {
		if (msg != NULL)
			std::cerr << "Error: " << msg << std::endl;
		exit(1);
	}

	while (std::cin.good()) {
		/* XXX Assumes STDIN_FILENO == std::cin.  Sigh.  */
		if (isatty(STDIN_FILENO))
			std::cout << "? ";

		std::string line;
		std::getline(std::cin, line);

		if (line == "?" || line == "help") {
			Program::instance_.help();
			continue;
		}

		try {
			Expression expr(Program::instance_.eval(parse(line), false));

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
