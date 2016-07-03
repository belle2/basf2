#!/bin/bash                                                                                                                                                              

# == que list ==                                                                                                                                                         
# e <30min                                                                                                                                                               
# s <6h                                                                                                                                                                  
# l <48h                                                                                                                                                                 
# b_a <48h                                                                                                                                                               
# b_l <70h   

#input=/ghi/fs01/belle2/bdata/users/igal/output_phase1_v2                                                                                                              
#log=log_study_phase1_v2
#output=output_study_phase1_v2

#input=/ghi/fs01/belle2/bdata/users/igal/output_phase1_nturn25
#log=log_study_phase1_nturn25                                                                                                                                           
#output=output_study_phase1_nturn25  

input=/ghi/fs01/belle2/bdata/users/igal/output_phase1_nturn1000                                                                                                  
log=log_study_phase1_nturn1000                                                                                                                                           
output=output_study_phase1_nturn1000

#threshold in MeV
THR=0.1
#range in MeV
RANGE=1000
#Sample time in second
ST=0.01

for f in Touschek Coulomb Brems; do                                                                                                                                     
    for g in HER LER; do                                                                                                                                                
        RND=`od -An -N3 -l /dev/random |  sed 's/^ *\(.*\) *$/\1/'`
        bsub -q s -o /dev/null "basf2 ./Run_study_phase1.py ${input} ${f} ${g} ${output} ${RND} ${THR} ${RANGE} ${ST} >& ${log}/${f}_${g}.log"
    done
done
