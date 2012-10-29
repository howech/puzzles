CXX=g++
CXXFLAGS=-Wall -pedantic-errors -g -ggdb
SOURCES=numberOfStates.cc
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=millionSort

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) -lcln

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<