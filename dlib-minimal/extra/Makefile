# This is the makefile used to build the dlib C++ library's regression test suite
# on Debian Linux using the gcc compiler.

# this is the name of the output executable
TARGET = test

# these are the compile time flags passed to gcc
CFLAGS = -std=c++0x -ggdb  -DDEBUG -DDLIB_NO_GUI_SUPPORT  -I./lib  -Wall

# These are the link time flags passed to gcc
LFLAGS = -lpthread

# The name of the compiler.  If you only have one version of
# gcc installed then you probably want to change this to just g++
CC = nice g++

####################################################
####################################################
#  Here we list all the cpp files we want to compile

SRC = main.cpp
SRC += lib/dlib/all/source.cpp

####################################################

TMP = $(SRC:.cpp=.o)
OBJ = $(TMP:.c=.o)

$(TARGET): $(OBJ)
	@echo Linking $@
	@$(CC) $(LFLAGS) $(OBJ) -o $@
	@echo Build Complete

.cpp.o: $<
	@echo Compiling $<
	@$(CC) -c $(CFLAGS) $< -o $@

clean:
	@rm -f $(OBJ) $(TARGET)
	@echo All object files and binaries removed

dep:
	@echo Running makedepend
	@makedepend -- $(CFLAGS) -- $(SRC) 2> /dev/null
	@echo Completed makedepend

###############################################################################
##########  Stuff from makedepend                                         #####
##########  type make dep at the command line to rebuild the dependencies #####
##########  Also, DON'T edit the contents of this file beyond this line.  #####
###############################################################################

