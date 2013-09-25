#! /bin/tcsh
onintr emergency
@ ncore=`cat /proc/cpuinfo | grep "processor" | wc | awk '{print $1}'` / 2
basf2 recon.py $1 $2 $3 $ncore
exit
emergency:
echo "interrupt received"
killall basf2
exit

