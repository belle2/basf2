#
# Makefile for the ttrx driver (eunil@hep.korea.ac.kr)
#
# $Id: Makefile,v 1.1.1.1 2005/02/23 09:44:12 ttrx Exp $
# $Log: Makefile,v $
# Revision 1.1.1.1  2005/02/23 09:44:12  ttrx
# First into cvs
#
#
CC  = gcc
OUTO  = ttrx.o ttrx_fifo.o
USRSRCLINUX = /usr/src/linux-2.4.30

.SUFFIXES	: .o .c

PROGRAMS	: $(OUTO)
all		: $(PROGRAMS)

GENKSYMS = /sbin/genksyms
GENKSYMS += -p smp_

CFLAGS += $(DEBFLAGS)
CFLAGS += -D__KERNEL__ -I$(USRSRCLINUX)/include -Wall 
CFLAGS += -Wstrict-prototypes -Wno-trigraphs -O2 -fno-strict-aliasing  
CFLAGS += -fno-common -fomit-frame-pointer -pipe -mpreferred-stack-boundary=2 
CFLAGS += -march=i686 -DMODULE -DMODVERSIONS -include $(USRSRCLINUX)/include/linux/modversions.h 
CFLAGS += -nostdinc -iwithprefix include -DKBUILD_BASENAME=s_base 
CFLAGS += -DEXPORT_SYMTAB  
CFLAGS += -DDEBUG
CFLAGS += -DGENKSYMS

.c.o      :
	$(CC) $(CFLAGS) -c $<

clean     :
	rm -f *.o *.ver
