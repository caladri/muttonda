PROGRAM=mda

SRCS+=	expression.cc
SRCS+=	fe-rep.cc
SRCS+=	function.cc
SRCS+=	name.cc
SRCS+=	number.cc
SRCS+=	parse.cc
SRCS+=	program.cc
SRCS+=	string.cc

CXXFLAGS+=-std=c++11
CXXFLAGS+=-Weverything
CXXFLAGS+=-Wno-switch-enum
CXXFLAGS+=-Wno-global-constructors
CXXFLAGS+=-Wno-exit-time-destructors
CXXFLAGS+=-Wno-padded
CXXFLAGS+=-Wno-unreachable-code
CXXFLAGS+=-Wno-c++98-compat-pedantic
CXXFLAGS+=-Wno-implicit-fallthrough
CXXFLAGS+=-Wno-weak-vtables
CXXFLAGS+=-Wno-missing-noreturn
CXXFLAGS+=-Wno-vla
CXXFLAGS+=-Wno-vla-extension
CXXFLAGS+=-Werror

OBJS+=  $(patsubst %.cc,%.o,$(patsubst %.c,%.o,${SRCS}))

${PROGRAM}: ${OBJS}
	${CXX} ${LDFLAGS} -o $@ ${OBJS} ${LDADD}

.cc.o:
	${CXX} ${CPPFLAGS} ${CXXFLAGS} ${CFLAGS} -c -o $@ $<

.c.o:
	${CC} ${CPPFLAGS} ${CFLAGS} -c -o $@ $<

clean:
	rm -f ${PROGRAM} ${OBJS}
