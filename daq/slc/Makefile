.PHONY: all runcontrold cprcontrold rocontrold full clean ttdd 

all:
	make -C nsm2/
	make -C src/ full
	make -C templated/
	make -C cprcontrold/
#	make ttdd
	make -C rocontrold/
	make -C runcontrold/
	make -C nsmmonitor/
	make -C bin/

clean:
	make -C javalib/ clean
	make -C tools/ clean
	make -C dqmserver/ clean
	make -C templated/ clean
	make -C cprcontrold/ clean
	make -C rocontrold/ clean
	make -C runcontrold/ clean
	make -C nsmmonitor/ clean
#	make -C ttdd/ clean
	make -C src/ clean
	make -C nsm2/ clean
	make -C bin/ clean

dqm:
	make -C src/ dqm
	make -C tools/ gen_dqm
	make -C tools/ mapfiletest
	make -C dqmserver/
	make -C javalib/b2dqm
