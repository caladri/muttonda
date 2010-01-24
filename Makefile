PROG_CXX=mda

SRCS+=	expression.cc
SRCS+=	fe-rep.cc
SRCS+=	function.cc
SRCS+=	grammar.yy
SRCS+=	name.cc
SRCS+=	parse.cc
SRCS+=	program.cc
SRCS+=	scalar.cc
SRCS+=	string.cc

WARNS?=	3

NO_MAN=	1

#CFLAGS+=-DYYERROR_VERBOSE

grammar.cc grammar.hh: grammar.yy
	bison -L c++ -o grammar.cc $>

grammar.o: grammar.cc

parse.h: grammar.hh

fe-rep.cc parse.cc: parse.h

CLEANFILES+=grammar.cc grammar.hh

.include <bsd.prog.mk>

CFLAGS:=${CFLAGS:N-Wsystem-headers}
