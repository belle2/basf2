#!/bin/bash
dateVal="DATE"$(date +%y-%m-%d-%H:%M)
val1="filterThisOutput"
val4="onlyOldExportSecMap"
val2=".txt"
outPutFile=$val1$dateVal$val2
outPutFile2=$val4$dateVal$val2
echo "starting FilterCalculator-iterations, export to file $outputFile with date $dateVal"
date

date
for ((  i = 2 ;  i <= 5;  i++  ))
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
basf2 onlyExportMapWithSPs.py 1 >> $outPutFile
# basf2 onlyExportMapWithSPs.py 0 >> $outPutFile2
date
mkdir 20k
mv *.xml 20k

date
for ((  i = 6 ;  i <= 11;  i++  ))
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
basf2 onlyExportMapWithSPs.py 1 >> $outPutFile
# basf2 onlyExportMapWithSPs.py 0 >> $outPutFile2
date
mkdir 50k
mv *.xml 50k

date
 
for ((  i = 12 ;  i <= 21;  i++  ))
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
basf2 onlyExportMapWithSPs.py 1 >> $outPutFile
# basf2 onlyExportMapWithSPs.py 0 >> $outPutFile2
date
mkdir 100k
mv *.xml 100k
 
date
 
for ((  i = 22 ;  i <= 31;  i++  ))
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
basf2 onlyExportMapWithSPs.py 1 >> $outPutFile
# basf2 onlyExportMapWithSPs.py 0 >> $outPutFile2
date
mkdir 150k
mv *.xml 150k
date

date
 
for ((  i = 32 ;  i <= 41;  i++  ))
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
basf2 onlyExportMapWithSPs.py 1 >> $outPutFile
# basf2 onlyExportMapWithSPs.py 0 >> $outPutFile2
date
mkdir 200k
mv *.xml 200k
date


for ((  i = 42 ;  i <= 51;  i++  ))
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
basf2 onlyExportMapWithSPs.py 1 >> $outPutFile
# basf2 onlyExportMapWithSPs.py 0 >> $outPutFile2
date
mkdir 250k
mv *.xml 250k
date


for ((  i = 52 ;  i <= 61;  i++  ))
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
basf2 onlyExportMapWithSPs.py 1 >> $outPutFile
# basf2 onlyExportMapWithSPs.py 0 >> $outPutFile2
date
mkdir 300k
mv *.xml 300k
date


for ((  i = 62 ;  i <= 71;  i++  ))
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
basf2 onlyExportMapWithSPs.py 1 >> $outPutFile
# basf2 onlyExportMapWithSPs.py 0 >> $outPutFile2
date
mkdir 350k
mv *.xml 350k
date


for ((  i = 72 ;  i <= 81;  i++  ))
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
basf2 onlyExportMapWithSPs.py 1 >> $outPutFile
# basf2 onlyExportMapWithSPs.py 0 >> $outPutFile2
date
mkdir 400k
mv *.xml 400k
date


for ((  i = 82 ;  i <= 91;  i++  ))
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
basf2 onlyExportMapWithSPs.py 1 >> $outPutFile
# basf2 onlyExportMapWithSPs.py 0 >> $outPutFile2
date
mkdir 450k
mv *.xml 450k
date


for ((  i = 92 ;  i <= 101;  i++  ))
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
basf2 onlyExportMapWithSPs.py 1 >> $outPutFile
# basf2 onlyExportMapWithSPs.py 0 >> $outPutFile2
date
mkdir 500k
mv *.xml 500k
date


for ((  i = 102 ;  i <= 111;  i++  ))
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
basf2 onlyExportMapWithSPs.py 1 >> $outPutFile
# basf2 onlyExportMapWithSPs.py 0 >> $outPutFile2
date
mkdir 550k
mv *.xml 550k
date


for ((  i = 112 ;  i <= 121;  i++  ))
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
basf2 onlyExportMapWithSPs.py 1 >> $outPutFile
# basf2 onlyExportMapWithSPs.py 0 >> $outPutFile2
date
mkdir 600k
mv *.xml 600k
date