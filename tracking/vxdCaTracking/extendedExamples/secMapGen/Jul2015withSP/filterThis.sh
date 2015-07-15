#!/bin/bash
dateVal="DATE"$(date +%y-%m-%d-%H:%M)
val1="filterThisOutput"
val2=".txt"
val3="Fine"
outPutFile=$val1$dateVal$val2
outPutFile2=$val1$val3$dateVal$val2
echo "starting FilterCalculator-iterations, export to file $outputFile with date $dateVal"
date
#for ((  i = 1 ;  i <= 5;  i++  ))
#do
#	date
#	echo " Iteration " $i
## 	evtAdd=$(( i - 1 ))
#	basf2 oneForAlltestWithSPs.py $i 5000 >> $outPutFile
#	date
#	echo "That was iteration number " $i
#	date
#done
#date
#echo "starting exportSecMap"
#basf2 onlyExportMapWithSPs.py >> $outPutFile
#date
#mkdir 25k
#mv *.xml 25k

date
for ((  i = 2 ;  i <= 10;  i++  ))
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
# echo "starting exportSecMap"
# basf2 onlyExportMapWithSPs.py >> $outPutFile
# date
# mkdir 50k
# mv *.xml 50k

date
for ((  i = 11 ;  i <= 15;  i++  ))
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
mkdir 75k
mv *.xml 75k

date
 
for ((  i = 16 ;  i <= 20;  i++  ))
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
# cp *.root 100k
 
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
