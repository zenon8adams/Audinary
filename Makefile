CC      = g++
CFLAGS  = -std=c++17 -Wall
EXTRAS  = deps/beep.o deps/i18n.o
DEPS    = -lcAudio -lpthread -lhttpserver -lfontconfig
PACKAGE = audinary

all:
	$(MAKE) $(PACKAGE)

$(PACKAGE): source/intermediate driver.cpp
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $(EXTRAS) $(OBJS) -o $@ driver.cpp $(DEPS)

include source/Makefile

.PHONY: all clean