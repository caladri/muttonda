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

	program.defun("S", Lambda("x", Lambda("y", Lambda("z", Expression(Expression(Name("x"), Name("z")), Expression(Name("y"), Name("z")))))));
	program.defun("K", Lambda("x", Lambda("y", Name("x"))));

	program.defun("T", Lambda("x", Lambda("y", Name("x"))));
	program.defun("F", Lambda("x", Lambda("y", Name("y"))));

	program.defun("nil", Lambda("z", Lambda("x", Lambda("y", Name("y")))));
	program.defun("cons", Lambda("x", Lambda("y", Lambda("m", Expression(Expression(Name("m"), Name("x")), Name("y"))))));
	program.defun("car", Lambda("z", Expression(Name("z"), Lambda("x", Lambda("y", Name("x"))))));
	program.defun("cdr", Lambda("z", Expression(Name("z"), Lambda("x", Lambda("y", Name("y"))))));
	program.defun("cond", Lambda("p", Lambda("t", Lambda("f", Expression(Expression(Name("p"), Name("t")), Name("f"))))));

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
