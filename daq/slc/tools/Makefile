.PHONY: all full clean

all:
	make -C runcontrold
	make -C cprcontrold
	make -C rocontrold
	make -C nsmmonitor
#	make -C uploader
	make -C javalib

dqm:
	make -C dqmviewd
	make -C gen_dqm
	make -C mapfiletest
	make -C javalib/b2dqm

clean:
	make -C runcontrold clean
	make -C cprcontrold clean
	make -C rocontrold clean
	make -C nsmmonitor clean
	make -C dqmviewd clean
	make -C uploader clean
	make -C gen_dqm clean
	make -C mapfiletest clean
	make -C javalib clean
