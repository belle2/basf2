#!/usr/bin/env python3
# -*-  coding: utf-8 -*-
# Author: Marcel Hohmann (marcel.hohmann@desy.de)

from basf2 import *
from modularAnalysis import *
from stdV0s import *
import sys

try:
    input_file_name = str(sys.argv[1])
except BaseException:
    input_file_name = '/hsm/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00004770/'\
        's00/e0000/4S/r00000/mixed/mdst/sub00/mdst_00000*_prod00004770_task0000000*.root'

try:
    identifier = str(sys.argv[2])
except BaseException:
    identifier = 'Kshort_FastBDT.xml'  # by default train to xml then upload to localdb


tree_name = 'ks_training_variables'
training_file_name = 'KshortClassifierTrainingData.root'

variables = ['SigM',
             'formula( E / E_uncertainty )',
             'pt',
             'cosTheta',  # polar angle
             'formula( x / x_uncertainty )',
             'formula( y / y_uncertainty )',
             'formula( z / z_uncertainty )',
             'dr',
             'formula( flightTime / flightTimeErr )',
             'flightDistance',
             'cosAngleBetweenMomentumAndVertexVector',
             'formula( daughter(0, pValue) + daughter(1, pValue) )',
             'min(abs(daughter(0, d0)),abs(daughter(1, d0)))',
             'max(abs(daughter(0, d0)),abs(daughter(1, d0)))',
             'min(abs(daughter(0, z0)),abs(daughter(1, z0)))',
             'formula(daughter(0,  pionID) + daughter(1,  pionID))',
             'formula(daughter(0,nCDCHits) + daughter(1,nCDCHits))',
             'formula(daughter(0,nVXDHits) + daughter(1,nVXDHits))',
             ]

target_variable = 'isSignal'


# --- create training data set ---
inputMdst('default', input_file_name)
stdKshorts()
matchMCTruth('K_S0:all')

variablesToNtuple('K_S0:all',
                  variables + [target_variable],
                  tree_name,
                  training_file_name,
                  )

process(analysis_main, int(1e5))
print(statistics)

# --- train variables ---
training_string = 'basf2_mva_teacher --datafiles {data_files} --treename {tree_name}'\
                  ' --identifier {identifier} --variables {variables} --target_variable'\
                  ' {target_variable} --method FastBDT --nTrees 400 --nCutLevels 8 --nLevels 4'.format(
                      data_files=training_file_name,
                      tree_name=tree_name,
                      identifier=identifier,
                      variables=''.join([' "%s" ' % var for var in variables]),
                      target_variable=target_variable)

os.system(training_string)

ex_b = 0   # experiment begin, 0 for all of them
ex_e = -1   # experiment end, -1 for all of them
run_b = 0   # run begin, 0 for all
run_e = -1   # run end, -1 for all of them
tag_name = "development"  # global tag name

upload = True  # upload to conditions database
remove_local_files = False  # delete local db and training data

# upload to local database from xml file
os.system('basf2_mva_upload --identifier {identifier} --db_identifier {identifier_db}'
          ' --begin_experiment {ex_b} --end_experiment {ex_e} --begin_run {run_b} --end_run {run_e}'.format(
              identifier=identifier,
              identifier_db=identifier.split(".xml")[0],
              ex_b=ex_b,
              ex_e=ex_e,
              run_b=run_b,
              run_e=run_e))

here = os.getcwd()
data_base_file = here + "/localdb/database.txt"

# upload to global database
if upload:
    os.system("conditionsdb upload {TAGNAME} {DATABASEFILE}".format(TAGNAME=tag_name, DATABASEFILE=data_base_file))

if remove_local_files:
    os.system('rm -r {}'.format(here + '/localdb/'))
    os.system('rm {}/{}'.format(here, training_file_name))
    os.system('rm {}/{}'.format(here, identifier))
