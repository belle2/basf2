#!/bin/sh
/ sbin / rmmod copper
cd / home / yamagata / cprdist - 0.1.0 / drv
/ sbin / insmod . / copper.ko
.. / src / mkdevs.sh
echo 8 > / proc / copper / FINESSE_STA
#echo -1 > /proc/copper/debug_flag
cd / home / yamagata / ttrxprogs - 20060413 / driver
. / load
. / load_fifo
