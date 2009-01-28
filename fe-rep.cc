#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <vector>

#include "expression.h"
#include "function.h"
#include "lambda.h"
#include "name.h"
#include "parse.h"
#include "program.h"

#include "church.h"

static Program program;

struct DefineBuiltin {
	static std::string name(void)
	{
		return ("define");
	}

	static Expression function(const std::vector<Expression>& expressions)
	{
		Expression var = expressions[0];
		Name name = var.name();

		program.defun(name.str(), expressions[1]);
		return (expressions[1]);
	}
};

static Builtin<DefineBuiltin> Define(2);

int
main(void)
{
	/* Load some useful library functions.  */
	program.defun("define", Define);

	program.defun("S", parse("\\x -> \\y -> \\z -> x z (y z)"));
	program.defun("K", parse("\\x -> \\y -> x"));

	program.defun("T", parse("\\x -> \\y -> x"));
	program.defun("F", parse("\\x -> \\y -> y"));

	program.defun("nil", parse("\\z -> \\x -> \\y -> y"));
	program.defun("cons", parse("\\x -> \\y -> \\m -> m x y"));
	program.defun("car", parse("\\z -> z (\\x -> \\y -> x)"));
	program.defun("cdr", parse("\\z -> z (\\x -> \\y -> y)"));
	program.defun("cond", parse("\\p -> \\t -> \\f -> p t f"));

	program.defun("church", Church);
	program.defun("unchurch", unchurch);
	program.defun("+", plus);
	program.defun("*", mult);
	program.defun("**", expn);

	/* XXX Assumes STDIN_FILENO == std::cin.  Sigh.  */
	if (isatty(STDIN_FILENO))
		program.begin();

	while (std::cin.good()) {
		/* XXX Assumes STDIN_FILENO == std::cin.  Sigh.  */
		if (isatty(STDIN_FILENO))
			std::cout << "? ";

		std::string line;
		std::getline(std::cin, line);
		std::istringstream istr(line);

		try {
			Expression expr(program.eval(parse(istr)));

			std::cout << expr << std::endl;

			program.defun("_", expr);
		} catch (const char *msg) {
			if (msg != NULL)
				std::cerr << "Error: " << msg << std::endl;
		} catch (int status) {
			switch (status) {
			case 0:
				continue;
			case EOF:
				std::cerr << "EOF." << std::endl;
				return (0);
			default:
				std::cerr << "Unexpected status: " << status << std::endl;
			}
		}
	}
}
