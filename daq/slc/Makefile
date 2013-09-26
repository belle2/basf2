.PHONY: all runcontrold cprcontrold rocontrold full clean ttdd 

all:
	make -C nsm2/
	make -C src/ full
	make -C templated/
	make -C cprcontrold/
#	make ttdd
	make -C rocontrold/
	make -C runcontrold/
	make -C tools/
	cd tools/gen_copper; ./bin/gen_copper; cd -
	make -C javalib/

clean:
	make -C javalib/ clean
	make -C tools/ clean
	make -C templated/ clean
	make -C cprcontrold/ clean
	make -C rocontrold/ clean
	make -C runcontrold/ clean
#	make -C ttdd/ clean
	make -C src/ clean
	make -C nsm2/ clean
	rm -rf *~ .*~ \#*~

