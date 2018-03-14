[ -z $BASH ] || shopt -s expand_aliases
alias BEGINCOMMENT="if : ; then"    #uncomment
alias ENDCOMMENT="fi"

rootdir=$(grep dir_root location  | awk -F"\"" '{print $2}')
storedir=$(grep dir_store location  | awk -F"\"" '{print $2}')
#dbdir=$(grep dir_dbdata location  | awk -F"\"" '{print $2}')
subdbdir=$(grep dir_params location  | awk -F"\"" '{print $2}')

dbdir=${BELLE2_LOCAL_DIR}/cdc/data/
storeDB=${dbdir}store_${subdbdir}

xtfile=$(grep xt_file location  | awk -F"\"" '{print $2}')
sigmafile=$(grep sigma_file location  | awk -F"\"" '{print $2}')
twfile=$(grep tw_file location  | awk -F"\"" '{print $2}')
t0file=$(grep t0_file location  | awk -F"\"" '{print $2}')

mkdir -p $storeDB
mkdir -p $storedir

#rm -rf $storedir/*
#rm -rf $storeDB/*
rm -rf result.out


#echo $rootdir
mkdir -p $rootdir
rm -f exit
rm -f error
rm -f temp
time_start=`date`
SECONDS=0   #start clock

BEGINCOMMENT

PrintStart()
{
    Loop=$1
    echo ====================================================
    echo start loop : $Loop
}

#function to correct t0
Correct_T0()
{
    probcut=$1
    if [ "$probcut" == "" ] ; then
	probcut="0.001"
    fi
    echo T0 correction
    echo run with Probcut for DAF = $probcut

    bash runCollector4Calib.sh $probcut
    [ -f exit -o -f error ] && echo stop, please check root file  && exit

    basf2 T0Correction.py
    [ -f error ] && echo stop, please check rootfile && exit
    echo '.x getResult.C'|root -b -l
    cp -r $dbdir/$subdbdir/ $storeDB/db_${i}/
    cp -r t0.dat $dbdir/$subdbdir/$t0file
    mv Correct_T0.root $rootdir
    mkdir -p $storedir/it${i}_t0
    mv $rootdir/* $storedir/it${i}_t0/  

}

# function to update xt and sigma, t0
Update_XT()
{
    bash runCollector4Calib.sh
    [ -f exit -o -f error ] && echo stop  && exit

    basf2 XTFit.py
    echo '.x getResult.C'|root -b -l
    [ -f exit -o -f error ] && echo stop, please check root file && exit
    cp -r $dbdir/$subdbdir/ $storeDB/db_xt_${i}/
    cp xt_new.dat $dbdir/$subdbdir/$xtfile
    mkdir -p $storedir/it${i}_XT
    mv $rootdir/* $storedir/it${i}_XT/  
}

# update sigma, t0
Update_Sigma_T0()
{
    bash runCollector4Calib.sh
    [ -f exit -o -f error ] && echo stop, please check root file   && exit
    basf2 SpaceResol.py
    basf2 T0Correction.py
    echo '.x getResult.C'|root -b -l
    [ -f error ] && echo stop, please check sigma fit && exit

    cp -r $dbdir/$subdbdir/ $storeDB/db_sigma_${i}/
    cp -r t0.dat $dbdir/$subdbdir/$t0file
    cp sigma_new.dat $dbdir/$subdbdir/$sigmafile

    mv Corect_T0.root $rootdir
    mv sigma_histo.root $rootdir
    mkdir -p $storedir/it${i}_sigma
    mv $rootdir/* $storedir/it${i}_sigma/  
}

# Time Walk
TimeWalk(){
    bash runCollector4Calib.sh
    [ -f exit -o -f error ] && echo stop  && exit
    echo get tw
    basf2 TimeWalk.py
    echo '.x getResult.C'|root -b -l
    cp -r $dbdir/$subdbdir/ $storeDB/db_9/
    cp tw.dat $dbdir/$subdbdir/$twfile 
    mv tw_histo.root $rootdir
    mkdir -p $storedir/it${i}_tw
    mv $rootdir/* $storedir/it${i}_tw/
}

# Now we do 4 iteration to correct t0

for ((i = 1; i <= 2; i++)); do
    PrintStart $i
    Correct_T0 0.00001
done

for ((i = 3; i <= 4; i++)); do
    PrintStart $i
    Correct_T0
done
for ((i = 5; i <= 8; i++)); do
    PrintStart $i
    Update_XT
    Update_Sigma_T0
done
# correct Time Walk 
i=9
PrintStart $i
TimeWalk

# correct T0 again, two iteration
for ((i = 10; i <= 10; i++)); do
    PrintStart $i
    Correct_T0
done
# Update XT,Sigma T0 again
for ((i = 11; i <= 11; i++)); do
    PrintStart $i
    Update_XT
    Update_Sigma_T0
done

#submit jobs to get the last result
bash runCollector4Calib.sh

#sumary 
time_end=`date`
echo =============================================================
echo =---------------------- Finish -----------------------------+
echo =============================================================
echo =  Start  at:      $time_start -----------------------+                               
echo =  Finish at:      $time_end   ----------------------+                              
duration=$SECONDS
echo "=  $(($duration / 60)) minutes and $(($duration % 60)) seconds ---+" 
echo =============================================================

ENDCOMMENT
exit
