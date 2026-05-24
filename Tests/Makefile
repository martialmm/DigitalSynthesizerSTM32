.PHONY: test clean prepare

COMPILE=gcc -c
LINK=gcc
CFLAGS=-I../App/Inc/

PATHU	=./Unity/src/
PATHT	=./testFiles/
PATHSRC	=../App/Src/
PATHO	=./objects/
PATHOUT	=./out/
PATHR	=./results/

# look for every test files and create a corresponding .o file
SRCT 		 = $(wildcard $(PATHT)Test*.c)
TEST_TARGETS = $(patsubst $(PATHT)Test%.c, $(PATHOUT)Test%, $(SRCT))

# used to create a result .txt file
RESULTS      = $(patsubst $(PATHT)Test%.c, $(PATHR)Test%.txt, $(SRCT))

PASSED = `grep -s PASS $(PATHR)*.txt`
FAIL   = `grep -s FAIL $(PATHR)*.txt`
IGNORE = `grep -s IGNORE $(PATHR)*.txt`


# prepare testing and display
test: prepare $(RESULTS)
	@echo "\n-----------------------"
	@echo "PASSED:"
	@echo "-----------------------"
	@echo "$(PASSED)"
	@echo "\n-----------------------"
	@echo "FAILURES:"
	@echo "-----------------------"
	@echo "$(FAIL)"
	@echo "\n-----------------------"
	@echo "IGNORED:"
	@echo "-----------------------"
	@echo "$(IGNORE)"
	@echo "\nDONE"


# execute test files
$(PATHR)Test%.txt: $(PATHOUT)Test%
	-./$< > $@


# Link every object file to create an output file.
# Each test file has its own output file.
$(PATHOUT)Test%: $(PATHO)Test%.o $(PATHO)%.o $(PATHO)unity.o
	$(LINK) $^ -o $@
	
	
# compile test.c files (Testenvelope.c, Testsynthesizer.c etc from test code)
$(PATHO)%.o: $(PATHT)%.c
	$(COMPILE) $(CFLAGS) $< -o $@

# compile src.c files (envelope.c, synthesizer.c etc from the real App source code)
$(PATHO)%.o: $(PATHSRC)%.c
	$(COMPILE) $(CFLAGS) $< -o $@

# compile unity.c
$(PATHO)%.o: $(PATHU)%.c
	$(COMPILE) $(CFLAGS) $< -o $@


# create 'objects' directory + content for generated files.o
# create 'out'     directory + content for generated files.out
prepare:
	mkdir -p $(PATHO)
	mkdir -p $(PATHOUT)
	mkdir -p $(PATHR)


# remove 'objects' directory + content
# remove 'out'     directory + content
# remove 'results' directory + content
clean:
	rm -rf $(PATHR)
	rm -rf $(PATHO)
	rm -rf $(PATHOUT)
	
.PRECIOUS: $(PATHOUT)Test%
.PRECIOUS: $(PATHO)%.o