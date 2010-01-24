#include <inttypes.h>

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <tr1/unordered_map>

#include "expression.h"
#include "function.h"
#include "name.h"
#include "parse.h"

static Ref<Expression> apply(const std::vector<Ref<Expression> >&);
static Ref<Expression> read(std::wstring&, bool);
static Ref<Expression> read_single(std::wstring&);
static std::wstring read_token(std::wstring&, bool);

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
	std::wstring token;

	while (!is.empty()) {
		token = read_token(is, in_parens);

		if (!in_parens && token == L"--") 
			break;

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
			std::vector<Ref<Name> > names;

			for (;;) {
				token = read_token(is, in_parens);

				if (token == L"(" || token == L")" || token == L"\"" || token == L"\\" || token == L"\n" || token == L"" || token == L"let" || token == L",")
					throw "Expected variables for lambda.";

				if (token == L"->") {
					if (names.empty())
						throw "Expected at least one variable for lambda.";
					break;
				}

				names.push_back(Name::name(token));
			}

			Ref<Expression> expr(read(is, in_parens));
			if (expr.null())
				throw "Empty lambda expression.";

			std::vector<Ref<Name> >::const_reverse_iterator it;
			for (it = names.rbegin(); it != names.rend(); ++it) {
				expr = Expression::lambda(*it, expr);
			}
			expressions.push_back(expr);
			return (apply(expressions));
		} else if (token == L"let") {
			token = read_token(is, in_parens);

			if (token == L"(" || token == L")" || token == L"\"" || token == L"\\" || token == L"\n" || token == L"" || token == L"let" || token == L"->" || token == L",")
				throw "Expected variable for let.";

			Ref<Expression> name(read(token, false));
			if (name.null())
				throw "Invalid variable for let.";

			try {
				token = name->name()->string();
			} catch (...) {
				throw "Variable for let is not a name.";
			}

			Ref<Expression> val(read_single(is));
			if (val.null())
				throw "Empty let value.";

			Ref<Expression> expr(read(is, in_parens));
			if (expr.null())
				throw "Empty let expression.";

			return (Expression::let(Name::name(token), val, expr));
		} else if (token == L",") {
			if (expressions.empty())
				expressions.push_back(Expression::name(Name::name(L"nil")));

			Ref<Expression> a(expressions.back());
			expressions.pop_back();

			Ref<Expression> b(read_single(is));
			if (b.null())
				b = Expression::name(Name::name(L"nil"));

			Ref<Expression> expr(Expression::apply(Expression::apply(Expression::name(Name::name(L"pair")), a), b));
			expressions.push_back(expr);
		} else if (token == L"\n") {
			break;
		} else if (token != L"" && token[0] == L'"') {
			expressions.push_back(Expression::string(token.substr(1)));
		} else if (token != L"") {
			if (token == L"->")
				throw "Unexpected arrow outside of lambda.";

			std::wstring::iterator it = token.begin();
			bool dollar = *it == L'$';
			if (dollar)
				it++;
			if (it != token.end() && std::isdigit(*it)) {
				while (++it != token.end()) {
					if (!std::isdigit(*it)) {
						Ref<Expression> expr = Expression::name(Name::name(token));
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
						scalar = Expression::apply(Expression::name(Name::name(L"church")), scalar);
					}
					expressions.push_back(scalar);
				}
			} else {
				Ref<Expression> expr = Expression::name(Name::name(token));
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

static Ref<Expression>
read_single(std::wstring& is)
{
	Ref<Expression> expr;
	if (!is.empty()) {
		std::wstring t = read_token(is, false);
		if (t == L"(") {
			expr = read(is, true);
		} else {
			expr = read(t, false);
		}
	}
	return (expr);
}

static std::wstring
read_token(std::wstring& is, bool in_parens)
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
		case L',':
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
			case L'-':
				if (!in_parens && token == L"") {
					return (L"--");
				}
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
				case L'\\':
					if (is.empty())
						throw "Escape at end of string.";
					ch = is[0];
					is.erase(is.begin());

					switch (ch) {
					case L'\\': case L'"':
						token += ch;
						break;
					case 'n':
						token += L'\n';
						break;
					default:
						throw "Unknown escape sequence in string";
					}
					break;
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
