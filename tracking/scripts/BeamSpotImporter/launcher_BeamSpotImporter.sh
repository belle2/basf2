#!/bin/bash

#usage source launch_BeamSpotImporter.sh path_to_folder_relative_to_local_release

#each user should set these variables:
OUTPUT_ROOT=$BELLE2_LOCAL_DIR/$1/
IMPORTER=$BELLE2_LOCAL_DIR/tracking/scripts/BeamSpotImporter/beamSpotImporter.py

echo "importer script = ${IMPORTER}"
echo "output folder = ${OUTPUT_ROOT}"

if [ ! -d ${OUTPUT_ROOT}  ]; then
    echo "WARNING!"
    echo "the output folder ${OUTPUT_ROOT} does not exist. Create it before launching recostruction."
    echo "mkdir ${OUTPUT_ROOT}"
    mkdir ${OUTPUT_ROOT}
fi

#do not change this variable:
INPUT_PATH="/group/belle2/users/nbraun/results/git_hash\=release-03-01-01/experiment_number\=7"
# echo ${#INPUT_PATH}+12-2

cd ${OUTPUT_ROOT}
ls ${INPUT_PATH}/run_number\=*/prefix\=physics/expressreco_dqm_histos.root > run.list

let iterator=0
while IFS="" read -r line; do
    echo "basf2 ${IMPORTER} -- --exp 7 --run ${line:82+10:4} --dqm $line"
    basf2 ${IMPORTER} -- --exp 7 --run ${line:82+10:4} --dqm $line --verbose
    let iterator=iterator+1
done < run.list

cd -
echo "processed ${iterator} DQM files"
echo "localDB with BeamSpot located in ${OUTPUT_ROOT}"

echo "Thanks for using the BeamSpotImporter and its launcher!"