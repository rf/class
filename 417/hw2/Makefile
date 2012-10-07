TESTDRIVERS=$(subst .cpp,.o,$(wildcard test*.cpp))
TESTFILES=$(subst test_,,$(TESTDRIVERS))
TESTS=$(subst .o,,$(TESTFILES))

.PHONY: libbitster bitstercli test $(TESTS)

CPPFILES=$(wildcard *.cpp)
TORRENT=../project2.torrent
OUT=out.mp3
SWITCHES=

CFLAGS=-std=gnu++0x
CC=g++

test: $(TESTDRIVERS) $(TESTFILES) $(TESTS)

$(TESTS): 
	@echo "Running $@ test"
	@$(CC) $@.o test_$@.o -o $@
	@./$@
	@rm -f $@

.cpp.o: 
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -f *.o
