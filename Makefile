PROG_CXX=mda

SRCS+=	expression.cc
SRCS+=	fe-rep.cc
SRCS+=	function.cc
SRCS+=	name.cc
SRCS+=	parse.cc
SRCS+=	program.cc
SRCS+=	scalar.cc
SRCS+=	string.cc

WARNS?=	3

NO_MAN=	1

.include <bsd.prog.mk>

CFLAGS:=${CFLAGS:N-Wsystem-headers}
