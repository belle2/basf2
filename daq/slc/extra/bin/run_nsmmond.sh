#!/bin/bash

exe="${B2SLC_PATH}/nsmmonitor/bin/nsmmond NSMMON "
for format in $*
do
arr=(`echo ${format} | tr -s ':' ' '`)
revision=`cat ${NSM2_INCDIR}/${arr[1]}.h | grep revision | sed "s/\;//g" | awk '{print $5}'`
exe="${exe} ${format}:${revision}"
done

$exe