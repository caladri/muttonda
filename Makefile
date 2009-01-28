PROG_CXX=mda

SRCS+=	expression.cc
SRCS+=	fe-rep.cc
SRCS+=	function.cc
SRCS+=	lambda.cc
SRCS+=	name.cc
SRCS+=	parse.cc
SRCS+=	program.cc
SRCS+=	scalar.cc

WARNS?=	3

NO_MAN=	1

.include <bsd.prog.mk>
