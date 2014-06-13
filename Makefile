CC = g++
INSTDIR = /usr/bin
CFLAGS = -std=c++11 -O -c -Wall
LIBS = -ljsoncpp -lcurl -lncurses -lpanel -lmenu

SOURCES =  main.cpp FeedlyProvider.cpp CursesProvider.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = feednix 

all: $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE) : $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LIBS)

.cpp.o: *.h
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)

clean :
	-rm -f $(OBJECTS) $(EXECUTABLE)

.PHONY: all clean

install: $(EXECUTABLE)
	@ mkdir -p $(HOME)/.config/feednix && cp config.json $(HOME)/.config/feednix/config.json
