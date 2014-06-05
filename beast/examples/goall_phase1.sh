#!/bin/bash

# == que list ==
# e <30min
# s <6h
# l <48h
# b_a <48h
# b_l <720h

mkdir -p log
mkdir -p output

# near: 1us*1000 = 1000us 
for f in Brems Touschek Coulomb; do
    for g in LER HER; do
	h=0;
	while [ $h -lt 1000 ]; do 
            #echo "#!/bin/sh" > my_${f}_${g}_${h}.sh
	    #echo "basf2 ./RunSADBgMC_phase1.py ${f}_${g} ${h} >& log/${f}_${g}_${h}.log" >> my_${f}_${g}_${h}.sh
	    #chmod +x  my_${f}_${g}_${h}.sh
	    #sbatch -A belle2 -e err/${f}_${g}_${h}.err -o out/${f}_${g}_${h}.out -n 1 -p shared -t 16:00:00 ./my_${f}_${g}_${h}.sh
	    bsub -q s "basf2 ./RunSADBgMC_phase1.py ${f}_${g} $h >& log/${f}_${g}_$h.log"
	    h=$(($h+1))
	done
    done
done




