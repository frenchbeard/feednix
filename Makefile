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

+install: $(EXECUTABLE)
	@ if [ -d $(INSTDIR)   ]; then \
		cp $(EXECUTABLE) $(INSTDIR); \
		chmod a+x $(INSTDIR)/$(EXECUTABLE); \
		chmod og-w $(INSTDIR)/$(EXECUTABLE); \
		echo "$(EXECUTABLE) is installed in $(INSTDIR)"; \
	else \
		echo "Warning! $(INSTDIR) does not exist"; \
	fi 
