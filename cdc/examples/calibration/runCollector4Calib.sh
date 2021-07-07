[ -z $BASH ] || shopt -s expand_aliases
#alias BEGINCOMMENT="if : ; then"    #uncomment
alias BEGINCOMMENT="if [ ]; then"   #comment
alias ENDCOMMENT="fi"

#rm -f it/*.root
#rm -f it/*.log
probcut=$1
#echo $probcut

if [ "$probcut" == "" ] ; then
    probcut="0.001"
fi


BEGINCOMMENT
echo $probcut
ENDCOMMENT

rootdir=$(grep dir_root location  | awk -F"\"" '{print $2}')
rm -f $rootdir/*.root
rm -f $rootdir/*.log
mkdir -p $rootdir
# read in runlist , submit job and get job id
i=0
maxresubmit=1
waitloop=100
track="l"
echo submitting job
while read line || [[ -n "$line" ]]; do   
    if [ "$line" == "" ]; then
	break
    fi 
    run[$i]=$line
    jobid[$i]=$(bsub -q $track "basf2 ana.py ${run[$i]} $probcut >& "$rootdir"/run_"${run[$i]}".log" | awk -F'[<>]' '{print $2}')
    #ENDCOMMENT
    jobstat[$i]="running"
    repeated[$i]=0
    let i++
done <"runlist"
n=$i
njob=$n
waitTime=300
echo wait $waitTime min
sleep $waitTime'm'
for (( j=0; j<waitloop; j++ ))
do    
    njob=0
    bjobs >jobs
    for (( i=0; i<n; i++ ))
    do
	#echo i=$i
	if [ "${jobstat[$i]}" == "good" ];then
	    continue
	fi
   # 
	if [ "${jobstat[$i]}" == "running" ];then
	    stat=$(grep ${jobid[$i]} jobs  | awk '{print $3}')
	    #echo $stat
	    if [ "$stat" == "RUN" -o "$stat" == "PEND"  ];then
		#echo still running ${run[$i]}
		let njob++		
		continue
	    else
		jobstat[$i]="done"
	    fi
	fi
#check finished job /submit new one
	if [ "${jobstat[$i]}" == "done" ];then
	#echo x
	    ending=$(grep "Finish" $rootdir/run_${run[$i]}.log | awk '{print $1}')
	    #echo $ending
	    if [ "$ending" == "Finish" ];then
		#echo finish ${run[$i]}
		jobstat[$i]="good"
		continue
	    else
		if [ "${repeated[$i]}" -eq "$maxresubmit"  ];then
		    echo reach $maxresubmit resubmit, stop resubmit
		    jobstat[$i]="bad"
		    continue
		fi
		let repeated[$i]++
		let njob++
# resubmit + get new id here
		jobid[$i]=$(bsub -q $track "basf2 ana.py ${run[$i]} $probcut >& "$rootdir"/run_"${run[$i]}".log" | awk -F'[<>]' '{print $2}')
		jobstat[$i]="running"
		echo run again ${run[$i]}
	    fi 
	fi
		
    done
    [[ "$njob" -ne "0" ]] || break
    echo $j waiting for $njob job, sleep 5 minute
    sleep 5m
done

for (( i=0; i<n; i++ ))
do
    if [ "${jobstat[$i]}" == "bad" ];then
    echo  CHECK run ${run[i]} redo ${repeated[i]} stat ${jobstat[$i]} | tee -a error
    fi
done

if [ $j -eq $waitloop ];then
    echo waiting too long
    echo please check server jobs
    echo quit unfinish
    touch exit
    #echo $j
fi



