SHELL = /bin/sh
OSNAME = $(shell uname -s)
CFLAGS = -O3
LIBS = -lssl -lpthread
BIN = $(DESTDIR)/usr/bin

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
	install -d $(BIN)
	install --group=root --owner=root --mode=755 rcracki_mt $(BIN)

rebuild: clean all

uninstall:
	rm -f $(BIN)/rcracki_mt
