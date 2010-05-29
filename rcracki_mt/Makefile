# authors have been contacted and the code in this file has been approved
# for gpl 2/3
#
# rcracki_mt is a multithreaded implementation and fork of the original 
# RainbowCrack
#
# Copyright Martin Westergaard Jørgensen <martinwj2005@gmail.com>
# Copyright 2009 Daniël Niggebrugge <niggebrugge@fox-it.com>
# Copyright 2009 James Nobis <frt@quelrod.net>
#
# This file is part of racrcki_mt.
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
