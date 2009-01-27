PROG_CXX=mda

SRCS+=	expression.cc
SRCS+=	function.cc
SRCS+=	lambda.cc
SRCS+=	name.cc
SRCS+=	scalar.cc

# Frontend.
SRCS+=	fe-rep.cc
#SRCS+=	fe-test.cc

WARNS?=	3

NO_MAN=	1

.include <bsd.prog.mk>
