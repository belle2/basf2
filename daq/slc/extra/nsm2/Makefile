all:
	make -C daemon/
	make -C corelib/
	make -C corelib/ install
	make -C b2lib/
	make -C b2lib/ install
	make -C example/ -f Makefile.no-readline

clean: 
	make -C daemon/ clean
	make -C corelib/ clean
	make -C b2lib/ clean
	make -C example/ clean
