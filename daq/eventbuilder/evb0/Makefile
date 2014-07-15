TARGET=eb0 eb1tx eb1rx eb2rx fetch_event iseof
TARGET+=check-eb2-output shift-eb2-output extract-onsen-from-eb2-output
TEST1=b2eb_format_test random_event_generator
TEST2=eofcheck sockread
TEST3=fread-intr pthread-read-intr
TEST4=onsen_tcp_test pxdcheck swab framenr textdump
all: $(TARGET) $(TEST1) $(TEST2) $(TEST3) $(TEST4)
CXXFLAGS=-g
#CXX=g++

eb0: eb0.cc log.h neighbor.h b2eb_format.h basf2_tcp.h
	${CXX} ${CXXFLAGS} -o $@ $(filter-out *.h, $<) -lpthread

eb1tx: eb1tx.cc log.h neighbor.h b2eb_format.h basf2_tcp.h
	${CXX} ${CXXFLAGS} -o $@ $(filter-out *.h, $<) -lpthread

eb1rx: eb1rx.cc log.h neighbor.h b2eb_format.h basf2_tcp.h
	${CXX} ${CXXFLAGS} -o $@ $(filter-out *.h, $<) -lpthread

eb2rx: eb2rx.cc log.h neighbor.h b2eb_format.h basf2_tcp.h onsen_tcp.h
	${CXX} ${CXXFLAGS} -o $@ $(filter-out *.h, $<) -lpthread

pthread-read-intr: pthread-read-intr.cc
	${CXX} ${CXXFLAGS} -o $@ $(filter-out *.h, $<) -lpthread


b2eb_format_test: b2eb_format_test.cc b2eb_format.h
	${CXX} ${CXXFLAGS} -o $@ $(filter-out *.h, $<)

test:
	./b2eb_format_test
	./eb0test.sh
	./eb1txtest.sh
	./eb1rxtest.sh

onsen_tcp_test: onsen_tcp_test.cc onsen_tcp.h neighbor.h
	${CXX} ${CXXFLAGS} -o $@ $(filter-out *.h, $<)
