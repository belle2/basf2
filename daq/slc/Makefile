.PHONY: all runcontrold cprcontrold rocontrold full clean ttdd 

all:
	make -C src/
	make -C vmecontrold/
	make -C bin/

full:
	make -C src/ full
	make -C tools/
	make -C bin/ full

clean:
	make -C bin/ clean
	make -C tools/ clean
	make -C src/ clean
	make -C nsm2/ clean
	rm -rf data/nsm/*.h
	rm -rf data/dqm/lib/*.so

dqm:
	make -C src/ dqm
	make -C tools/ dqm
