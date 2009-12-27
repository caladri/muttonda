#include <map>
#include <string>
#include <vector>

#include "expression.h"
#include "function.h"
#include "lambda.h"
#include "name.h"
#include "parse.h"

#include "church.h"

static Ref<Expression> apply(const std::vector<Ref<Expression> >&);
static Ref<Expression> read(std::string&, bool);
static std::string read_token(std::string&);

Ref<Expression>
parse(const std::string& str)
{
	std::string tmp(str);
	return (read(tmp, false));
}

static Ref<Expression>
apply(const std::vector<Ref<Expression> >& expressions)
{
	std::vector<Ref<Expression> >::const_iterator it;

	Ref<Expression> expr(expressions[0]);
	unsigned i;

	for (i = 1; i < expressions.size(); i++)
		expr = new Expression(expr, expressions[i]);

	return (expr);
}

static Ref<Expression>
read(std::string& is, bool in_parens)
{
	std::vector<Ref<Expression> > expressions;
	std::map<std::string, Ref<Expression> > token_cache;
	std::string token;

	while (!is.empty()) {
		token = read_token(is);

		if (token == "(") {
			Ref<Expression> expr(read(is, true));
			if (expr.null())
				throw "Empty expression in parentheses.";
			expressions.push_back(expr);
		} else if (token == ")") {
			if (in_parens) {
				if (expressions.empty())
					return (Ref<Expression>());
				return (apply(expressions));
			}
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

			Ref<Expression> expr(read(is, in_parens));
			if (expr.null())
				throw "Empty lambda expression.";
			expressions.push_back(new Expression(Lambda(names, expr)));
			return (apply(expressions));
		} else if (token == "\n") {
			break;
		} else if (token != "" && token[0] == '"') {
			expressions.push_back(new Expression(String(token.substr(1))));
		} else if (token != "") {
			if (token == "->")
				throw "Unexpected arrow outside of lambda.";

			if (!token_cache.empty()) {
				std::map<std::string, Ref<Expression> >::const_iterator it;

				it = token_cache.find(token);
				if (it != token_cache.end()) {
					expressions.push_back(it->second);
					continue;
				}
			}

			std::string::iterator it = token.begin();
			bool dollar = *it == '$';
			if (dollar)
				it++;
			if (it != token.end() && std::isdigit(*it)) {
				while (++it != token.end()) {
					if (!std::isdigit(*it)) {
						Ref<Expression> expr = new Expression(Name(token));
						token_cache[token] = expr;
						expressions.push_back(expr);
						token = "";
						break;
					}
				}
				if (token != "") {
					Ref<Expression> scalar(new Expression(Scalar(atoi(token.c_str() + (dollar ? 1 : 0)))));
					if (dollar) {
						std::vector<Ref<Expression> > args;
						args.push_back(scalar);
						scalar = ChurchBuiltin::function(args);
					}
					token_cache[token] = scalar;
					expressions.push_back(scalar);
				}
			} else {
				Ref<Expression> expr = new Expression(Name(token));
				token_cache[token] = expr;
				expressions.push_back(expr);
			}
		}
	}
	if (in_parens)
		throw "EOL before end of expression in parentheses.";
	if (expressions.empty())
		return (Ref<Expression>());
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
