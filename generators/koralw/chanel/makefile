#
# Makefile for library CHANEL for  HP 9000
#    for KORALW (1996/05/03)
#
SHELL         = /bin/csh
FFLAGS        = -fno-automatic 
FC            = gfortran
AR            = ar
#FC            = fort77
#FFLAGS        = 
#FFLAGS        = +ppu
#FC            = /usr/lib/hf_FORT/f77
#FFLAGS        = -W0,'langlvl(darg(l)),testmode(a,s(2))'
GRACELLIB     = ../grclib
#
CHANELLIB     = $(GRACELLIB)/libchanel.a

OBJS	= $(CHANELLIB)(sminit.o) $(CHANELLIB)(smconf.o) \
          $(CHANELLIB)(smconv.o) $(CHANELLIB)(smcons.o) \
          $(CHANELLIB)(smffv_new2.o) $(CHANELLIB)(smffs.o) \
          $(CHANELLIB)(smvvv.o) $(CHANELLIB)(smsvv.o) \
          $(CHANELLIB)(smssv.o) $(CHANELLIB)(smsss.o) \
          $(CHANELLIB)(smvvvv.o) $(CHANELLIB)(smssvv.o) \
          $(CHANELLIB)(smssss.o) $(CHANELLIB)(smextv.o) \
          $(CHANELLIB)(smintv.o) $(CHANELLIB)(smintf.o) \
          $(CHANELLIB)(smintp.o) $(CHANELLIB)(smintt.o) \
          $(CHANELLIB)(smextf.o) $(CHANELLIB)(smggg.o) \
          $(CHANELLIB)(smgggg.o) $(CHANELLIB)(smprpd.o) \
          $(CHANELLIB)(pol.o) $(CHANELLIB)(spltq.o) \
          $(CHANELLIB)(phaseq.o) \
          $(CHANELLIB)(ffvmm1.o) $(CHANELLIB)(ffvmm2.o) \
          $(CHANELLIB)(ffvmm3.o) $(CHANELLIB)(ffvmm4.o)\
          $(CHANELLIB)(ffvmm5.o) $(CHANELLIB)(ffvmm6.o)\
          $(CHANELLIB)(ffvmm7.o) $(CHANELLIB)(ffvmm8.o)\
          $(CHANELLIB)(ffvmm9.o) $(CHANELLIB)(splt.o) \
          $(CHANELLIB)(vvv.o) $(CHANELLIB)(vvvv.o) \
          $(CHANELLIB)(vvs.o) $(CHANELLIB)(ssv.o) \
          $(CHANELLIB)(pola.o) $(CHANELLIB)(ffs0.o) \
          $(CHANELLIB)(ffs.o) \
	$(CHANELLIB)(smvww.o) $(CHANELLIB)(wwva.o) \
	$(CHANELLIB)(polp.o) $(CHANELLIB)(smintp.o) \
	$(CHANELLIB)(smintt.o) $(CHANELLIB)(smcpol.o)

.f.a:
	$(FC) -c $(FFLAGS) $<
	ar rv $@ $*.o
	/bin/rm -f $*.o

# m.s. 9/3/97 
#all:	$(OBJS)
all:	${CHANELLIB}

${CHANELLIB}:	${OBJS}

clean:
		/bin/rm -f $(CHANELLIB)
#		('cd' $(GRACE_INIDIR);make Clean)
Clean:
		/bin/rm -f $(CHANELLIB)
#		('cd' $(GRACE_INIDIR);make Clean)
