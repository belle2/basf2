CC = gcc44
CXX = g++44

CFLAGS = -g -O3 -Wall -fPIC -D_REENTRANT

LIBS = libMGT.so

APPS = boothslb B2LDover1 record-nakao mgt_test 

all: $(APPS)

clean:
	@ echo "<< clearing directry >>"
	@ rm -rf bin/* *~ .*~ \#*~

$(APPS): % : %.cc
	@ echo "<< making $@ >>"
	@ $(CXX) $^ -I.. ../libMGT.so -o bin/$@