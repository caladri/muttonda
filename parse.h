#ifndef	PARSE_H
#define	PARSE_H

#include "grammar.hh"

Ref<Expression> parse(const std::wstring&);

int yylex(yy::parser::semantic_type *, std::wstring&);

#endif /* !PARSE_H */
