#! /bin/tcsh -f

# script to analyze the hitfiles using either the _Extended or the _AsicByAsic macro.
# Author: Umberto Tamponi
# tamponi@to.infn.it
#
# syntax example:
#
# ./runTopCafAna_3.csh /ghi/fs01/belle2/bdata/group/detector/TOP/CRT_analysis/hits/ slot05   slot05_AnalsisOutput/
#
# analyzes all the files form slot 05 stored in /ghi/fs01/belle2/bdata/group/detector/TOP/CRT_analysis/hits/ and puts the
# output files in the slot05_AnalsisOutput/ folder 

set hitFilePath=$1
set slot=$2
set outFolder=$3
mkdir $outFolder

foreach file (`\ls -1 $hitFilePath`) 
    set type=`echo $file | awk -F '_' '{print $3}'`
    set thisSlot=`echo $file | awk -F '_' '{print $2}'`
    if($type == 'laser'  && $thisSlot == $slot) then
	bsub -q s  ${BELLE2_LOCAL_DIR}/bin/Linux_x86_64/opt/analyzeHits_AsicByAsic $hitFilePath$file   $outFolder/$file"_out.root"              
	#bsub -q s  ${BELLE2_LOCAL_DIR}/bin/Linux_x86_64/opt/analyzeHits_Extended $hitFilePath$file   $outFolder/$file"_out.root"              
	echo $hitFilePath$file
    endif 
end

