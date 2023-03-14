##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Test if b2info-luminosity works.
Note that the RunDB token must be passed to the test as environment variable (BELLE2_RUNDB_TOKEN).
'''


import os
import subprocess

import b2test_utils as b2tu

lumiDB_path = b2tu.require_file('luminosity/', 'validation')

if __name__ == '__main__':

    rundb_token = os.getenv('BELLE2_RUNDB_TOKEN', '')
    if rundb_token == '':
        b2tu.skip_test('The BELLE2_RUNDB_TOKEN environment variable is not set')

    with b2tu.clean_working_directory():

        # Check few configurations
        # Please note: --what offline cannot be properly tested (it works only at KEKCC)

        subprocess.check_call(
            ['b2info-luminosity', '--exp', '18', '--runs', '100,200-300,600-650', '--what',
             'online', '--token', os.environ['BELLE2_RUNDB_TOKEN']]
        )

        subprocess.check_call(
            ['b2info-luminosity', '--exp', '18', '--runs', '100,200-300,600-650', '--what',
             'online', '--verbose', '--token', os.environ['BELLE2_RUNDB_TOKEN']]
        )

        subprocess.check_call(
            ['b2info-luminosity', '--exp', '18', '--runs', '200-300', '--what', 'online',
             '--good', '--beam-energy', '4S', '--token', os.environ['BELLE2_RUNDB_TOKEN']]
        )

        subprocess.check_call(
            ['b2info-luminosity', '--exp', '18', '--runs', '100,600-650', '--what', 'online',
             '--beam-energy', '4S_offres', '--token', os.environ['BELLE2_RUNDB_TOKEN']]
        )

        subprocess.check_call(
            ['b2info-luminosity', '--exp', '12,14', '--what', 'offline',
             '--dboffline', f'{lumiDB_path}/test_OffLineLuminosityProc.db', '--token', os.environ['BELLE2_RUNDB_TOKEN']]
        )

        subprocess.check_call(
            ['b2info-luminosity', '--exp', '12,14', '--what', 'offline', '--proc', 'proc12',
             '--dboffline', f'{lumiDB_path}/test_OffLineLuminosityProc.db', '--token', os.environ['BELLE2_RUNDB_TOKEN']]
        )
