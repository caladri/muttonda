#include <inttypes.h>

#include <map>
#include <string>
#include <vector>

#include <tr1/unordered_map>

#include "expression.h"
#include "function.h"
#include "name.h"
#include "parse.h"

static std::wstring read_token(std::wstring&);

Ref<Expression>
parse(const std::wstring& str)
{
	Ref<Expression> expr;
	yy::parser p(&expr, str);
	p.parse();
	return (expr);
}

int
yylex(yy::parser::semantic_type *yylvalp, std::wstring& is)
{
	std::wstring token = read_token(is);

#if 0
	std::wcerr << "token: " << token << std::endl;
#endif

	if (token == L"") {
		return (0);
	}

	if (token == L"let") {
		return (yy::parser::token::LET);
	}

	if (token == L"\\") {
		return (yy::parser::token::LAMBDA);
	}

	if (token == L"(") {
		return (yy::parser::token::LPAREN);
	}

	if (token == L")") {
		return (yy::parser::token::RPAREN);
	}

	if (token == L"->") {
		return (yy::parser::token::ARROW);
	}

	if (token == L"$") {
		return (yy::parser::token::DOLLAR);
	}

	if (token[0] == L'"') {
		token.erase(token.begin());
		yylvalp->token_ = new std::wstring(token);
		return (yy::parser::token::STRING);
	}

	uintmax_t n;
	const wchar_t *s = token.c_str();
	wchar_t *end;

	yylvalp->token_ = new std::wstring(token);

	n = wcstoumax(s, &end, 10);
	if (*end == L'\0') {
		return (yy::parser::token::SCALAR);
	}
	return (yy::parser::token::VARIABLE);
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
		case L'$':
			if (token == L"")
				return (L"$");
			token += L'$';
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
				return (token + L'-');
			case L'>':
				if (token != L"") {
					is = std::wstring(L"->") + is;
					return (token);
				}
				return (L"->");
			case L'-':
				if (token == L"") {
					is.clear();
					return (token);
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
