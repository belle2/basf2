all: eb0 random_event_generator fetch_event

H = raw_copper_header_and_trailer.h send_header_and_trailer.h

%: %.cc ${H}
	${CXX} -o $@ $@.cc
