# errorhandling for neurobayes library and include paths

ifndef NEUROBAYES_LIB
$(error Library path NEUROBAYES_LIB is not set)
endif

ifndef NEUROBAYES_INC
$(error Include path NEUROBAYES_INC is not set)
endif

# print neurobayes paths

$(info NeuroBayes library path (NEUROBAYES_LIB) -> $(NEUROBAYES_LIB))
$(info NeuroBayes include path (NEUROBAYES_INC) -> $(NEUROBAYES_INC))

# build executable

CC = g++
CFLAGS = -c -Wall
LDFLAGS = -L$(NEUROBAYES_LIB) -lNeuroBayesTeacherCPP -I$(NEUROBAYES_INC)
SOURCES = NeuroBayesTrainer.cc
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = NeuroBayesTrainer

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(EXECUTABLE)
