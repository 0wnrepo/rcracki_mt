SHELL = /bin/sh
OSNAME = $(shell uname -s)
CFLAGS = -O3
LIBS = -lssl -lpthread

LIBS_Darwin = -lcrypto
LIBS_NetBSD = -ldes
LIBS_OpenBSD = -lcrypto

LIBS += ${LIBS_$(OSNAME)}

rcracki_mt:
	g++ -Wall *.cpp $(LIBS) $(CFLAGS) -o rcracki_mt

all: rcracki_mt

clean:
	rm -f rcracki_mt

install:
	install --group=root --owner=root --mode=755 rcracki_mt /usr/local/bin/rcracki_mt

rebuild: clean all

uninstall:
	rm -f /usr/local/bin/rcracki_mt
