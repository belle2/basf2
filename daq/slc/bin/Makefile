
all:
	@ rm -f *d nsmd2 *.h
	@ ln -s $(B2SLC_PATH)/nsm2/daemon/nsmd2
	@ ln -s $(B2SLC_PATH)/vmecontrold/bin/vmecontrold
	@ rm cui; cp $(B2SLC_PATH)/nsm2/example/master cui

full:
	@ rm -f *d nsmd2 *.h
	@ ln -s $(B2SLC_PATH)/nsm2/daemon/nsmd2
	@ ln -s $(B2SLC_PATH)/dqmserver/bin/dqmservd
	@ ln -s $(B2SLC_PATH)/runcontrold/bin/runcontrold
	@ ln -s $(B2SLC_PATH)/cprcontrold/bin/cprcontrold
#	@ ln -s $(B2SLC_PATH)/ttdcontrold/bin/ttdcontrold
	@ ln -s $(B2SLC_PATH)/rocontrold/bin/rocontrold
	@ ln -s $(B2SLC_PATH)/vmecontrold/bin/vmecontrold
	@ ln -s $(B2SLC_PATH)/include/nsm/pocket_ttd.h .
	@ ln -s $(B2SLC_PATH)/include/nsm/run_status.h .
	@ ln -s $(B2SLC_PATH)/include/nsm/run_config.h .
	@ rm -f cui; cp $(B2SLC_PATH)/nsm2/example/master cui

clean:
	@ rm -f *d nsmd2 *.h
	@ rm -rf *~ .*~ \#*~ cui
