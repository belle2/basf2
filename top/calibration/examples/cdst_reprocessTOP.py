#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
from reconstruction import add_top_modules, add_cdst_output

# ---------------------------------------------------------------------------------------
# Example of reprocessing cdst files with new TOP calibration constants
#
# Note: replace local database name/location before running or comment it out
# ---------------------------------------------------------------------------------------


class ReplaceTOPLikelihoods(Module):
    ''' replacing TOP likelihoods in PIDLikelihoods with new values '''

    def event(self):
        ''' event function '''

        chargedStableSet = [Belle2.Const.electron,
                            Belle2.Const.muon,
                            Belle2.Const.pion,
                            Belle2.Const.kaon,
                            Belle2.Const.proton,
                            Belle2.Const.deuteron]

        for track in Belle2.PyStoreArray('Tracks'):
            pid = track.getRelated('PIDLikelihoods')
            # should unset TOP in PIDLikelihoods first, but such function is not available
            top = track.getRelated('TOPLikelihoods')
            if top and pid:
                if top.getFlag() == 1:
                    for chargedStable in chargedStableSet:
                        logL = top.getLogL(chargedStable)
                        pid.setLogLikelihood(Belle2.Const.TOP, chargedStable, logL)

# Database:
# - replace the name and location of the local DB before running!
# - one can even use several local DB's
# - payloads are searched for in the reverse order of DB's given below; therefore the new
#   calibration, if provided, is taken from the local DB.
use_central_database('development')  # some new stuff not in production tag
use_central_database('data_reprocessing_prod6')  # global tag used in production of cdst
use_local_database('zzTBCdb/localDB/localDB.txt', 'zzTBCdb/localDB/')  # new calibration

# Create path
main = create_path()

# input: cdst file(s)
roinput = register_module('RootInput')
main.add_module(roinput)

# geometry parameters
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
geometry.param('components', ['MagneticField', 'TOP'])
geometry.param('useDB', False)
main.add_module(geometry)

# Time Recalibrator
recalibrator = register_module('TOPTimeRecalibrator')
recalibrator.param('subtractBunchTime', False)
main.add_module(recalibrator)

# TOP reconstruction
add_top_modules(main)
for m in main.modules():
    if m.type() == "TOPBunchFinder":
        m.param('usePIDLikelihoods', True)

# Replace TOP in PID likelihoods with new values
main.add_module(ReplaceTOPLikelihoods())

# output: cdst file
add_cdst_output(main)

# Print progress
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print statistics
print(statistics)
