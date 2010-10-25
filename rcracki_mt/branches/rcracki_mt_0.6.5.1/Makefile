# rcracki_mt is a multithreaded implementation and fork of the original 
# RainbowCrack
#
# Copyright Martin Westergaard Jørgensen <martinwj2005@gmail.com>
# Copyright 2009, 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
# Copyright 2009, 2010 James Nobis <frt@quelrod.net>
#
# This file is part of rcracki_mt.
#
# rcracki_mt is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# rcracki_mt is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with rcracki_mt.  If not, see <http://www.gnu.org/licenses/>.

SHELL = /bin/sh
BIN = $(DESTDIR)/usr/bin
CC = g++
OPTIMIZATION = -O3 -mtune=native
CFLAGS = -Wall -ansi $(OPTIMIZATION) -c $(DEBUG)
LFLAGS = -Wall -ansi -filt $(OPTIMIZATION) $(DEBUG)
LIBS = -lcrypto -lpthread
OBJS = BaseRTReader.o ChainWalkContext.o ChainWalkSet.o CrackEngine.o fast_md5.o HashAlgorithm.o HashRoutine.o HashSet.o lm2ntlm.o md4.o MemoryPool.o Public.o RainbowCrack.o rcrackiThread.o RTI2Reader.o
#sha1.o
WARNALL = -Wextra -Wunused-macros -Wunsafe-loop-optimizations -Wundef -Woverlength-strings -Wdisabled-optimization -Wformat-extra-args -Wformat-security -Winline

OSNAME = $(shell uname -s)
# apparently --string-debug works on Linux, OpenBSD, NetBSD, and FreeBSD
# but not on OSX so to the short name -S
STRIP = $(shell which strip) -S

#LIBS_Darwin = -lcrypto
LIBS_NetBSD = -ldes
#LIBS_OpenBSD = -lcrypto

LIBS += ${LIBS_$(OSNAME)}

all: rcracki_mt strip

rcracki_mt: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) $(LIBS) -o rcracki_mt

clean:
	rm -f *.o *.gcno *.gcda *.gcov rcracki_mt

debug: DEBUG += -DDEBUG -g
debug: rcracki_mt

debugall: DEBUG += -DDEBUG -g $(WARNALL)
debugall: rcracki_mt

dumpasmall: DEBUG += -S -fverbose-asm
dumpasmall: rcracki_mt

dumpasmprofileuseall: DEBUG += -S -fverbose-asm
dumpasmprofileuseall: profileuseall

debugprofileuseall: DEBUG += -fprofile-use
debugprofileuseall: debugall

m32: DEBUG += -m32
m32: rcracki_mt

m32debug: DEBUG += -m32
m32debug: debug

m32debugall: DEBUG += -m32
m32debugall: debugll

m32dumpasmall: DEBUG += -m32
m32dumpasmall: dumpasmall

m32dumpasmprofileuseall: DEBUG += -S -fverbose-asm -m32
m32dumpasmprofileuseall: profileuseall

m32profilegenall: DEBUG += -m32
m32profilegenall: profilegenall

m32profileuseall: DEBUG += -m32
m32profileuseall: profileuseall

profilegenall: DEBUG += -fprofile-generate $(WARNALL)
profilegenall: rcracki_mt

profileuseall: DEBUG += -fprofile-use $(WARNALL)
profileuseall: rcracki_mt

gcovall: DEBUG += -DDEBUG -g -fprofile-arcs -ftest-coverage
gcovall: rcracki_mt

gprofall: DEBUG += -DDEBUG -g -pg
gprofall: rcracki_mt

install:
	install -d $(BIN)
	install --group=root --owner=root --mode=755 rcracki_mt $(BIN)

rebuild: clean all

strip:
	$(STRIP) rcracki_mt

uninstall:
	rm -f $(BIN)/rcracki_mt

BaseRTReader.o: BaseRTReader.h BaseRTReader.cpp
	 $(CC) $(CFLAGS) BaseRTReader.cpp

ChainWalkContext.o: ChainWalkContext.h ChainWalkContext.cpp HashRoutine.h Public.h
	 $(CC) $(CFLAGS) ChainWalkContext.cpp

ChainWalkSet.o: Public.h ChainWalkSet.h ChainWalkSet.cpp
	 $(CC) $(CFLAGS) ChainWalkSet.cpp

CrackEngine.o: CrackEngine.h CrackEngine.cpp Public.h HashSet.h ChainWalkContext.h MemoryPool.h ChainWalkSet.h rcrackiThread.h RTI2Reader.h
	 $(CC) $(CFLAGS) CrackEngine.cpp

fast_md5.o: fast_md5.h fast_md5.cpp global.h
	 $(CC) $(CFLAGS) fast_md5.cpp

#HashAlgorithm.o: HashAlgorithm.h HashAlgorithm.cpp Public.h fast_md5.h md4.h sha1.h
HashAlgorithm.o: HashAlgorithm.h HashAlgorithm.cpp Public.h fast_md5.h md4.h
	 $(CC) $(CFLAGS) HashAlgorithm.cpp

HashRoutine.o: HashRoutine.h HashRoutine.cpp global.h HashAlgorithm.h
	 $(CC) $(CFLAGS) HashRoutine.cpp

HashSet.o: HashSet.h HashSet.cpp Public.h
	 $(CC) $(CFLAGS) HashSet.cpp

lm2ntlm.o: lm2ntlm.h lm2ntlm.cpp Public.h md4.h
	 $(CC) $(CFLAGS) lm2ntlm.cpp

md4.o: md4.h md4.cpp global.h
	 $(CC) $(CFLAGS) md4.cpp

MemoryPool.o: MemoryPool.h MemoryPool.cpp Public.h global.h
	 $(CC) $(CFLAGS) MemoryPool.cpp

Public.o: Public.h Public.cpp global.h
	 $(CC) $(CFLAGS) Public.cpp

RainbowCrack.o: RainbowCrack.cpp CrackEngine.h lm2ntlm.h
	 $(CC) $(CFLAGS) RainbowCrack.cpp

rcrackiThread.o: rcrackiThread.h rcrackiThread.cpp ChainWalkContext.h Public.h HashSet.h
	 $(CC) $(CFLAGS) rcrackiThread.cpp

RTI2Reader.o: RTI2Reader.h RTI2Reader.cpp BaseRTReader.h
	$(CC) $(CFLAGS) RTI2Reader.cpp

#sha1.o: sha1.h sha1.cpp global.h
#	 $(CC) $(CFLAGS) sha1.cpp
