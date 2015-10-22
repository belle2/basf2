SRCS = $(wildcard *.cxx)
TRGS = $(patsubst %.cxx, ../bin/%, $(wildcard *.cxx))

all: $(TRGS)

ROOTLIBS := $(shell root-config --glibs --libs)  -lMinuit -lThread
ROOTINCLUDES := $(shell root-config --prefix)/include
ROOTCFLAGS := $(shell root-config --cflags)

FLAGS = -g -Wall -DUNIX $(ROOTCFLAGS)
LIBS := $(ULIBS) -L/usr/lib64 -lpthread $(ROOTLIBS)
INCS = -I$(ROOTINCLUDES)

$(TRGS) : $(SRCS)

../bin/% : %.cxx
	g++ $(FLAGS) $(INCS) -o $@ $< $(LIBS)

clean:
	rm -f $(TRGS)
