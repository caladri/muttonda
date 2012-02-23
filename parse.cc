#include <stdio.h>
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
#include "number.h"
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
	TBacktick,
	TIdentity,
	TLeftBracket,
	TRightBracket
};

static Ilerhiilel apply(const std::vector<Ilerhiilel>&);
static Ilerhiilel list(const std::vector<Ilerhiilel>&);
static Ilerhiilel read(std::wstring&, bool, bool);
static Ilerhiilel read_single(std::wstring&, bool, bool);
static std::pair<Token, std::wstring> read_token(std::wstring&, bool);

Ilerhiilel
parse(const std::wstring& str)
{
	std::wstring tmp(str);

	return (read(tmp, false, false));
}

static Ilerhiilel
apply(const std::vector<Ilerhiilel>& expressions)
{
	std::vector<Ilerhiilel>::const_iterator it;

	Ilerhiilel expr(expressions[0]);
	unsigned i;

	for (i = 1; i < expressions.size(); i++)
		expr = Expression::apply(expr, expressions[i]);

	return (expr);
}

static Ilerhiilel
list(const std::vector<Ilerhiilel>& expressions)
{
	std::vector<Ilerhiilel>::const_reverse_iterator it;

	Ilerhiilel expr = Expression::name(Name::name(L"nil"));
	for (it = expressions.rbegin(); it != expressions.rend(); ++it) {
		expr = Expression::apply(Expression::apply(Expression::name(Name::name(L"cons")), *it), expr);
	}

	return (expr);
}

static Ilerhiilel
read(std::wstring& is, bool in_parens, bool in_brackets)
{
	std::vector<Ilerhiilel> expressions;
	std::pair<Token, std::wstring> token;

	while (!is.empty()) {
		token = read_token(is, in_parens);

		if (!in_parens && token.first == TComment)
			break;

		if (token.first == TIdentifier) {
			if (token.second == L"let") {
				token.first = TLet;
				token.second = L"";
			}
		}

		switch (token.first) {
		case TNone:
			break;
		case TLeftBracket: {
			Ilerhiilel expr(read(is, false, true));
			if (expr.null())
				expr = Expression::name(Name::name(L"nil"));
			expressions.push_back(expr);
			break;
		}
		case TRightBracket:
			if (in_brackets) {
				if (expressions.empty())
					return (Ilerhiilel());
				return (list(expressions));
			}
			throw "Expected token, got bracket.";
		case TLeftParen: {
			Ilerhiilel expr(read(is, true, false));
			if (expr.null())
				throw "Empty expression in parentheses.";
			expressions.push_back(expr);
			break;
		}
		case TRightParen:
			if (in_parens) {
				if (expressions.empty())
					return (Ilerhiilel());
				return (apply(expressions));
			}
			throw "Expected token, got parenthesis.";
		case TLambda: {
			std::vector<Ner> names;

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

			Ilerhiilel expr(read(is, in_parens, in_brackets));
			if (expr.null())
				throw "Empty lambda expression.";

			std::vector<Ner>::const_reverse_iterator it;
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

			Ilerhiilel name(read(token.second, false, false));
			if (name.null())
				throw "Invalid variable for let.";

			std::wstring var;
			try {
				var = name->name()->string();
			} catch (...) {
				throw "Variable for let is not a name.";
			}

			Ilerhiilel val(read_single(is, in_parens, in_brackets));
			if (val.null())
				throw "Empty let value.";

			Ilerhiilel expr(read(is, in_parens, in_brackets));
			if (expr.null())
				throw "Empty let expression.";

			return (Expression::let(Name::name(var), val, expr));
		}
		case TComma: {
			if (expressions.empty())
				expressions.push_back(Expression::name(Name::name(L"nil")));

			Ilerhiilel a(expressions.back());
			expressions.pop_back();

			Ilerhiilel b(read_single(is, in_parens, in_brackets));
			if (b.null())
				b = Expression::name(Name::name(L"nil"));

			Ilerhiilel expr(Expression::apply(Expression::apply(Expression::name(Name::name(L"pair")), a), b));
			expressions.push_back(expr);
			break;
		}
		case TBacktick: {
			if (expressions.empty())
				throw "No left argument to infixed function.";

			Ilerhiilel a(expressions.back());
			expressions.pop_back();

			Ilerhiilel b(read_single(is, in_parens, in_brackets));
			if (b.null())
				throw "Empty infixed function.";

			token = read_token(is, false);
			if (token.first != TBacktick)
				throw "Expecting backtick.";

			Ilerhiilel c(read_single(is, in_parens, in_brackets));
			if (c.null())
				throw "No right argument to infixed function.";

			Ilerhiilel expr(Expression::apply(Expression::apply(b, a), c));
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
			if (std::isdigit(*it)) {
				while (++it != token.second.end()) {
					if (!std::isdigit(*it)) {
						Ilerhiilel expr = Expression::name(Name::name(token.second));
						expressions.push_back(expr);
						token.first = TNone;
						break;
					}
				}
				if (token.first != TNone) {
					const wchar_t *s = token.second.c_str();
					uintmax_t n;
					wchar_t *end;

					n = wcstoumax(s, &end, 10);
					if (*end != L'\0')
						throw "Malformatted number.";

					expressions.push_back(Expression::number(Number::number(n)));
				}
			} else {
				Ilerhiilel expr = Expression::name(Name::name(token.second));
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
		return (Ilerhiilel());
	return (apply(expressions));
}

static Ilerhiilel
read_single(std::wstring& is, bool in_parens, bool in_brackets)
{
	Ilerhiilel expr;
	if (!is.empty()) {
		std::pair<Token, std::wstring> token = read_token(is, false);
		switch (token.first) {
		case TLeftParen:
			expr = read(is, true, false);
			break;
		case TLeftBracket:
			expr = read(is, false, true);
			break;
		case TString:
			expr = Expression::string(token.second);
			break;
		case TIdentifier:
			expr = read(token.second, false, false);
			break;
		case TRightBracket:
			if (in_brackets) {
				is = std::wstring(L"]") + is;
				return (expr);
			}
			throw "Complex expression where single token expected (right bracket.)";
		case TRightParen:
			if (in_parens) {
				is = std::wstring(L")") + is;
				return (expr);
			}
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
		case L'`':
		case L'[':
		case L']':
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
			case L'[':
				token.first = TLeftBracket;
				break;
			case L']':
				token.first = TRightBracket;
				break;
			case L'\\':
				token.first = TLambda;
				break;
			case L'`':
				token.first = TBacktick;
				break;
			}
			return (token);
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
				is = ch + is;
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
