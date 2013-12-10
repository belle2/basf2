#
#  Makefile for NSM2
#

SUBDIRS = corelib b2lib daemon

all::
	for subdir in $(SUBDIRS); do \
	  cd $$subdir; make; cd .. ; \
	done
install::
	for subdir in $(SUBDIRS); do \
	  cd $$subdir; make install; cd .. ; \
	done

clean::
	for subdir in $(SUBDIRS); do \
	  cd $$subdir; make clean; cd .. ; \
	done
