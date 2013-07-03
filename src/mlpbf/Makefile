CXXFLAGS=-std=c++0x -Wall
CPPFLAGS=-I../tmx-parser
LDFLAGS=-L../tmx-parser/bin -ltinyxml -ltmx-parser -llua5.2 -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
SOURCES=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp,obj/%.o,$(SOURCES))
EXECUTABLE=budding-friendships
EXECDIR=../../

all: $(SOURCES) $(EXECUTABLE)

debug: CXXFLAGS += -g
debug: all

release: CXXFLAGS += -O3
release: all

clean:
	@$(RM) $(OBJECTS) $(EXECDIR)$(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CXX) $(OBJECTS) -o $(EXECDIR)$(EXECUTABLE) $(LDFLAGS)

$(OBJECTS): obj/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $< -o $@

