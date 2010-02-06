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

enum Token {
	TNone,
	TIdentifier,
	TLambda,
	TLet,
	TComma,
	TArrow,
	TLeftParen,
	TRightParen,
	TComment,
	TString,
	TAssign,
	TSemicolon,
	TBacktick,
	TTilde,
};

static Ref<Expression> apply(const std::vector<Ref<Expression> >&);
static Ref<Expression> read(std::wstring&, bool);
static Ref<Expression> read_single(std::wstring&);
static std::pair<Token, std::wstring> read_token(std::wstring&, bool);

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
	std::pair<Token, std::wstring> token;

	while (!is.empty()) {
		token = read_token(is, in_parens);

		if (!in_parens && token.first == TComment)
			break;

		switch (token.first) {
		case TNone:
			break;
		case TLeftParen: {
			Ref<Expression> expr(read(is, true));
			if (expr.null())
				throw "Empty expression in parentheses.";
			expressions.push_back(expr);
			break;
		}
		case TRightParen:
			if (in_parens) {
				if (expressions.empty())
					return (Ref<Expression>());
				return (apply(expressions));
			}
			throw "Expected token, got parenthesis.";
		case TLambda: {
			std::vector<Ref<Name> > names;

			for (;;) {
				token = read_token(is, in_parens);

				switch (token.first) {
				case TArrow:
					if (names.empty())
						throw "Expected at least one variable for lambda.";
					break;
				case TIdentifier:
					names.push_back(Name::name(token.second));
					continue;
				default:
					throw "Expecting variable for lambda.";
				}
				break;
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
		}
		case TLet: {
			token = read_token(is, in_parens);

			switch (token.first) {
			case TIdentifier:
				break;
			default:
				throw "Expected variable for let.";
			}

			Ref<Expression> name(read(token.second, false));
			if (name.null())
				throw "Invalid variable for let.";

			std::wstring var;
			try {
				var = name->name()->string();
			} catch (...) {
				throw "Variable for let is not a name.";
			}

			Ref<Expression> val(read_single(is));
			if (val.null())
				throw "Empty let value.";

			Ref<Expression> expr(read(is, in_parens));
			if (expr.null())
				throw "Empty let expression.";

			return (Expression::let(Name::name(var), val, expr));
		}
		case TComma: {
			if (expressions.empty())
				expressions.push_back(Expression::name(Name::name(L"nil")));

			Ref<Expression> a(expressions.back());
			expressions.pop_back();

			Ref<Expression> b(read_single(is));
			if (b.null())
				b = Expression::name(Name::name(L"nil"));

			Ref<Expression> expr(Expression::apply(Expression::apply(Expression::name(Name::name(L"pair")), a), b));
			expressions.push_back(expr);
			break;
		}
		case TBacktick: {
			if (expressions.empty())
				expressions.push_back(Expression::name(Name::name(L"nil")));

			Ref<Expression> a(expressions.back());
			expressions.pop_back();

			Ref<Expression> b(read_single(is));
			if (b.null())
				b = Expression::name(Name::name(L"cons"));

			token = read_token(is, false);
			if (token.first != TBacktick)
				throw "Expecting backtick.";

			Ref<Expression> c(read_single(is));
			if (c.null())
				c = Expression::name(Name::name(L"nil"));

			Ref<Expression> expr(Expression::apply(Expression::apply(b, a), c));
			expressions.push_back(expr);
			break;
		}
		case TTilde: {
			if (expressions.empty())
				expressions.push_back(Expression::name(Name::name(L"nil")));

			Ref<Expression> a(expressions.back());
			expressions.pop_back();

			Ref<Expression> b(read_single(is));
			if (b.null())
				b = Expression::name(Name::name(L"cons"));

			token = read_token(is, false);
			if (token.first != TTilde)
				throw "Expecting tilde.";

			Ref<Expression> expr(Expression::apply(b, a));
			expressions.push_back(expr);
			break;
		}
		case TString:
			expressions.push_back(Expression::string(token.second));
			break;
		case TArrow:
			throw "Unexpected arrow outside of lambda.";
		case TIdentifier: {
			std::wstring::iterator it = token.second.begin();
			bool dollar = *it == L'$';
			if (dollar)
				it++;
			if (it != token.second.end() && std::isdigit(*it)) {
				while (++it != token.second.end()) {
					if (!std::isdigit(*it)) {
						Ref<Expression> expr = Expression::name(Name::name(token.second));
						expressions.push_back(expr);
						token.first = TNone;
						break;
					}
				}
				if (token.first != TNone) {
					const wchar_t *s = token.second.c_str();
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
				Ref<Expression> expr = Expression::name(Name::name(token.second));
				expressions.push_back(expr);
			}
			break;
		}
		default:
			throw "Unexpected token.";
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
		std::pair<Token, std::wstring> token = read_token(is, false);
		switch (token.first) {
		case TLeftParen:
			expr = read(is, true);
			break;
		case TString:
		case TIdentifier:
			expr = read(token.second, false);
			break;
		default:
			throw "Complex expression where single token desired.";
		}
	}
	return (expr);
}

static std::pair<Token, std::wstring>
read_token(std::wstring& is, bool in_parens)
{
	std::pair<Token, std::wstring> token(TNone, L"");
	wchar_t ch;

	while (!is.empty()) {
		ch = is[0];
		is.erase(is.begin());

		switch (ch) {
		case L' ':
		case L'\t':
			if (token.first != TNone)
				return (token);
			break;
		case L',':
		case L'(':
		case L')':
		case L'\\':
		case L';':
		case L'`':
		case L'~':
			if (token.first != TNone) {
				is = ch + is;
				return (token);
			}

			switch (ch) {
			case L',':
				token.first = TComma;
				break;
			case L'(':
				token.first = TLeftParen;
				break;
			case L')':
				token.first = TRightParen;
				break;
			case L'\\':
				token.first = TLambda;
				break;
			case L';':
				token.first = TSemicolon;
				break;
			case L'`':
				token.first = TBacktick;
				break;
			case L'~':
				token.first = TTilde;
				break;
			}
			return (token);
		case L'<':
			if (is.empty())
				ch = EOF;
			else {
				ch = is[0];
				is.erase(is.begin());
			}

			switch (ch) {
			case EOF:
				if (token.first == TNone)
					token.first = TIdentifier;
				token.second += L'<';
				return (token);
			case L'-':
				if (token.first != TNone) {
					is = std::wstring(L"<-") + is;
					return (token);
				}
				token.first = TAssign;
				return (token);
			default:
				if (token.first == TNone)
					token.first = TIdentifier;
				token.second += L'<';
				token.second += ch;
				break;
			}
			break;
		case L'-':
			if (is.empty())
				ch = EOF;
			else {
				ch = is[0];
				is.erase(is.begin());
			}

			switch (ch) {
			case EOF:
				if (token.first == TNone)
					token.first = TIdentifier;
				token.second += L'-';
				return (token);
			case L'>':
				if (token.first != TNone) {
					is = std::wstring(L"->") + is;
					return (token);
				}
				token.first = TArrow;
				return (token);
			case L'-':
				if (!in_parens && token.first == TNone) {
					token.first = TComment;
					return (token);
				}
			default:
				if (token.first == TNone)
					token.first = TIdentifier;
				token.second += L'-';
				token.second += ch;
				break;
			}
			break;
		case L'"':
			if (token.first != TNone) {
				is = ch + is;
				return (token);
			}
			token.first = TString;
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
						token.second += ch;
						break;
					case 'n':
						token.second += L'\n';
						break;
					default:
						throw "Unknown escape sequence in string";
					}
					break;
				default:
					token.second += ch;
					break;
				}
			}
			/* NOTREACHED */
		default:
			if (token.first == TNone)
				token.first = TIdentifier;
			token.second += ch;
			break;
		}
	}
	return (token);
}
