
all :
	@ make -C b2daq
	@ make -C b2daq.runcontrol
	@ make -C b2daq.pscontrol
	@ make -C b2daq.dqmview
	@ make -C b2daq.monitor
	@ mkdir -p jars
	@ rm -rf build/b2daq
	@ make -C build

clean:
	@ make -C b2daq clean
	@ make -C b2daq.runcontrol clean
	@ make -C b2daq.pscontrol clean
	@ make -C b2daq.dqmview clean
	@ make -C b2daq.monitor clean
	@ rm build/ME* build/b2daq jars *~ -rf
