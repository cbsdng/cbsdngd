CXXFLAGS += -I. -I/usr/local/include -std=c++17
SRCS != ls -1 src/*.cpp
LDADD = -pthread
PROG_CXX = cbsdngd
MK_MAN = no

.include <bsd.prog.mk>
