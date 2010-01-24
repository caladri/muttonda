%{
#include <inttypes.h>

#include <iostream>
#include <string>
#include <vector>

#include <tr1/unordered_map>

#include "expression.h"
#include "function.h"

#include "parse.h"

#define	REF(ty, src)	new Ref<ty>(src)

void yy::parser::error(yy::location const& loc, std::string const& str)
{
	std::cerr << loc << ": " << str << std::endl;
	throw str;
}
%}

%defines
%pure_parser
%parse-param { Ref<Expression> *exprp }
%lex-param { std::wstring& is }
%parse-param { std::wstring is }

%token LET
%token LAMBDA
%token ARROW
%token LPAREN
%token RPAREN
%token DOLLAR
%token VARIABLE
%token SCALAR
%token STRING

%union {
	std::wstring *token_;

	Ref<Expression> *expression_;

	std::vector<Ref<Name> > *names_;
	Ref<Name> *name_;

	Scalar *scalar_;

	String *string_;
}

%type<token_> VARIABLE STRING SCALAR

%type<expression_> expression terminal_expression single_expression apply_expressions apply_expression let_expression lambda_expression
%type<names_> variables
%type<name_> variable
%type<scalar_> scalar
%type<string_> string

%start unit

%%

/*
 * XXX In error cases, memory leaks abound!
 */

unit			: expression
			{
				*exprp = *$1;
				delete $1;
			}

expression		: single_expression
			{
				$$ = $1;
			}
			| apply_expressions
			{
				$$ = $1;
			}
			| terminal_expression
			{
				$$ = $1;
			}
			| /* nil */
			{
				$$ = REF(Expression, Ref<Expression>());
			}
			;

terminal_expression	: let_expression
			{
				$$ = $1;
			}
			| lambda_expression
			{
				$$ = $1;
			}
			;

single_expression	: LPAREN expression RPAREN
			{
				$$ = $2;
			}
			| variable
			{
				$$ = REF(Expression, Expression::name(*$1));
				delete $1;
			}
			| DOLLAR scalar
			{
				$$ = REF(Expression, Expression::apply(Expression::name(Name::name(L"church")), Expression::scalar(*$2)));
				delete $2;
			}
			| scalar
			{
				$$ = REF(Expression, Expression::scalar(*$1));
				delete $1;
			}
			| string
			{
				$$ = REF(Expression, Expression::string(*$1));
				delete $1;
			}
			;

apply_expressions	: apply_expression terminal_expression
			{
				$$ = REF(Expression, Expression::apply(*$1, *$2));
				delete $1;
				delete $2;
			}
			| single_expression terminal_expression
			{
				$$ = REF(Expression, Expression::apply(*$1, *$2));
				delete $1;
				delete $2;
			}
			| apply_expression
			{
				$$ = $1;
			}
			;

apply_expression	: apply_expression single_expression
			{
				$$ = REF(Expression, Expression::apply(*$1, *$2));
				delete $1;
				delete $2;
			}
			| single_expression single_expression
			{
				$$ = REF(Expression, Expression::apply(*$1, *$2));
				delete $1;
				delete $2;
			}
			;

let_expression		: LET variable single_expression expression
			{
				$$ = REF(Expression, Expression::let(*$2, *$3, *$4));
				delete $2;
				delete $3;
				delete $4;
			}
			;

lambda_expression	: LAMBDA variables ARROW expression
			{
				std::vector<Ref<Name> > names(*$2);
				delete $2;

				Ref<Expression> expr(*$4);
				delete $4;

				std::vector<Ref<Name> >::const_reverse_iterator it;
				for (it = names.rbegin(); it != names.rend(); ++it) {
					expr = Expression::lambda(*it, expr);
				}
				$$ = REF(Expression, expr);
			}
			;

variables		: variables variable
			{
				$$ = $1;
				$$->push_back(*$2);
				delete $2;
			}
			| variable
			{
				$$ = new std::vector<Ref<Name> >();
				$$->push_back(*$1);
				delete $1;
			}
			;

variable		: VARIABLE
			{
				$$ = REF(Name, Name::name(*$1));
				delete $1;
			}
			;

scalar			: SCALAR
			{
				wchar_t *end;
				uintmax_t n;
				
				n = wcstoumax($1->c_str(), &end, 10);
				if (*end != '\0')
					throw "Invalid scalar!";
				$$ = new Scalar(n);
				delete $1;
			}
			;

string			: STRING
			{
				$$ = new String(*$1);
				delete $1;
			}
			;
