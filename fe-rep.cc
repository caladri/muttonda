#include <sys/resource.h>

#include <iostream>
#include <string>
#include <map>
#include <set>
#include <vector>

#include <tr1/unordered_map>

#include "debugger.h"
#include "expression.h"
#include "function.h"
#include "number.h"
#include "parse.h"
#include "program.h"

static void usage(void);

int
main(int argc, char *argv[])
{
	bool verbose = false;
	std::wstring initlib;
	bool interactive;
	int ch;

	initlib = L"lib/init.mda";
	interactive = isatty(STDIN_FILENO);

	while ((ch = getopt(argc, argv, "l:v")) != -1) {
		switch (ch) {
		case 'l':
			initlib = std::wstring(optarg, optarg + strlen(optarg));
			break;
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
		Program::instance_.begin(initlib, !interactive);
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
			Debugger::instance()->set(expr);
			expr = Program::instance_.eval(expr, !(interactive || verbose));

			if (interactive || verbose) {
				std::wcout << expr << std::endl;
			}
		} catch (const char *msg) {
			std::wcerr << "Runtime error: " << msg << std::endl;
			if (interactive || verbose) {
				Debugger::instance()->show(std::wcerr);
				std::wcerr << "While evaluating of expression: " << expr << std::endl;
			}
			continue;
		}
	}
}

static void
usage(void)
{
	std::wcerr << "usage: mda [-l initlib] [-v]" << std::endl;
}
