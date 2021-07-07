##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import basf2 as b2
import subprocess

subprocess.run(['b2conditionsdb-recommend'], check=True)

subprocess.run(['b2conditionsdb-recommend', '--oneline'], check=True)

subprocess.run(['b2conditionsdb-recommend', b2.find_file('analysis/tests/mdst.root')], check=True)
