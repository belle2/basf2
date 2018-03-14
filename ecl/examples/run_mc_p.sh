#!/bin/bash -f

type=2212
mom=(300 400 500 750 1000 1500 2000 3000 4000 4500 5000 5500)
theta=(0 17 31 33 44 117 128 131 150 180)
i=0
while [ $i -lt 11 ]
do
j=0
while [ $j -lt 9 ]
do
echo ./MakeMC_BGx0.py $type ${mom[i]} ${mom[i+1]} ${theta[j]} ${theta[j+1]}
bsub -q l basf2 ./MakeMC_BGx0.py $type ${mom[i]} ${mom[i+1]} ${theta[j]} ${theta[j+1]}
echo ./MakeMC_BGx1.py $type ${mom[i]} ${mom[i+1]} ${theta[j]} ${theta[j+1]}
bsub -q l basf2 ./MakeMC_BGx1.py $type ${mom[i]} ${mom[i+1]} ${theta[j]} ${theta[j+1]}
echo ./MakeMCAnti_BGx0.py $type ${mom[i]} ${mom[i+1]} ${theta[j]} ${theta[j+1]}
bsub -q l basf2 ./MakeMCAnti_BGx0.py $type ${mom[i]} ${mom[i+1]} ${theta[j]} ${theta[j+1]}
echo ./MakeMCAnti_BGx1.py $type ${mom[i]} ${mom[i+1]} ${theta[j]} ${theta[j+1]}
bsub -q l basf2 ./MakeMCAnti_BGx1.py $type ${mom[i]} ${mom[i+1]} ${theta[j]} ${theta[j+1]}
let j=j+1
done
let i=i+1
: <<'END'

END
done

exit 0
