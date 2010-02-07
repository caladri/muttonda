#include <sys/resource.h>

#include <iostream>
#include <string>
#include <map>
#include <set>
#include <vector>

#include <tr1/unordered_map>

#include "expression.h"
#include "function.h"
#include "parse.h"
#include "program.h"

static void usage(void);

int
main(int argc, char *argv[])
{
	bool verbose = false;
	bool interactive;
	int ch;

	interactive = isatty(STDIN_FILENO);

	while ((ch = getopt(argc, argv, "v")) != -1) {
		switch (ch) {
		case 'v':
			verbose = true;
			break;
		default:
			usage();
		}
	}

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
		Program::instance_.begin(!interactive);
	} catch (const char *msg) {
		std::wcerr << "Startup error: " << msg << std::endl;
		exit(1);
	}

	while (std::wcin.good()) {
		if (interactive)
			std::wcout << "? ";

		std::wstring line;
		std::getline(std::wcin, line);

		if (!interactive && verbose) {
			std::wcout << "? " << line << std::endl;
		}

		if (line == L"?") {
			Program::instance_.help(false);
			continue;
		}

		if (line == L"help") {
			Program::instance_.help(true);
			continue;
		}

		Ilerhiilel expr;

		try {
			expr = parse(line);
			if (expr.null())
				continue;
		} catch (const char *msg) {
			std::wcerr << "Parse error: " << msg << std::endl;
			if (interactive || verbose) {
				std::wcerr << "Offending input: " << line << std::endl;
			}
			continue;
		}

		try {
			expr = Program::instance_.eval(expr, !(interactive || verbose));

			if (interactive || verbose) {
				std::wcout << expr << std::endl;
			}
		} catch (const char *msg) {
			std::wcerr << "Runtime error: " << msg << std::endl;
			if (interactive || verbose) {
				std::wcerr << "Offending expression: " << expr << std::endl;
			}
			continue;
		}
	}
}

static void
usage(void)
{
	std::wcerr << "usage: mda [-v]" << std::endl;
}
