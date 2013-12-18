
all:
	@ make -C nsm2/daemon
	@ make -C nsm2/daemon install
	@ make -C tools

full:
	@ make -C tools full

nsm:
	@ source nsm2/export.sh

clean:
	@ make -C tools clean
	@ make -C nsm2 clean