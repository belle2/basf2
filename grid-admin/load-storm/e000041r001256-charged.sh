
#!/bin/bash

#

Site=$(python -c "import DIRAC;print DIRAC.siteName()")

Platform=$(dirac-platform)

#
if [ -d $OSG_APP/belle/$Platform ]; then

export VO_BELLE_SW_DIR=$OSG_APP/belle

fi

export BHOME=${BHOME:-/belle}

[ $VO_BELLE_SW_DIR ] && export BHOME=$VO_BELLE_SW_DIR/$Platform

if [ ! -d $BHOME/belle -o ! -d $BHOME/cern ] ; then

echo Software not properly installed at $BHOME

ls -l $BHOME
ls -l $BHOME/../

exit 1

else

ln -s $BHOME/* .

export BHOME=.

fi

#

if [ "$Site" = "LCG.GRIDKA.de" ] ; then

export BELLE_POSTGRES_SERVER=ekpbelle.physik.uni-karlsruhe.de

elif [ "$Site" = "LCG.MPPMU.de" ] ; then

export BELLE_POSTGRES_SERVER=ekpbelle.physik.uni-karlsruhe.de

elif [ "$Site" = "LCG.CYFRONET.pl" ] ; then

export BELLE_POSTGRES_SERVER=ekpbelle.physik.uni-karlsruhe.de

elif [ "$Site" = "LCG.CESNET.cz" ] ; then

export BELLE_POSTGRES_SERVER=belle1.egee.cesnet.cz

elif [ "$Site" = "LCG.KISTI.kr" ] ; then

export BELLE_POSTGRES_SERVER=134.75.123.6

elif [ "$Site" = "LCG.IJS.si" ] ; then

export BELLE_POSTGRES_SERVER=ptjot.ijs.si

elif [ "$Site" = "LCG.Melbourne.au" ] ; then

export BELLE_POSTGRES_SERVER=bellepostgres.collab.unimelb.edu.au

elif [ "$Site" = "LCG.KEK2.jp" ] ; then

export BELLE_POSTGRES_SERVER=bellepostgres.collab.unimelb.edu.au

else

export BELLE_POSTGRES_SERVER=${BELLE_POSTGRES_SERVER:-bellepostgres.collab.unimelb.edu.au}

fi

echo $Site | grep -q Amazon && export BASF_NPROCESS=${BASF_NPROCESS:-$(( 2 * 12 / 10 )) }

export BASF_NPROCESS=${BASF_NPROCESS:-0}

export PGUSER=belle

unset CERN

export MCPROD_CERN_LEVEL=2005

#

# First create background index files

#

./addbg_index.sh *Background*.bbs &

#

# Then unzip pgen file

#

for pgen in *.pgen.gz

do

gunzip $pgen &

done

#

wait

#


#

# Now execute charged event simulation

#

start=$(date +%s)

echo Executing evtgen-charged-09-all-e000041r001256-b20090127_0910.sh @ $Site on $Platform

echo

./evtgen-charged-09-all-e000041r001256-b20090127_0910.sh

exitcode=$?

end=$(date +%s)

echo

echo Done evtgen-charged-09-all-e000041r001256-b20090127_0910.sh

echo Exit code $exitcode

[ $exitcode -eq 0 ] || exit $exitcode

echo

echo DIRAC Simulation Report: $(host $HOSTNAME | cut -d " " -f 4 ) $JOBID charged 22955
$(($end - $start))

echo


#

# Now Upload the results

#

for file in *.mdst*

do

mkdir -p ../mdst

mv $file ../mdst

done

#

tar -cvzf e000041r001256-charged.logs.tgz* *.his* *.log* *.status* && rm *.his* *.log* *.status*

mkdir -p ../logs

mv e000041r001256-charged.logs.tgz* ../logs

#

if ( echo $Site | grep -q "LCG" ) ; then

python -c "from DIRAC.Core.Base.Script import parseCommandLine;parseCommandLine('OutputDataUpload');from BelleDIRAC.WorkloadManagementSystem.private.OutputDataExecutor import OutputDataExecutor; o = OutputDataExecutor(); o.log.setLevel( 'DEBUG' ); o.checkForTransfers(); o.processAllPendingTransfers() " -o /Operations/belle/OutputData/fromJobToKEK-mdst/InputPath=$( cd ../mdst ; pwd) -o /Operations/belle/OutputData/fromJobToKEK-logs/InputPath=$( cd ../logs ; pwd)

fi

#

exit 0

