SOURCES=$(wildcard *.cpp) 
OBJECTS=$(patsubst %.cpp,obj/%.o,$(SOURCES))
EXECUTABLE=tmx-parser.so
EXECDIR=bin/
INSTALL_PATH?=/usr/local/lib/

all: $(SOURCES) $(EXECUTABLE)

install: all
	@cp $(EXECDIR)lib$(EXECUTABLE) $(INSTALL_PATH)
	
uninstall:
	@$(RM) $(INSTALL_PATH)lib$(EXECUTABLE)

release: CXXFLAGS += -O3
release: all

clean:
	@$(RM) $(OBJECTS) $(EXECDIR)lib$(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CXX) -shared -o $(EXECDIR)lib$(EXECUTABLE) $(OBJECTS)

$(OBJECTS): obj/%.o: %.cpp
	$(CXX) -fpic -c $< -o $@

