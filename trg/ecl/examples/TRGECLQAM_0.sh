#!/bin/bash 

#a=5000;



#physics="physics"
for a in $(seq -f %05g 101 1499) ; do

    directory="/hsm/belle2/bdata/Data/e0008/4S/PromptSkim/release-03-01-04/DB00000607/r$a/skim/hlt_bhabha/raw/sub00";
    echo $directory;
    number_bhabha=`find $directory -name "*bhabha*" | wc -l ;`
    echo $number_bhabha;
#Make Unpacked directory

    if [ $number_bhabha == 0 ]
    then
	echo "$directory has no files";
    elif [ $number_bhabha != 0 ]
    then
	mkdir ./r$a
echo "	hadd -f ./r$a/packed_run$a.root /hsm/belle2/bdata/Data/e0008/4S/PromptSkim/release-03-01-04/DB00000607/r$a/skim/hlt_bhabha/raw/sub00/*.root "

	hadd -f ./r$a/packed_run$a.root /hsm/belle2/bdata/Data/e0008/4S/PromptSkim/release-03-01-04/DB00000607/r$a/skim/hlt_bhabha/raw/sub00/*.root


# #Check The Number of High Level Trigger files
#     for b in 0 1 2 3 4 5 ; do
# 	number_hlt=`find $directory -name "*HLT$b*" | wc -l ;`
# 	echo $number_hlt;
# 	number_hlt=$[$number_hlt-1];
# #Make Unpacked Files
# 	for c in $(seq -f %05g 0 $number_hlt); do
# 	    nfile="/hsm/belle2/bdata/Data/e0008/4S/PromptSkim/release-03-01-04/DB00000607/r$a/skim/hlt_bhabha/raw/sub00/physics.0008.$a.HLT$b.hlt_bhabha.f$c.root";
# 	    bsub "basf2 TrgEclQAM.py $nfile /hsm/belle2/bdata/users/haneol/Analysis/exp8/promptskim/bhabha/r$a/run${a}_hlt${b}_f${c}.root"
	    
# 	done #Finished Unpacked Files

#     done # Finished HLT files check

bsub "basf2 TrgEclQAM.py ./r$a/packed_run$a.root /hsm/belle2/bdata/users/haneol/Analysis/exp8/promptskim/bhabha/r$a/unpacked_run$a.root"
	
    fi #Finished Run Process  

done #Finished All Process

#===<END>



