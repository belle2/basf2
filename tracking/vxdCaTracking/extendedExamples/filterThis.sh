#!/bin/bash
dateVal="DATE"$(date +%y-%m-%d-%H:%M)
val1="filterThisOutput"
val2=".txt"
val3="Fine"
outPutFile=$val1$dateVal$val2
outPutFile2=$val1$val3$dateVal$val2
echo "starte FilterCalculator-Durchgänge, exportieren in $outputFile mit datum $dateVal"
date
for ((  i = 2 ;  i <= 9;  i++  ))
do
	date
	echo " Durchgang " $i
	evtAdd=$(( i - 1 ))
	basf2 oneForAlltest.py $i 5000 $evtAdd >> $outPutFile
	date
#	echo " jetzt das Feine "
#  basf2 filterCalcFine.py $i >> $outPutFile2#+
	echo "das war Durchgang nummer " $i
	date
done
date
echo "starten exportSecMap"
basf2 oneForAlltest.py 10 5000 9 1
date
