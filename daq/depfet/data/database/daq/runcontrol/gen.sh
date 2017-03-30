list=`dblist daq RUNCONTROL`
for line in $list
do
file=`echo "$line" | sed "s/RUNCONTROL/runcontrol/g" | sed "s/@RC//g" | sed "s/:/./g"`".conf"
echo $file
rcconfigget daq $line > $file
done
