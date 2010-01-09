#include <inttypes.h>

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
static Ref<Expression> read(std::wstring&, bool);
static std::wstring read_token(std::wstring&);

Ref<Expression>
parse(const std::wstring& str)
{
	std::wstring tmp(str);
	return (read(tmp, false));
}

static Ref<Expression>
apply(const std::vector<Ref<Expression> >& expressions)
{
	std::vector<Ref<Expression> >::const_iterator it;

	Ref<Expression> expr(expressions[0]);
	unsigned i;

	for (i = 1; i < expressions.size(); i++)
		expr = Expression::apply(expr, expressions[i]);

	return (expr);
}

static Ref<Expression>
read(std::wstring& is, bool in_parens)
{
	std::vector<Ref<Expression> > expressions;
	std::map<std::wstring, Ref<Expression> > token_cache;
	std::wstring token;

	while (!is.empty()) {
		token = read_token(is);

		if (token == L"(") {
			Ref<Expression> expr(read(is, true));
			if (expr.null())
				throw "Empty expression in parentheses.";
			expressions.push_back(expr);
		} else if (token == L")") {
			if (in_parens) {
				if (expressions.empty())
					return (Ref<Expression>());
				return (apply(expressions));
			}
			throw "Expected token, got parenthesis.";
		} else if (token == L"\\") {
			std::vector<Name> names;

			for (;;) {
				token = read_token(is);

				if (token == L"(" || token == L")" || token == L"\\" || token == L"\n" || token == L"")
					throw "Expected variables for lambda.";

				if (token == L"->") {
					if (names.empty())
						throw "Expected at least one variable for lambda.";
					break;
				}

				names.push_back(token);
			}

			Ref<Expression> expr(read(is, in_parens));
			if (expr.null())
				throw "Empty lambda expression.";

			std::vector<Name>::const_reverse_iterator it;
			for (it = names.rbegin(); it != names.rend(); ++it) {
				expr = Expression::lambda(*it, expr);
			}
			expressions.push_back(expr);
			return (apply(expressions));
		} else if (token == L"\n") {
			break;
		} else if (token != L"" && token[0] == L'"') {
			expressions.push_back(Expression::string(token.substr(1)));
		} else if (token != L"") {
			if (token == L"->")
				throw "Unexpected arrow outside of lambda.";

			if (!token_cache.empty()) {
				std::map<std::wstring, Ref<Expression> >::const_iterator it;

				it = token_cache.find(token);
				if (it != token_cache.end()) {
					expressions.push_back(it->second);
					continue;
				}
			}

			std::wstring::iterator it = token.begin();
			bool dollar = *it == L'$';
			if (dollar)
				it++;
			if (it != token.end() && std::isdigit(*it)) {
				while (++it != token.end()) {
					if (!std::isdigit(*it)) {
						Ref<Expression> expr = Expression::name(token);
						token_cache[token] = expr;
						expressions.push_back(expr);
						token = L"";
						break;
					}
				}
				if (token != L"") {
					const wchar_t *s = token.c_str();
					uintmax_t n;
					wchar_t *end;

					if (dollar)
						s++;

					n = wcstoumax(s, &end, 10);
					if (*end != L'\0')
						throw "Malformatted number.";

					Ref<Expression> scalar(Expression::scalar(n));
					if (dollar) {
						std::vector<Ref<Expression> > args;
						args.push_back(scalar);
						scalar = ChurchBuiltin::function(args);
					}
					token_cache[token] = scalar;
					expressions.push_back(scalar);
				}
			} else {
				Ref<Expression> expr = Expression::name(token);
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

static std::wstring
read_token(std::wstring& is)
{
	std::wstring token;
	wchar_t ch;

	while (!is.empty()) {
		ch = is[0];
		is.erase(is.begin());

		switch (ch) {
		case L' ':
		case L'\t':
			if (token != L"")
				return (token);
			break;
		case L'(':
		case L')':
		case L'\\':
			if (token != L"") {
				is = ch + is;
				return (token);
			}
			return (std::wstring() + ch);
		case L'-':
			if (is.empty())
				ch = EOF;
			else {
				ch = is[0];
				is.erase(is.begin());
			}

			switch (ch) {
			case EOF:
				return (token + L'-');
			case L'>':
				if (token != L"") {
					is = std::wstring(L"->") + is;
					return (token);
				}
				return (L"->");
			default:
				token += L'-';
				token += ch;
				break;
			}
			break;
		case L'"':
			if (token != L"") {
				is = ch + is;
				return (token);
			}
			token = L'"';
			for (;;) {
				if (is.empty())
					throw "Unterminated string.";

				ch = is[0];
				is.erase(is.begin());

				switch (ch) {
				case L'"':
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
