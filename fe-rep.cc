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

Program Program::instance_;

int
main(void)
{
	/* Built-in functions.  */
	Program::instance_.defun(Church);
	Program::instance_.defun(Define);
	Program::instance_.defun(Eval);
	Program::instance_.defun(ScalarAdd);

	/* SK-calculus.  */
	Program::instance_.defun("S", parse("\\x y z -> x z (y z)"));
	Program::instance_.defun("K", parse("\\x y -> x"));

	/* Booleans.  */
	Program::instance_.defun("T", parse("\\x y -> x"));
	Program::instance_.defun("F", parse("\\x y -> y"));

	/* Boolean logic.  */
	Program::instance_.defun("and", parse("\\m n -> m n m"));
	Program::instance_.defun("or", parse("\\m n -> m m n"));
	Program::instance_.defun("not", parse("\\m -> m F T"));
	Program::instance_.defun("xor", parse("\\m n -> m (n F T) n"));

	/* Church pairs.  */
	Program::instance_.defun("pair", parse("\\x y z -> z x y"));
	Program::instance_.defun("fst", parse("\\p -> p \\x y -> x"));
	Program::instance_.defun("snd", parse("\\p -> p \\x y -> y"));

	/* Lists.  */
	Program::instance_.defun("nil", parse("pair T error"));
	Program::instance_.defun("nil?", parse("fst"));
	Program::instance_.defun("cons", parse("\\h t -> pair F (pair h t)"));
	Program::instance_.defun("car", parse("\\z -> fst (snd z)"));
	Program::instance_.defun("cdr", parse("\\z -> snd (snd z)"));

	/* Church numerals.  */
	Program::instance_.defun("unchurch", parse("\\n -> n (\\x -> scalar+ x 1) 0"));
	Program::instance_.defun("+", parse("\\m n f x -> m f (n f x)"));
	Program::instance_.defun("*", parse("\\m n f -> n (m f)"));
	Program::instance_.defun("**", parse("\\m n -> n m"));
	Program::instance_.defun("pred", parse("\\n f x -> n (\\g h -> h (g f)) (\\u -> x) (\\u -> u)"));
	Program::instance_.defun("zero?", parse("\\n -> n (\\x -> F) T"));

	/* XXX Assumes STDIN_FILENO == std::cin.  Sigh.  */
	if (isatty(STDIN_FILENO))
		Program::instance_.begin();

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
			std::istringstream istr(line);

			Expression expr(Program::instance_.eval(parse(istr), false));

			std::cout << expr << std::endl;

			Program::instance_.defun("_", expr);
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
