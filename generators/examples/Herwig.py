#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Example showing how to run the Herwig continuum generator (e.g. for ccbar production) using
`add_herwig_continuum_generator` from the `herwig` module.

The generator runs in three stages (all managed internally):
  Stage 1: KKMC produces N ccbar quark pairs and writes an LHE file.
  Stage 2: Herwig fragments all N events in one batch call and produces a HepMC output.
  Stage 3: HerwigHepMCFragmentationModule reads the HepMC output,
           merges the KKMC truth tree, and applies EvtGen.

Usage
-----
MCri or local run example (beam energy from default conditions DB, i.e., it defaults to exp 0, run 1 if left unspecified):
    basf2 Herwig.py -n 100

Run-dependent example (per-run beam energy from MCrd conditions DB; assuming data liaison configures global tags):
    basf2 Herwig.py --experiment 12 --run 798 -n 100

experiment, run, and number of events are read from the basf2 framework
arguments (--experiment, --run, -n, --skip-events) via Belle2.Environment.Instance(),
expList and runList are forwarded explicitly to add_herwig_continuum_generator()
so that Stage 1 queries the conditions DB for the correct per-run beam parameters.
'''

import basf2 as b2
from herwig import add_herwig_continuum_generator
from ROOT import Belle2

# -----------------------------------------------------------------------
# Read experiment, run, and event count from command line arguments.
# -----------------------------------------------------------------------
_exp = Belle2.Environment.Instance().getExperimentOverride()               # --experiment
experiment = _exp if _exp >= 0 else 0   # -1 means not set; default to exp 0
_run = Belle2.Environment.Instance().getRunOverride()                      # --run
run = _run if _run >= 0 else 1          # -1 means not set; default to run 1
lastEventNumber = Belle2.Environment.Instance().getNumberEventsOverride()  # -n
firstEventNumber = Belle2.Environment.Instance().getSkipEventsOverride()   # --skip-events
nevents = lastEventNumber - firstEventNumber

# -----------------------------------------------------------------------
# Path
# -----------------------------------------------------------------------
main = b2.create_path()

# Flags override the experiment and run numbers, but if these are set explicitly in EventInfoSetter,
# they must match in add_herwig_continuum_generator().
main.add_module('EventInfoSetter')

# 3-stage Herwig continuum generator.
# To hardcode a specific beam energy, use
# beamparametersLabel='Y4S', 'Y4S-off', 'Y5S', 'Y1S', etc. instead of None.
add_herwig_continuum_generator(
    path=main,
    finalstate='ccbar',  # uubar, ddbar, ssbar, ccbar
    nevents=nevents,
    expList=[experiment],
    runList=[run],
    beamparametersLabel=None,  # None (default): access beam parameters from DB for a given exp/run
)

main.add_module('Progress')
main.add_module('RootOutput', outputFileName='herwig_continuum.root')
# main.add_module('PrintMCParticles', logLevel=b2.LogLevel.DEBUG, onlyPrimaries=False)

b2.process(main, calculateStatistics=True)
print(b2.statistics)
