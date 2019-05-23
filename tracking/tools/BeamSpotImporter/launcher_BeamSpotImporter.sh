#!/bin/bash

#usage source launch_BeamSpotImporter.sh path_to_folder_relative_to_local_release

#each user should set these variables:
OUTPUT_ROOT=$BELLE2_LOCAL_DIR/$1/
IMPORTER=$BELLE2_LOCAL_DIR/tracking/tools/BeamSpotImporter/beamSpotImporter.py

echo "importer script = ${IMPORTER}"
echo "output folder = ${OUTPUT_ROOT}"

if [ ! -d ${OUTPUT_ROOT}  ]; then
    echo "WARNING!"
    echo "the output folder ${OUTPUT_ROOT} does not exist. Create it before launching recostruction."
    echo "mkdir ${OUTPUT_ROOT}"
    mkdir ${OUTPUT_ROOT}
fi


#change this variable only if you know what you are doing:
#NILS REPROCESSING, exp 7 runs 1118 - 1784
#INPUT_PATH="/group/belle2/users/nbraun/results/git_hash\=release-03-01-01/experiment_number\=7" 
#EXP 7 runs 1784 to the end
INPUT_PATH="/group/belle2/phase3/dqm/e0007/expreco/dqmserver/"
#EXP 8 runs 1784 to the last
#INPUT_PATH="/group/belle2/phase3/dqm/dqmsrv1/e0008/dqmhisto/"
# echo ${#INPUT_PATH}+12-2

cd ${OUTPUT_ROOT}
#NILS REPROCESSING
#ls ${INPUT_PATH}/run_number\=*/prefix\=physics/expressreco_dqm_histos.root > run.list
#EXP 7
ls ${INPUT_PATH}dqm_e0007r*.root > run.list

let iterator=0
while IFS="" read -r line; do
#NILS REPROCESSING
#    echo "basf2 ${IMPORTER} -- --exp 7 --run ${line:82+10:4} --dqm $line"
#    basf2 ${IMPORTER} -- --exp 7 --run ${line:82+10:4} --dqm $line --verbose
#EXP 7
    echo "basf2 ${IMPORTER} -- --exp 7 --run ${line:71-10:4} --dqm $line"
    bsub -q s -o importer_$iterator.log basf2 ${IMPORTER} -- --exp 7 --run ${line:71-10:4} --dqm $line
    let iterator=iterator+1
done < run_bucket6.list

cd -
echo "processed ${iterator} DQM files"
echo "localDB with BeamSpot located in ${OUTPUT_ROOT}"

echo "Thanks for using the BeamSpotImporter and its launcher!"