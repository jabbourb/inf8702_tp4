src := $(wildcard *.cpp)
obj := $(src:.cpp=.o)
exe := main

LDFLAGS := -lGL -lGLEW -lglfw
CXXFLAGS := -Iglm/glm

all: $(exe)

$(exe): $(obj) libobjparser
	$(CXX) $(LDFLAGS) -o $@ $(obj) ObjParser/libobjparser.so

libobjparser:
	cd ObjParser; $(MAKE) $(MFLAGS)

run: all
	primusrun ./$(exe)

.PHONY: clean
clean:
	rm -f $(exe) $(obj)
	cd ObjParser; $(MAKE) clean
