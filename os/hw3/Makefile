CC=gcc
CFLAGS=-g

TESTDRIVERS=$(subst .c,.o,$(wildcard test*.c))
TESTFILES=$(subst test_,,$(TESTDRIVERS))
TESTS=$(subst .o,,$(TESTFILES))

.PHONY: tests $(TESTS)

tests: third.o $(TESTDRIVERS) $(TESTS)

$(TESTS): 
	@echo -e "Compiling $@ test"
	$(CC) $(OBJS) third.o test_$@.o -o $@

.c.o: 
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -rf *.o sort join mutex box

doc:
	marked README.md > README.html
