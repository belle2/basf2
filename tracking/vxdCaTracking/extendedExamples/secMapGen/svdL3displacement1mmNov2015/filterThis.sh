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
# for ((  i = 2 ;  i <= 5;  i++  ))
# do
# 	date
# 	echo " Iteration " $i
# # 	evtAdd=$(( i - 1 ))
# 	basf2 generateAndTestSecMapWithSPs.py $i 5000 >> $outPutFile
# 	date
# 	echo "That was iteration number " $i
# 	date
# done
# date
# echo "starting exportSecMap"
# basf2 onlyExportMapWithSPs.py 1 >> $outPutFile
# # basf2 onlyExportMapWithSPs.py 0 >> $outPutFile2
# date
# mkdir 25k
# mv shifted*.xml 25k

date
for ((  i = 1 ;  i <= 12;  i++  ))
do
    date
    echo " Iteration " $i
#   evtAdd=$(( i - 1 ))
    basf2 generateAndTestSecMapWithSPs.py $i 5000 >> $outPutFile
    date
    echo "That was iteration number " $i
    date
done
date
echo "starting exportSecMap"
basf2 onlyExportMapWithSPs.py 1 >> $outPutFile
# basf2 onlyExportMapWithSPs.py 0 >> $outPutFile2
date
mkdir 60k
mv shifted*.xml 60k

date
 
for ((  i = 13 ;  i <= 14;  i++  ))
do
	date
	echo " Iteration " $i
# 	evtAdd=$(( i - 1 ))
	basf2 generateAndTestSecMapWithSPs.py $i 5000 >> $outPutFile
	date
	echo "That was iteration number " $i
	date
done
date
echo "starting exportSecMap"
basf2 onlyExportMapWithSPs.py 1 >> $outPutFile
# basf2 onlyExportMapWithSPs.py 0 >> $outPutFile2
date
mkdir 70k
mv shifted*.xml 70k
 
date
 
for ((  i = 15 ;  i <= 16;  i++  ))
do
	date
	echo " Iteration " $i
# 	evtAdd=$(( i - 1 ))
	basf2 generateAndTestSecMapWithSPs.py $i 5000 >> $outPutFile
	date
	echo "That was iteration number " $i
	date
done
date
echo "starting exportSecMap"
basf2 onlyExportMapWithSPs.py 1 >> $outPutFile
# basf2 onlyExportMapWithSPs.py 0 >> $outPutFile2
date
mkdir 80k
mv shifted*.xml 80k
date

date
 
for ((  i = 17 ;  i <= 18;  i++  ))
do
	date
	echo " Iteration " $i
# 	evtAdd=$(( i - 1 ))
	basf2 generateAndTestSecMapWithSPs.py $i 5000 >> $outPutFile
	date
	echo "That was iteration number " $i
	date
done
date
echo "starting exportSecMap"
basf2 onlyExportMapWithSPs.py 1 >> $outPutFile
# basf2 onlyExportMapWithSPs.py 0 >> $outPutFile2
date
mkdir 90k
mv shifted*.xml 90k
date


for ((  i = 19 ;  i <= 20;  i++  ))
do
    date
    echo " Iteration " $i
#   evtAdd=$(( i - 1 ))
    basf2 generateAndTestSecMapWithSPs.py $i 5000 >> $outPutFile
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
mv shifted*.xml 100k
date


for ((  i = 21 ;  i <= 22;  i++  ))
do
    date
    echo " Iteration " $i
#   evtAdd=$(( i - 1 ))
    basf2 generateAndTestSecMapWithSPs.py $i 5000 >> $outPutFile
    date
    echo "That was iteration number " $i
    date
done
date
echo "starting exportSecMap"
basf2 onlyExportMapWithSPs.py 1 >> $outPutFile
# basf2 onlyExportMapWithSPs.py 0 >> $outPutFile2
date
mkdir 110k
mv shifted*.xml 110k
date


for ((  i = 23 ;  i <= 24;  i++  ))
do
    date
    echo " Iteration " $i
#   evtAdd=$(( i - 1 ))
    basf2 generateAndTestSecMapWithSPs.py $i 5000 >> $outPutFile
    date
    echo "That was iteration number " $i
    date
done
date
echo "starting exportSecMap"
basf2 onlyExportMapWithSPs.py 1 >> $outPutFile
# basf2 onlyExportMapWithSPs.py 0 >> $outPutFile2
date
mkdir 120k
mv shifted*.xml 120k
date


for ((  i = 25 ;  i <= 26;  i++  ))
do
    date
    echo " Iteration " $i
#   evtAdd=$(( i - 1 ))
    basf2 generateAndTestSecMapWithSPs.py $i 5000 >> $outPutFile
    date
    echo "That was iteration number " $i
    date
done
date
echo "starting exportSecMap"
basf2 onlyExportMapWithSPs.py 1 >> $outPutFile
# basf2 onlyExportMapWithSPs.py 0 >> $outPutFile2
date
mkdir 130k
mv shifted*.xml 130k
date


for ((  i = 27 ;  i <= 28;  i++  ))
do
    date
    echo " Iteration " $i
#   evtAdd=$(( i - 1 ))
    basf2 generateAndTestSecMapWithSPs.py $i 5000 >> $outPutFile
    date
    echo "That was iteration number " $i
    date
done
date
echo "starting exportSecMap"
basf2 onlyExportMapWithSPs.py 1 >> $outPutFile
# basf2 onlyExportMapWithSPs.py 0 >> $outPutFile2
date
mkdir 140k
mv shifted*.xml 140k
date


for ((  i = 29 ;  i <= 30;  i++  ))
do
    date
    echo " Iteration " $i
#   evtAdd=$(( i - 1 ))
    basf2 generateAndTestSecMapWithSPs.py $i 5000 >> $outPutFile
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
mv shifted*.xml 150k
date

# 
# for ((  i = 102 ;  i <= 111;  i++  ))
# do
#     date
#     echo " Iteration " $i
# #   evtAdd=$(( i - 1 ))
#     basf2 generateAndTestSecMapWithSPs.py $i 5000 >> $outPutFile
#     date
#     echo "That was iteration number " $i
#     date
# done
# date
# echo "starting exportSecMap"
# basf2 onlyExportMapWithSPs.py 1 >> $outPutFile
# # basf2 onlyExportMapWithSPs.py 0 >> $outPutFile2
# date
# mkdir 550k
# mv *.xml 550k
# date
# 
# 
# for ((  i = 112 ;  i <= 121;  i++  ))
# do
#     date
#     echo " Iteration " $i
# #   evtAdd=$(( i - 1 ))
#     basf2 generateAndTestSecMapWithSPs.py $i 5000 >> $outPutFile
#     date
#     echo "That was iteration number " $i
#     date
# done
# date
# echo "starting exportSecMap"
# basf2 onlyExportMapWithSPs.py 1 >> $outPutFile
# # basf2 onlyExportMapWithSPs.py 0 >> $outPutFile2
# date
# mkdir 600k
# mv *.xml 600k
# date