#!/bin/sh
./cleanup_ecl01_ecl02.sh
./ecl_hslb_setup.sh
for i in `seq 201 10000`
do
echo "run $i starts"
./run_start_eclcrt.sh $i
./cleanup_ecl01_ecl02.sh
echo "cleaning up done"
./ecl_hslb_setup.sh
sleep 2
done