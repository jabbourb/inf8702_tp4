#src := $(wildcard *.cpp)
src := OBJParser.cpp ParserTextAbstrait.cpp StringUtils.cpp Vecteur3.cpp
obj := $(src:.cpp=.o)
exe := libobjparser.so

CXXFLAGS := -fPIC
LDFLAGS := -shared

all: $(exe)

$(exe): $(obj)
	$(CXX) $(LDFLAGS) -o $@ $(obj)

.PHONY: clean
clean:
	rm -f $(exe) $(obj)
