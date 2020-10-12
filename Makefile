PREFIX ?= /usr/local
LOCALBASE ?= /usr/local
BINDIR = ${PREFIX}/bin
MANDIR = ${PREFIX}/man/man
LIBDIR = ${PREFIX}/lib
INCLUDEDIR = ${PREFIX}/include/cbsdng

CXXFLAGS += -I. -I${LOCALBASE}/include -std=c++17
LDADD = -pthread
PROG_CXX = cbsdngd
MK_MAN = no
SRCS != ls -1 src/*.cpp

MAININCS = cbsdng/message.h
MAININCSDIR = ${INCLUDEDIR}
DAEMONINCS != ls -1 cbsdng/daemon/*.h
DAEMONINCSDIR = ${INCLUDEDIR}/daemon
INCSGROUPS = MAININCS DAEMONINCS

beforeinstall:
	${INSTALL} -d -m 0755 ${DESTDIR}${INCLUDEDIR}/daemon

.include <bsd.prog.mk>
