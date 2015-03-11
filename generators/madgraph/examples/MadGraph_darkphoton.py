#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
# MadGraph
#
# Torben Ferber
#
# Example production script using dark model to
# produce 100 events e+ e- -> gamma A' [->mu+ mu-]
# in the Belle II labframe
########################################################

from basf2 import *
import os

# parameters that can be modified
mg_nevents = '100'
mg_beamenergy = '5.28695'
mg_generate = 'e+ e- > a ap, ap > mu+ mu-'
mg_parameter_wap = '1.0e-3'
mg_parameter_map = '3.0e0'
mg_seed = '1'
mg_model = \
    os.path.expandvars('$BELLE2_LOCAL_DIR/generators/madgraph/models/darkphoton'
                       )

# full path to steering template file (full path to model must be inside the template steering file)
mg_steeringtemplate = \
    os.path.expandvars('$BELLE2_LOCAL_DIR/generators/madgraph/examples/run_darkphoton.steeringtemplate'
                       )

# full path to output directory
mg_outputdir = \
    os.path.expandvars('$BELLE2_LOCAL_DIR/generators/madgraph/output/darkphoton_mumu'
                       )

# -------------------------------------------------------
# no user input needed below this line

# full path to MadGraph externals
mg_externals = os.path.expandvars('$BELLE2_EXTERNALS_DIR/madgraph/bin/mg5_aMC')

# full path to steering file (generated automatically, will be overwritten (if existing) or created)
mg_steeringfile = \
    os.path.expandvars('$BELLE2_LOCAL_DIR/generators/madgraph/examples/run_darkphoton.steering'
                       )

# full path to run card (param_card is generated automatically, defaults are overwritten in the steering file)
mg_runcard = \
    os.path.expandvars('$BELLE2_LOCAL_DIR/generators/madgraph/cards/run_card.dat'
                       )

# Replace the output directory and the run card
mydict = {}
mydict['MGMODEL'] = mg_model
mydict['MGOUTPUT'] = mg_outputdir
mydict['MGRUNDCARD'] = mg_runcard
mydict['MGNEVENTS'] = mg_nevents
mydict['MGBEAMENERGY'] = mg_beamenergy
mydict['MGGENERATE'] = mg_generate
mydict['MGPARAMETERWAP'] = mg_parameter_wap
mydict['MGPARAMETERMAP'] = mg_parameter_map
mydict['MGSEED'] = mg_seed

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
subprocess.check_call(['gunzip', mg_outputdir
                      + '/Events/run_01/unweighted_events.lhe.gz'])

# read in via basf2
set_log_level(LogLevel.ERROR)
lhereader = register_module('LHEInput')
lhereader.param('makeMaster', True)
lhereader.param('runNum', 1)
lhereader.param('expNum', 1)
lhereader.param('inputFileList', [mg_outputdir
                + '/Events/run_01/unweighted_events.lhe'])
lhereader.param('useWeights', False)
lhereader.param('nInitialParticles', 2)
lhereader.param('nVirtualParticles', 0)
lhereader.param('boost2Lab', True)
lhereader.param('wrongSignPz', True)

# creating the path for the processing
main = create_path()

# Add lhereader module
main.add_module(lhereader)
print_params(lhereader)

# Add progress module
progress = register_module('Progress')
progress.set_log_level(LogLevel.INFO)
main.add_module(progress)

# Add rootoutput module
rootoutput = register_module('RootOutput')
rootoutput.param('outputFileName', 'LHEReaderMasterOutputDarkMuMu.root')
main.add_module(rootoutput)

# Add mcparticleprinter module
main.add_module('PrintMCParticles', logLevel=LogLevel.DEBUG,
                onlyPrimaries=False)

# Process
process(main)
