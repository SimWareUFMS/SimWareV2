CC=g++
CFLAGS=-c -Wall -I ~/boost -DNO_DEP_CHECK 
LDFLAGS=-lstdc++
SOURCES=main.cpp DataCenter.cpp JobQueue.cpp SchedulingAlgorithm.cpp Server.cpp Users.cpp VirtualMachine.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=SimWare

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *o SimWare
