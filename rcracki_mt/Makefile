rcracki_mt:
ifeq ($(shell uname -s), Darwin)
	g++ *.cpp -lssl -lcrypto -lpthread -O3 -o rcracki_mt
else
	g++ *.cpp -lssl -lpthread -O3 -o rcracki_mt
endif

clean:
	rm rcracki_mt
