EXE = babayaga
PAW = -lpawlib -lpacklib -lmathlib
CERNLIB = -L/cern/pro/lib
PAW =
CERNLIB =
PAWyn = npaw
PAWyn = ypaw
# or 'ypaw' if you use ntuple PAW events (in which case comment the two lines above)

F77 = gfortran
# tune for your processor
FFLAGS = -O3
# aggressive flags
#FFLAGS = -Ofast -march=native -mtune=native
#-v
#FFLAGS = -g

FPCHECK =

EXTRADEPS = #Makefile

VPHLMNT=vp_hlmnt_v2_1
VPHLMNT=vp_hlmnt_v2_1_1
VPHLMNT=vp_hlmnt_v2_2

OBJECTS = main.o cuts.o sv.o matrix_model.o mapmomenta.o\
          routines.o sampling.o phasespace.o distributions.o $(VPHLMNT).o\
          hadr5n12.o hadr5n.o ranlux.o userinterface.o intpl.o Rteubner.o\
          Acp.o Aint.o Asu3.o interface.o mtx_eeenudbb.o $(PAWyn).o

F77 += $(FFLAGS)

default: $(EXE)

SAVEDIR = release
RELEASEDIR = BabaYaga
pack: # use only to release BABAYAGA
	mkdir -p $(RELEASEDIR)/form &&\
        cp -r Makefile README input-test-pack shared.inc \
        main.f cuts.f sv.f matrix_model.f mapmomenta.f vpol_novosibirsk.dat\
        routines.f sampling.f phasespace.f distributions.f $(VPHLMNT).f\
        hadr5n12.f hadr5n.f ranlux.f userinterface.f intpl.f Rteubner.f\
        Acp.f Aint.f Asu3.f interface.f mtx_eeenudbb.f npaw.f ypaw.f\
        $(RELEASEDIR) &&\
	cp form/*.f $(RELEASEDIR)/form/ &&\
	mkdir $(SAVEDIR) ;\
	tar cjvf $(SAVEDIR)/babayaga.tar.bz2 $(RELEASEDIR)/ &&\
	rm -rf $(RELEASEDIR)

clean:
	rm -f $(OBJECTS) *.a
deepclean:
	rm -f $(OBJECTS) *.a $(EXE) *~ form/*~

# source files
main.o: main.f $(EXTRADEPS)
	$(F77) -c main.f
cuts.o: cuts.f $(EXTRADEPS)
	$(F77) -c cuts.f
matrix_model.o: matrix_model.f  form/formme.f $(EXTRADEPS)
	$(F77) -c matrix_model.f
sv.o: sv.f  $(EXTRADEPS)
	$(F77) -c sv.f
userinterface.o: userinterface.f $(EXTRADEPS)
	$(F77) -c userinterface.f
phasespace.o: phasespace.f $(EXTRADEPS)
	$(F77) -c phasespace.f
hadr5n12.o: hadr5n12.f $(EXTRADEPS)
	$(F77) -c hadr5n12.f
intpl.o: intpl.f $(EXTRADEPS)
	$(F77) -c intpl.f
$(VPHLMNT).o: $(VPHLMNT).f $(EXTRADEPS)
	$(F77) -c $(VPHLMNT).f
hadr5n.o: hadr5n.f $(EXTRADEPS)
	$(F77) -c hadr5n.f
Rteubner.o: Rteubner.f $(EXTRADEPS)
	$(F77) -c Rteubner.f
mapmomenta.o: mapmomenta.f $(EXTRADEPS)
	$(F77) -c mapmomenta.f
sampling.o: sampling.f $(EXTRADEPS)
	$(F77) -c sampling.f
routines.o: routines.f $(EXTRADEPS)
	$(F77) -c routines.f
distributions.o: distributions.f shared.inc $(EXTRADEPS)
	$(F77) -c distributions.f
ranlux.o: ranlux.f $(EXTRADEPS)
	$(F77) -c ranlux.f
$(PAWyn).o: $(PAWyn).f $(EXTRADEPS)
	$(F77) -c $(PAWyn).f
### BEGIN ALPHA
Acp.o: Acp.f $(EXTRADEPS)
	$(F77) -fno-automatic -c Acp.f
Aint.o: Aint.f $(EXTRADEPS)
	$(F77) -fno-automatic -c Aint.f
Asu3.o: Asu3.f $(EXTRADEPS)
	$(F77) -fno-automatic -c Asu3.f
mtx_eeenudbb.o: mtx_eeenudbb.f $(EXTRADEPS)
	$(F77) -fno-automatic -c mtx_eeenudbb.f
interface.o: interface.f $(EXTRADEPS)
	$(F77) -fno-automatic -c interface.f
### END ALPHA

# babayaga library
libbabayaga.a: $(OBJECTS)
	ar cr libbabayaga.a $(OBJECTS)

# executable
$(EXE): libbabayaga.a
	$(F77) main.o $(FPCHECK) -o $(EXE) -L. -lbabayaga $(CERNLIB) $(PAW)
