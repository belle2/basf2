#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# MadGraph  Version2.6.1 needed for ISR
#
# Torben Ferber
#
# Example production script using dark model to
# produce 100 events e+ e- -> gamma A' [->mu+ mu-]
# in the Belle II labframe at the Y(4S)
#
# Qiang Li updated for including ISR
# https://confluence.desy.de/display/BI/WG8+Meeting+%28Dark+Sector%29+January+30th+2018+08%3A00-09%3A00+AM+JST
# Setting isr=1 to switch on ISR, otherwise not
########################################################

from basf2 import *
from beamparameters import add_beamparameters
import os
import subprocess

# parameters that can be modified
isr = 0
if isr == 1:
    mg_lpp1 = '3'
    mg_lpp2 = '-3'
else:
    mg_lpp1 = '0'
    mg_lpp2 = '0'

mg_nevents = '100'
mg_beamenergy = '10.58/2.'
mg_generate = 'e+ e- > a ap, ap > mu+ mu-'
mg_parameter_wap = '0.03102254'
mg_parameter_map = '1.0e0'
mg_seed = '1'
mg_ge = '0.3028177'
mg_gchi = '0.0'
mg_el = '0.2'
mg_ea = '0.2'
mg_etaa = '3.13'
mg_etal = '3.13'
mg_mll = '0.2'
mg_bwcutoff = '200.'

mg_model = \
    os.path.expandvars('$BELLE2_LOCAL_DIR/generators/madgraph/models/darkphoton'
                       )

# full path to steering template file (full path to model must be inside the template steering file)
if isr == 1:
    mg_steeringtemplate = \
        os.path.expandvars('$BELLE2_LOCAL_DIR/generators/madgraph/examples/run_darkphoton_isr.steeringtemplate'
                           )
else:
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
mg_externals = 'mg5_aMC'

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
mydict['MGlpp1'] = mg_lpp1
mydict['MGlpp2'] = mg_lpp2
mydict['MGge'] = mg_ge
mydict['MGgchi'] = mg_gchi
mydict['MGel'] = mg_el
mydict['MGea'] = mg_ea
mydict['MGetaa'] = mg_etaa
mydict['MGetal'] = mg_etal
mydict['MGmll'] = mg_mll
mydict['MGbwcutoff'] = mg_bwcutoff

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


# creating the path for the processing
set_log_level(LogLevel.ERROR)

# creating the path for the processing
main = create_path()

# beam parameters
beamparameters = add_beamparameters(main, "Y4S")

lhereader = register_module('LHEInput')
lhereader.param('makeMaster', True)
lhereader.param('inputFileList', [mg_outputdir + '/Events/run_01/unweighted_events.lhe'])
lhereader.param('useWeights', False)
lhereader.param('nInitialParticles', 2)
lhereader.param('nVirtualParticles', 0)
lhereader.param('boost2Lab', True)
lhereader.param('wrongSignPz', True)


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
# main.add_module('PrintMCParticles', logLevel=LogLevel.DEBUG,
#                onlyPrimaries=False)

# Process
process(main)
