#!/bin/zsh
# -*- coding: utf-8 -*-
# Jo-Frederik.krohn@desy.de
import sys
import os


# USE THIS TO TRAIN AND UPLOAD A NEW CLASSIFIER FOR THE KLONG ID
# USE THE OTHER SCRIPT TO GENERATE DATA. THEN TRAIN AND UPLOAD WITH &THIS.

# only thing to set is the path were you have the data to train the classifier on.

try:
    data_path = sys.argv[1]
except BaseException:
    data_path = "/home/belle2/jkrohn/root_files/training_R1/generic/*GENERIC*.root"
try:
    identifier = sys.argv[2]
except BaseException:
    # the default name used in KLMExpert
    identifier = "KLM_fBDT_10xbkg100k.xml"

ex_b = 0   # experiment begin, 0 for all of them
ex_e = -1   # experiment end, -1 for all of them
run_b = 0   # run begin, 0 for all
run_e = -1   # run end, -1 for all of them
upload = True  # upload to conditions database
remove_db = True  # delete local db
tag_name = "development"  # global tag name

training_string = 'basf2_mva_teacher --datafiles {data_path} --treename KLMdata --identifier {identifier} --variables\
    "KLMnLayer" "KLMnInnermostlayer" "KLMglobalZ" "KLMtime" "KLMdistToNextCl" "KLMenergy" "KLMTrackSepDist"\ 
    "KLMInitialtrackSepAngle" "KLMTrackRotationAngle" "KLMTrackSepAngle" "KLMhitDepth" "KLMECLenergy" "KLMECLE9oE25" \
    "KLMECLtiming" "KLMECLmintrackDist" "KLMECLZMVA" "KLMECLZ40" "KLMECLZ51"\
    - -target_variable isSignal - -method FastBDT - -nTrees 400 - -nCutLevels 8 \
    - -nLevels 4'.format(
        data_path=data_path,
        identifier=identifier)

os.system(training_string)

# "upload" to localdb
os.system(
    "basf2_mva_upload --identifier {identifier} --db_identifier {identifier_db} \
        --begin_experiment {ex_b} --end_experiment {ex_e} --begin_run {run_b} --end_run {run_e} ".format(
        identifier=identifier,
        identifier_db=identifier.split(".xml")[0],
        ex_b=ex_b,
        ex_e=ex_e,
        run_b=run_b,
        run_e=run_e))

here = os.getcwd()
data_base_file = here + "/localdb/database.txt"
if upload:
    os.system("conditionsdb upload {TAGNAME} {DATABASEFILE}".format(TAGNAME=tag_name, DATABASEFILE=data_base_file))
if remove_db:
    os.system("rm -r {}".format(data_base_file.split("database.txt")[0]))
