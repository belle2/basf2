
all:
	@ rm -f *d nsmd2 *.h
	@ ln -s $(BELLE2_DAQ_SLC_DIR)/nsm2/daemon/nsmd2
	@ ln -s $(BELLE2_DAQ_SLC_DIR)/vmecontrold/bin/vmecontrold
	@ rm -f cui; cp $(BELLE2_DAQ_SLC_DIR)/nsm2/example/master cui

full:
	@ rm -f *d nsmd2 *.h
	@ ln -s $(BELLE2_DAQ_SLC_DIR)/nsm2/daemon/nsmd2
	@ ln -s $(BELLE2_DAQ_SLC_DIR)/dqmserver/bin/dqmservd
	@ ln -s $(BELLE2_DAQ_SLC_DIR)/runcontrold/bin/runcontrold
	@ ln -s $(BELLE2_DAQ_SLC_DIR)/cprcontrold/bin/cprcontrold
#	@ ln -s $(BELLE2_DAQ_SLC_DIR)/ttdcontrold/bin/ttdcontrold
	@ ln -s $(BELLE2_DAQ_SLC_DIR)/rocontrold/bin/rocontrold
	@ ln -s $(BELLE2_DAQ_SLC_DIR)/vmecontrold/bin/vmecontrold
	@ ln -s $(BELLE2_DAQ_SLC_DIR)/include/nsm/pocket_ttd.h .
	@ ln -s $(BELLE2_DAQ_SLC_DIR)/include/nsm/run_status.h .
	@ ln -s $(BELLE2_DAQ_SLC_DIR)/include/nsm/run_config.h .
	@ rm -f cui; cp $(BELLE2_DAQ_SLC_DIR)/nsm2/example/master cui

clean:
	@ rm -f *d nsmd2 nsminfo2 *.h
	@ rm -rf *~ .*~ \#*~ cui
