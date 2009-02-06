#include <istream>
#include <sstream>
#include <string>
#include <vector>

#include "expression.h"
#include "function.h"
#include "lambda.h"
#include "name.h"
#include "parse.h"

static Expression apply(const std::vector<Expression>&);
static Expression read(std::istream&, bool);
static std::string read_token(std::istream&);

Expression
parse(std::istream& is)
{
	return (read(is, false));
}

Expression
parse(const std::string& str)
{
	std::istringstream is(str);

	return (parse(is));
}

static Expression
apply(const std::vector<Expression>& expressions)
{
	std::vector<Expression>::const_iterator it;

	if (expressions.empty())
		throw 0;

	Expression expr(expressions[0]);
	unsigned i;

	for (i = 1; i < expressions.size(); i++)
		expr = Expression(expr, expressions[i]);

	return (expr);
}

static Expression
read(std::istream& is, bool in_parens)
{
	std::vector<Expression> expressions;
	std::string token;

	while (is.good()) {
		token = read_token(is);

		if (token == "(") {
			expressions.push_back(read(is, true));
		} else if (token == ")") {
			if (in_parens)
				return (apply(expressions));
			throw "Expected token, got parenthesis.";
		} else if (token == "\\") {
			std::vector<Name> names;

			for (;;) {
				token = read_token(is);

				if (token == "(" || token == ")" || token == "\\" || token == "\n" || token == "")
					throw "Expected variables for lambda.";

				if (token == "->") {
					if (names.empty())
						throw "Expected at least one variable for lambda.";
					break;
				}

				names.push_back(token);
			}

			expressions.push_back(Lambda(names, read(is, in_parens)));

			return (apply(expressions));
		} else if (token == "\n") {
			break;
		} else if (token != "" && token[0] == '"') {
			expressions.push_back(String(token.substr(1)));
		} else if (token != "") {
			if (token == "->")
				throw "Unexpected arrow outside of lambda.";
			std::string::iterator it = token.begin();
			if (std::isdigit(*it)) {
				while (++it != token.end()) {
					if (!std::isdigit(*it)) {
						expressions.push_back(Name(token));
						token = "";
						break;
					}
				}
				if (token != "")
					expressions.push_back(Scalar(atoi(token.c_str())));
			} else {
				expressions.push_back(Name(token));
			}
		}
	}
	if (in_parens)
		throw "EOL before end of expression in parentheses.";
	if (expressions.empty())
		throw 0;
	return (apply(expressions));
}

static std::string
read_token(std::istream& is)
{
	std::string token;
	int ch;

	while (is.good()) {
		ch = is.get();

		switch (ch) {
		case EOF:
			return (token);
		case ' ':
		case '\t':
			if (token != "")
				return (token);
			break;
		case '(':
		case ')':
		case '\\':
		case '\n':
			if (token != "") {
				is.putback(ch);
				return (token);
			}
			return (std::string() + (char)ch);
		case '-':
			ch = is.get();

			switch (ch) {
			case EOF:
				return (token + '-');
			case '>':
				if (token != "") {
					is.putback('>');
					is.putback('-');
					return (token);
				}
				return ("->");
			default:
				token += '-';
				token += (char)ch;
				break;
			}
			break;
		case '"':
			if (token != "") {
				is.putback(ch);
				return (token);
			}
			token = '"';
			for (;;) {
				ch = is.get();

				switch (ch) {
				case EOF:
				case '\n':
					throw "Unterminated string.";
#if 0
				case '\\':
					throw "Invalid string.";
#endif
				case '"':
					return (token);
				default:
					token += (char)ch;
					break;
				}
			}
			/* NOTREACHED */
		default:
			token += (char)ch;
			break;
		}
	}
	return (token);
}
