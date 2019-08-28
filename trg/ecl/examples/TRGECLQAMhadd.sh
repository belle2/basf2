#!/bin/bash 

source_files=( `find /hsm/belle2/bdata/users/haneol/Analysis/exp8/promptskim/bhabha/ -name "unpacked*"` )

 bsub "hadd -f QAMhadd_test.root ${source_files[*]}"

#===<END>



