PROG = Bakery
EXEC = $(PROG)
sources = $(PROG).cpp

CXX = g++
RM = rm -f

CFLAGS = -std=c++11 -Wall -Wextra
LDFLAGS = -lrt -pthread -lpcap -lsimlib

OBJFILES = $(sources:.c=.o)

.PHONY : all

all : $(EXEC)

%.o : %.c
	$(CXX) $(CFLAGS) -c $< -o $@

$(EXEC) : $(OBJFILES)
	$(CXX) $(CFLAGS) -o $@ $(OBJFILES) $(LDFLAGS)

clean:
	$(RM) *.o

run:
	./$(EXEC) 100 4

cleanall: clean
	$(RM) $(EXEC)