OSNAME = $(shell uname -s)

ifeq ($(OSNAME), Darwin)
	LIBS = -lssl -lcrypto -lpthread
else
	ifeq ($(OSNAME), OpenBSD)
		LIBS = -lssl -lcrypto -lpthread
	else
		LIBS = -lssl -lpthread
	endif
endif

rcracki_mt:
		g++ *.cpp $(LIBS) -O3 -o rcracki_mt

clean:
	rm -f rcracki_mt
