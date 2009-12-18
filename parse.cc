#include <string>
#include <vector>

#include "expression.h"
#include "function.h"
#include "lambda.h"
#include "name.h"
#include "parse.h"

static Expression apply(const std::vector<Expression>&);
static Expression read(std::string&, bool);
static std::string read_token(std::string&);

Expression
parse(const std::string& str)
{
	std::string tmp(str);
	return (read(tmp, false));
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
read(std::string& is, bool in_parens)
{
	std::vector<Expression> expressions;
	std::string token;

	while (!is.empty()) {
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
read_token(std::string& is)
{
	std::string token;
	char ch;

	while (!is.empty()) {
		ch = is[0];
		is.erase(is.begin());

		switch (ch) {
		case ' ':
		case '\t':
			if (token != "")
				return (token);
			break;
		case '(':
		case ')':
		case '\\':
			if (token != "") {
				is = ch + is;
				return (token);
			}
			return (std::string() + ch);
		case '-':
			if (is.empty())
				ch = EOF;
			else {
				ch = is[0];
				is.erase(is.begin());
			}

			switch (ch) {
			case EOF:
				return (token + '-');
			case '>':
				if (token != "") {
					is = std::string("->") + is;
					return (token);
				}
				return ("->");
			default:
				token += '-';
				token += ch;
				break;
			}
			break;
		case '"':
			if (token != "") {
				is = ch + is;
				return (token);
			}
			token = '"';
			for (;;) {
				if (is.empty())
					throw "Unterminated string.";

				ch = is[0];
				is.erase(is.begin());

				switch (ch) {
				case '"':
					return (token);
				default:
					token += ch;
					break;
				}
			}
			/* NOTREACHED */
		default:
			token += ch;
			break;
		}
	}
	return (token);
}
