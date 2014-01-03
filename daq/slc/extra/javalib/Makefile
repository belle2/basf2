all:
	make -C b2daq -j20
	make -C b2dqm exe
	make -C b2rc exe
	make -C b2logger exe
	make -C b2gui -j20

clean:
	make -C b2daq clean
	make -C b2dqm clean
	make -C b2rc clean
	make -C b2logger clean
	make -C b2gui clean
