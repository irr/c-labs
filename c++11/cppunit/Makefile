#Author: Arvind Pereira

# Modify the includes, sources, headers, executable and
# other flags accordingly!
# Make sure you have installed CPPUnit
# Modify the paths if required so that we can find it
# http://robotics.usc.edu/~ampereir/wordpress/?p=772
INCLUDES = -I ./ -I/usr/local/include
LIBS = -L/usr/lib/ -lm -ldl -lcppunit

CC = g++
DEBUG = -g
LFLAGS = -Wall $(DEBUG)
CFLAGS = -Wall -c

SOURCES = TestRunner.cpp

HEADERS = Lin2dSolver.h \
	TestLin2dSolver.h

OBJ = $(SOURCES:.cpp=.o)

EXE = RunTests

all: $(EXE)

$(EXE):	$(OBJ)
	$(CC) -o $(EXE) $(OBJ) $(LFLAGS) $(LIBS)    

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

tar:
	tar -czf $(EXE).tar.gz $(SOURCES) $(HEADERS) Makefile

clean:
	rm -f $(OBJ)
	rm -f $(EXE)
