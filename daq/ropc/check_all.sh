#!/bin/sh
ssh -XY svd01 $1
ssh -XY svd02 $1
ssh -XY svd03 $1
ssh -XY svd04 $1
ssh -XY svd05 $1
ssh -XY svd06 $1
ssh -XY svd07 $1
ssh -XY svd08 $1
ssh -XY svd09 $1
ssh -XY svd10 $1
ssh -XY svd11 $1

ssh -XY cdc01 $1
ssh -XY cdc02 $1
ssh -XY cdc03 $1
ssh -XY cdc04 $1
ssh -XY cdc05 $1
ssh -XY cdc06 $1
ssh -XY cdc07 $1
ssh -XY cdc08 $1
ssh -XY cdc09 $1
ssh -XY cdc10 $1

ssh -XY top01 $1
ssh -XY top02 $1
ssh -XY top03 $1
ssh -XY top04 $1

ssh -XY arich01 $1
ssh -XY arich02 $1
ssh -XY arich03 $1

ssh -XY ecl01 $1
ssh -XY ecl02 $1
ssh -XY ecl03 $1
ssh -XY ecl04 $1
ssh -XY ecl05 $1
ssh -XY ecl06 $1
ssh -XY ecl07 $1

ssh -XY klm01 $1
ssh -XY klm02 $1
ssh -XY klm03 $1
ssh -XY klm04 $1

