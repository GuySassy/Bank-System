# Makefile for the smash program
CC = g++ -pthread
CXXFLAGS = -std=c++11 -Wall -g -Werror -pedantic-errors -DNDEBUG -pthread
CXXLINK = $(CC)
OBJS = bank.o atms.o
RM = rm -f
TARGET = Bank

# Creating the  executable
all: $(OBJS) $(CCC)
	$(CXXLINK) -o $(TARGET) $(OBJS)
	
# Creating the object files
atms.o: atms.cpp atms.h account.h 
bank.o: bank.cpp atms.h account.h

# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.*

# For rapid testing
test:
	make clean; make; echo "\033[1;32m\nStarting Test:\n\033[0m"; ./Bank
