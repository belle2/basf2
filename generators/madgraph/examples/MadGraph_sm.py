#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
# MadGraph                                                               #
#                                                                        #
# Example production script using SM model to produce                    #
# 100 events e+ e- -> mu+ mu- in the Belle II labframe                   #
##########################################################################

import basf2 as b2
from beamparameters import add_beamparameters
import os
import subprocess

# parameters that can be modified
mg_nevents = '100'
mg_beamenergy = '10.355/2.'
mg_generate = 'e+ e- > mu+ mu-'

# full path to steering template file (full path to model must be inside the template steering file)
mg_steeringtemplate = \
    os.path.expandvars('$BELLE2_LOCAL_DIR/generators/madgraph/examples/run_sm.steeringtemplate'
                       )

# full path to output directory
mg_outputdir = \
    os.path.expandvars('$BELLE2_LOCAL_DIR/generators/madgraph/output/sm_mumu')

# -------------------------------------------------------
# no user input needed below this line

# full path to MadGraph externals
mg_externals = 'mg5_aMC'

# full path to steering file (generated automatically, will be overwritten (if existing) or created)
mg_steeringfile = \
    os.path.expandvars('$BELLE2_LOCAL_DIR/generators/madgraph/examples/run_sm.steering'
                       )

# full path to run card (param_card is generated automatically, defaults are overwritten in the steering file)
mg_runcard = \
    os.path.expandvars('$BELLE2_LOCAL_DIR/generators/madgraph/cards/run_card.dat'
                       )

# Replace the output directory and the run card
mydict = {}
mydict['MGOUTPUT'] = mg_outputdir
mydict['MGRUNDCARD'] = mg_runcard
mydict['MGNEVENTS'] = mg_nevents
mydict['MGBEAMENERGY'] = mg_beamenergy
mydict['MGGENERATE'] = mg_generate

fp1 = open(mg_steeringfile, 'w')
fp2 = open(mg_steeringtemplate, 'r')
data = fp2.read()
fp2.close()
for (key, value) in mydict.items():
    data = data.replace(key, value)
fp1.write(data)
fp1.close()

# run MadGraph
subprocess.check_call([mg_externals, mg_steeringfile])

# gunzip the unweighted output file
subprocess.check_call(['gunzip', mg_outputdir + '/Events/run_01/unweighted_events.lhe.gz'])

# read in via basf2
b2.set_log_level(b2.LogLevel.ERROR)
lhereader = b2.register_module('LHEInput')
lhereader.param('makeMaster', True)
lhereader.param('runNum', 1)
lhereader.param('expNum', 1)
lhereader.param('inputFileList', [mg_outputdir + '/Events/run_01/unweighted_events.lhe'])
lhereader.param('useWeights', False)
lhereader.param('nInitialParticles', 2)
lhereader.param('nVirtualParticles', 0)
lhereader.param('boost2Lab', True)
lhereader.param('wrongSignPz', True)

# creating the path for the processing
b2.set_log_level(b2.LogLevel.ERROR)

# creating the path for the processing
main = b2.create_path()

# beam parameters
beamparameters = add_beamparameters(main, "Y3S")

# Add lhereader module
main.add_module(lhereader)
b2.print_params(lhereader)

# Add progress module
progress = b2.register_module('Progress')
progress.set_log_level(b2.LogLevel.INFO)
main.add_module(progress)

# Add rootoutput module
rootoutput = b2.register_module('RootOutput')
rootoutput.param('outputFileName', 'LHEReaderMasterOutputSM.root')
main.add_module(rootoutput)

# Add mcparticleprinter module
main.add_module('PrintMCParticles', logLevel=b2.LogLevel.DEBUG,
                onlyPrimaries=False)

# Process
b2.process(main)