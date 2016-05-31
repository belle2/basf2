#! /bin/tcsh -f

# script to analyze the hitfiles using either the _Extended or the _AsicByAsic macro.
# Author: Umberto Tamponi
# tamponi@to.infn.it
#
# syntax example:
#
# ./runTopCafAna_3.csh /ghi/fs01/belle2/bdata/group/detector/TOP/CRT_analysis/hits/ slot05   slot05_AnalsisOutput/ 2560 2660
#
# analyzes all the files form slot 05 stored in /ghi/fs01/belle2/bdata/group/detector/TOP/CRT_analysis/hits/ and puts the
# output files in the slot05_AnalsisOutput/ folder 
#
# 05/25/2016 : run range selection added (U.T.)
# 05/31/2016 : cosmic flag added (U.T.)

set hitFilePath=$1
set slot=$2
set outFolder=$3
set startRun=$4
set stopRun=$5
mkdir $outFolder

foreach file (`\ls -1 $hitFilePath`) 
    set type=`echo $file | awk -F '_' '{print $3}'`
    set thisSlot=`echo $file | awk -F '_' '{print $2}'`
    set fileEnd=`echo $file | awk -F '_' '{print $5}'`
    set thisRunRaw=`echo $file | awk -F '_' '{print $1}'`
    set thisRunRaw1=`echo $thisRunRaw | sed 's/^run*//'`  
    set thisRun=`echo $thisRunRaw1 | sed 's/^0*//'`
    if ( ( $type == 'laser'  || $type == 'cosmic' ) && $thisSlot == $slot && $thisRun <= $stopRun && $thisRun >= $startRun )  then
	echo $thisRun
        bsub -q s ${BELLE2_LOCAL_DIR}/bin/Linux_x86_64/opt/analyzeHits_AsicByAsic $hitFilePath$file   $outFolder/$file"_out.root"              
	#bsub -q s  ${BELLE2_LOCAL_DIR}/bin/Linux_x86_64/opt/analyzeHits_Extended $hitFilePath$file   $outFolder/$file"_out.root"              
	echo $hitFilePath$file
    endif 
end

