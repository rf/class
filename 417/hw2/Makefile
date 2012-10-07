TESTDRIVERS=$(subst .cpp,.o,$(wildcard test*.cpp))
TESTFILES=$(subst test_,,$(TESTDRIVERS))
TESTS=$(subst .o,,$(TESTFILES))

OBJS=dns.o store.o

.PHONY: test $(TESTS)

CPPFILES=$(wildcard *.cpp)

CFLAGS=-std=gnu++0x -g
CC=g++

all: dns-server

dns-server: $(OBJS) main.o
	$(CC) $(CFLAGS) $(OBJS) main.o -o dns-server

test: $(TESTDRIVERS) $(TESTFILES) $(TESTS)
	@echo -e "\n    \033[38;5;40m ✔ All tests passed \033[0m"

$(TESTS): 
	@echo -e "     Running $@ test"
	@$(CC) $(OBJS) test_$@.o -o $@
	@./$@
	@rm -f $@

.cpp.o: 
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -f *.o
	rm -f dns-server
