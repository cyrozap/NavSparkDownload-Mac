all:NavSparkDownload
CC = g++
INSTDIR = /usr/lib/i386-linux-gnu
INCLUDE = .
CFLAGS = -c -g -ansi -lrt -m32

NavSparkDownload: main.o serial.o download.o
	$(CC) -o NavSparkDownload32 -m32 main.o serial.o download.o
main.o: main.cpp
	$(CC) -I$(INCLUDE) $(CFLAGS) -c main.cpp
serial.o: serial.cpp serial.h
	$(CC) -I$(INCLUDE) $(CFLAGS) -c serial.cpp
download.o: download.cpp download.h
	$(CC) -I$(INCLUDE) $(CFLAGS) -c download.cpp

clean:
	rm -f main.o serial.o download.o

