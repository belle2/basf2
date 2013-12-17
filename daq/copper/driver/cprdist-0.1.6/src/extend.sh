#!/bin/sh -x
val=$(( 8 * 1024 * 1024 ))  # set default window size 8MByte
max=$(( 80 * 1024 * 1024 )) # set maximum window size 80MByte
cd /proc/sys/net/core
echo $max > rmem_max
echo $max > wmem_max
echo $val > rmem_default
echo $val > wmem_default
read X Y Z < /proc/sys/net/ipv4/tcp_rmem
echo $X $val $max > /proc/sys/net/ipv4/tcp_rmem
read X Y Z < /proc/sys/net/ipv4/tcp_wmem
echo $X $val $max > /proc/sys/net/ipv4/tcp_wmem
cat /proc/sys/net/ipv4/tcp_rmem
cat /proc/sys/net/ipv4/tcp_wmem
