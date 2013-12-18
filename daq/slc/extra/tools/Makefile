.PHONY: all full clean

all:
	make -C vmecontrold
	make -C logger
	make -C nsmmaster

full:
	make -C cprcontrold

clean:
	make -C vmecontrold clean
	make -C cprcontrold clean
	make -C logger clean
	make -C nsmmaster clean