#!/usr/bin/env python3
# Author: Marcel Hohmann (marcel.hohmann@desy.de)

import basf2
from basf2 import process, statistics
from modularAnalysis import inputMdst, matchMCTruth, variablesToNtuple
from stdV0s import stdKshorts
import sys
import os

try:
    input_file_name = str(sys.argv[1])
except BaseException:
    input_file_name = '/hsm/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00004770/'\
        's00/e0000/4S/r00000/mixed/mdst/sub00/mdst_00000*_prod00004770_task0000000*.root'

try:
    identifier = sys.argv[2]
except BaseException:
    identifier = 'Kshort_FastBDT.xml'  # by default train to xml then upload to localdb


tree_name = 'ks_training_variables'
training_file_name = 'KshortClassifierTrainingData.root'

my_variables = ['SigM',
                'formula( E / E_uncertainty )',
                'formula( flightTime / flightTimeErr)',
                'cosAngleBetweenMomentumAndVertexVector',
                'min(abs(daughter(0, d0)),abs(daughter(1, d0)))',
                'formula(daughter(0,  pionID) + daughter(1,  pionID))'
                ]

target_variable = 'isSignal'


# --- create training data set ---
training_path = basf2.core.Path()
inputMdst('default', input_file_name, path=training_path)
stdKshorts(path=training_path)
matchMCTruth('K_S0:merged', path=training_path)

variablesToNtuple('K_S0:merged',
                  my_variables + [target_variable],
                  tree_name,
                  training_file_name,
                  path=training_path
                  )

process(training_path, int(2e5))
print(statistics)


# --- train variables ---
training_string = 'basf2_mva_teacher --datafiles {data_files} --treename {tree_name}'\
                  ' --identifier {identifier} --variables {variables} --target_variable'\
                  ' {target_variable} --method FastBDT --nTrees 400 --nCutLevels 8 --nLevels 4'.format(
                      data_files=training_file_name,
                      tree_name=tree_name,
                      identifier=identifier,
                      variables=''.join([' "%s" ' % var for var in my_variables]),
                      target_variable=target_variable)

os.system(training_string)

ex_b = 0   # experiment begin, 0 for all of them
ex_e = -1   # experiment end, -1 for all of them
run_b = 0   # run begin, 0 for all
run_e = -1   # run end, -1 for all of them
tag_name = "development"  # global tag name

upload = False  # upload to conditions database
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
    os.system(f"conditionsdb upload {tag_name} {data_base_file}")

if remove_local_files:
    os.system('rm -r {}'.format(here + '/localdb/'))
    os.system(f'rm {here}/{training_file_name}')
    os.system(f'rm {here}/{identifier}')
