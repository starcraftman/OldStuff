# Make file for COMP428 assignment 1.

# Declaration to make these phoney targets, means not dependent on files.
.PHONY: all clean

# Dirs for src and build artifacts. There must be a demo directory to build exes.
SRC_DIR=.
EXE_DIR=./demo

# Commands and Flags
CC:=mpicc
RM:=@rm -rf
CFLAGS:=-std=c99
F_DEBUG:=-ggdb -Wall -Werror -Wextra -Winline -pedantic
LIBS:=

# Generic files to clean.
FILES_TO_CLEAN:=*.o *.a *.out *.exe *~ temp.* *.gcov *.gcda *.gcno

# Target executables to build
EXES = \
	${EXE_DIR}/serial \
	${EXE_DIR}/parallel \
	${EXE_DIR}/spawn_master \
	${EXE_DIR}/spawn_slave \
	${EXE_DIR}/llnlParallel \

# Custome rules, define how to link and compile respectively.
${EXE_DIR}/%: %.o 
	${CC} ${LIBS} -o $@ $< 

%.o: ${SRC_DIR}/%.c
	${CC} $(CFLAGS) $(F_DEBUG) -o $@ -c $< 

# Rules to build and clean
all: ${EXES}

clean:
	$(RM) $(EXES) $(FILES_TO_CLEAN)          
