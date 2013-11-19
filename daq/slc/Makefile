.PHONY: all runcontrold cprcontrold rocontrold full clean ttdd 

all:
	make -C src/
	make -C vmecontrold/
	make -C bin/

full:
	make -C src/ full
	make -C hvcontrold/
#	make -C tools/
	make -C bin/ full

clean:
	make -C bin/ clean
	make -C tools/ clean
	make -C hvcontrold/ clean
	make -C src/ clean
	make -C nsm2/ clean

dqm:
	make -C src/ dqm
	make -C tools/ dqm
