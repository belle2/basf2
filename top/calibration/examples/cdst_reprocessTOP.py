#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
from ROOT import Belle2
from reconstruction import add_top_modules, add_cdst_output

# ---------------------------------------------------------------------------------------
# Example of reprocessing cdst files with new TOP calibration constants
#
# Note: replace local database name/location before running or comment it out
#       check the global tag: it must be the same as used in production of input file(s)
# ---------------------------------------------------------------------------------------


class ReplaceTOPLikelihoods(b2.Module):
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
# - payloads are searched for in the reverse order of DB's given below;
#   therefore the new calibration, if provided, is taken from the local DB.
# - one can even use several local DB's
b2.use_central_database('data_reprocessing_proc7')  # global tag used in production of cdst
b2.use_local_database('localDB/localDB.txt', 'localDB/')  # new calibration

# Create path
main = b2.create_path()

# input: cdst file(s)
roinput = b2.register_module('RootInput')
main.add_module(roinput)

# Initialize TOP geometry parameters (creation of Geant geometry is not needed)
main.add_module('TOPGeometryParInitializer')

# Time Recalibrator
recalibrator = b2.register_module('TOPTimeRecalibrator')
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
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print statistics
print(b2.statistics)
