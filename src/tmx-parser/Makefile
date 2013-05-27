SOURCES=$(wildcard *.cpp) 
OBJECTS=$(patsubst %.cpp,obj/%.o,$(SOURCES))
EXECUTABLE=tmx-parser.a
EXECDIR=bin/

all: $(SOURCES) $(EXECUTABLE)

release: CXXFLAGS += -O3
release: all

clean:
	$(RM) $(OBJECTS) $(EXECDIR)lib$(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(AR) rcs $(EXECDIR)lib$(EXECUTABLE) $(OBJECTS)

$(OBJECTS): obj/%.o: %.cpp
	$(CXX) -c $< -o $@

