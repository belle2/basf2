#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2.core
from basf2 import process, statistics, register_module
from modularAnalysis import inputMdst, tagCurlTracks
from stdCharged import stdPi
import sys
import os

try:
    input_file_name = str(sys.argv[1])
except BaseException:
    input_file_name = '/hsm/belle2/bdata/MC/release-02-00-01/DB00000411/MC11/prod00005678/'\
        's00/e0000/4S/r00000/mixed/mdst/sub00/mdst_00000*_prod00005678_task0000000*.root'

upload = False  # upload to conditions database
remove_local_files = False  # delete local db and training data
tag_name = 'development'

# names used by the CurlTagger module:
training_file_name = 'CurlTagger_TrainingData_BelleII.root'
identifier = 'CurlTagger_FastBDT_BelleII'

# --- create training data set and train the classifier ---
training_path = basf2.core.Path()

inputMdst('default', input_file_name, path=training_path)
stdPi('all', path=training_path)
tagCurlTracks('pi+:all', train=True, selectorType='mva', path=training_path)

training_path.add_module('ProgressBar')

process(training_path, int(2e5))
print(statistics)


here = os.getcwd()
data_base_file = here + "/localdb/database.txt"

# upload to global database
if upload:
    os.system(f"b2conditionsdb-upload {tag_name} {data_base_file}")

if remove_local_files:
    os.system('rm -r {}'.format(here + '/localdb/'))
    os.system(f'rm {here}/{training_file_name}')
