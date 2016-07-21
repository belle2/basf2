# This is an extremely simple script that just runs on of the CAF test scripts.
# This should be run after the 1_create_sample_DSTs.sh script so that there is
# data ready to 'calibrate'.

# First define and create directories to store the output.
EXAMPLE_DIR=${BELLE2_LOCAL_DIR}/calibration/examples
DATA_DIR=${EXAMPLE_DIR}/example_calibration_dsts
pushd $EXAMPLE_DIR

# Feel free to run either of these basf2 scripts. Note that the CAF curently won't let
# you run a new calibration using the same output directory so this will
# fail quickly if you try and re-run a script without removing the CAF output
# of the previous process.

basf2 ${EXAMPLE_DIR}/caf_simplest.py $DATA_DIR
#basf2 ${EXAMPLE_DIR}/caf_multiple_options.py $DATA_DIR
#basf2 ${EXAMPLE_DIR}/caf_vxd.py $DATA_DIR

popd > /dev/null
