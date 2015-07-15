#!/bin/bash
dateVal="DATE"$(date +%y-%m-%d-%H:%M)
val1="filterThisOutput"
val2=".txt"
val3="Fine"
outPutFile=$val1$dateVal$val2
outPutFile2=$val1$val3$dateVal$val2
echo "starting FilterCalculator-iterations, export to file $outputFile with date $dateVal"
date

date
for ((  i = 1 ;  i <= 10;  i++  ))
do
	date
	echo " Iteration " $i
# 	evtAdd=$(( i - 1 ))
	basf2 oneForAlltestWithSPs.py $i 5000 >> $outPutFile
	date
	echo "That was iteration number " $i
	date
done
date
echo "starting exportSecMap"
basf2 onlyExportMapWithSPs.py >> $outPutFile
date
mkdir 50k
mv *.xml 50k

date
 
for ((  i = 11 ;  i <= 20;  i++  ))
do
	date
	echo " Iteration " $i
# 	evtAdd=$(( i - 1 ))
	basf2 oneForAlltestWithSPs.py $i 5000 >> $outPutFile
	date
	echo "That was iteration number " $i
	date
done
date
echo "starting exportSecMap"
basf2 onlyExportMapWithSPs.py >> $outPutFile
date
mkdir 100k
mv *.xml 100k
 
date
 
for ((  i = 21 ;  i <= 30;  i++  ))
do
	date
	echo " Iteration " $i
# 	evtAdd=$(( i - 1 ))
	basf2 oneForAlltestWithSPs.py $i 5000 >> $outPutFile
	date
	echo "That was iteration number " $i
	date
done
date
echo "starting exportSecMap"
basf2 onlyExportMapWithSPs.py >> $outPutFile
date
mkdir 150k
mv *.xml 150k
date

date
 
for ((  i = 31 ;  i <= 40;  i++  ))
do
	date
	echo " Iteration " $i
# 	evtAdd=$(( i - 1 ))
	basf2 oneForAlltestWithSPs.py $i 5000 >> $outPutFile
	date
	echo "That was iteration number " $i
	date
done
date
echo "starting exportSecMap"
basf2 onlyExportMapWithSPs.py >> $outPutFile
date
mkdir 200k
mv *.xml 200k
date


for ((  i = 41 ;  i <= 50;  i++  ))
do
    date
    echo " Iteration " $i
#   evtAdd=$(( i - 1 ))
    basf2 oneForAlltestWithSPs.py $i 5000 >> $outPutFile
    date
    echo "That was iteration number " $i
    date
done
date
echo "starting exportSecMap"
basf2 onlyExportMapWithSPs.py >> $outPutFile
date
mkdir 250k
mv *.xml 250k
date


for ((  i = 51 ;  i <= 60;  i++  ))
do
    date
    echo " Iteration " $i
#   evtAdd=$(( i - 1 ))
    basf2 oneForAlltestWithSPs.py $i 5000 >> $outPutFile
    date
    echo "That was iteration number " $i
    date
done
date
echo "starting exportSecMap"
basf2 onlyExportMapWithSPs.py >> $outPutFile
date
mkdir 300k
mv *.xml 300k
date


for ((  i = 61 ;  i <= 70;  i++  ))
do
    date
    echo " Iteration " $i
#   evtAdd=$(( i - 1 ))
    basf2 oneForAlltestWithSPs.py $i 5000 >> $outPutFile
    date
    echo "That was iteration number " $i
    date
done
date
echo "starting exportSecMap"
basf2 onlyExportMapWithSPs.py >> $outPutFile
date
mkdir 350k
mv *.xml 350k
date