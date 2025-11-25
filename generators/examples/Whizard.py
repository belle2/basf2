##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
This example shows how to run WHIZARD using our `whizard` module.
'''

import shutil

import basf2 as b2
import whizard
from ROOT import Belle2


# get the relevant information from command line arguments
experiment = Belle2.Environment.Instance().getExperimentOverride()  # --experiment
run = Belle2.Environment.Instance().getRunOverride()                # --run
lastEventNumber = Belle2.Environment.Instance().getNumberEventsOverride()      # -n
firstEventNumber = Belle2.Environment.Instance().getSkipEventsOverride()       # --skip-events
events = lastEventNumber - firstEventNumber

# run WHIZARD using the `whizard` module
path, lhe, log = whizard.run_whizard(process='mumumumu', experiment=experiment, run=run, events=events)

# run basf2
main = b2.Path()
main.add_module('EventInfoSetter')
main.add_module('LHEInput',
                inputFileList=lhe,
                createEventMetaData=False,
                nInitialParticles=2,
                nVirtualParticles=2,
                wrongSignPz=False)
main.add_module('BoostMCParticles')
main.add_module('SmearPrimaryVertex')
main.add_module('Progress')
main.add_module('PrintMCParticles', showStatus=True, showMomenta=True)
main.add_module('RootOutput')
b2.process(main)
print(b2.statistics)

# remove the files produced by WHIZARD (if necessary)
shutil.rmtree(path, ignore_errors=True)
