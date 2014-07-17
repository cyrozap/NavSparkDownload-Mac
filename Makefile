CXX := g++-4.8
CXXFLAGS := -g -ansi -lrt
INCLUDE  := .
PREFIX   := /usr/local
PROGRAM  := NavSparkDownload

all: NavSparkDownload

NavSparkDownload: main.o serial.o download.o
	$(CXX) -o $(PROGRAM) main.o serial.o download.o
main.o: main.cpp
	$(CXX) -I$(INCLUDE) $(CXXFLAGS) -c main.cpp
serial.o: serial.cpp serial.h
	$(CXX) -I$(INCLUDE) $(CXXFLAGS) -c serial.cpp
download.o: download.cpp download.h
	$(CXX) -I$(INCLUDE) $(CXXFLAGS) -c download.cpp

install:
	install -m 0755 $(PROGRAM) $(PREFIX)/bin

clean:
	rm -f *.o

