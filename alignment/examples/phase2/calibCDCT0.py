#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2

import os
import sys
import ROOT

from ROOT import Belle2

from caf import backends
from caf.framework import Calibration, CAF
from alignment import MillepedeCalibration

import reconstruction as reco
import modularAnalysis as ana

inputFiles = [os.path.abspath(file) for file in Belle2.Environment.Instance().getInputFilesOverride()]
if not len(inputFiles):
    print(' Please specify input files for calibration via the -i option of basf2')
    sys.exit(1)

# Pre-collector full standard reconstruction path
path = basf2.create_path()
path.add_module("RootInput")
path.add_module("Gearbox")

# We assume that we start from non-recontructed data
# as we change reco-constants in each iteration, std
# reco needs to be repeated after each calibration.
reco.add_reconstruction(path, pruneTracks=False)

# Now use analysis to select alignment tracks/decays
# Select single muons for aligment...
# ana.fillParticleList('mu+:bbmu', 'muonID > 0.1 and useLabFrame(p) < 5.0', True, path)
# Pre-fit with beam+vertex constraint decays for muon pairs
ana.fillParticleList('mu+:qed', 'muonID > 0.1 and useCMSFrame(p) > 5.0', writeOut=True, path=path)
ana.reconstructDecay('Z0:mumu -> mu-:qed mu+:qed', '', writeOut=True, path=path)
ana.vertexRaveDaughtersUpdate('Z0:mumu', 0.0, path=path, constraint='ipprofile')

millepede = MillepedeCalibration(['CDCTimeZeros'],
                                 tracks=[],
                                 particles=['mu+:bbmu'],
                                 vertices=[],
                                 primary_vertices=['Z0:mumu'],
                                 path=path)
# Use inversion as digonalization is about 10 times slower
# and takes really long for all 14k wires
millepede.set_command('method inversion 3 0.1')
# Set num threads for millepede reading and local fits
# millepede.set_command('threads 4 4')

# For simulated data:
millepede.algo.invertSign()
# This is needed to get payloads even if for some wire not enough data
# for calibration is available
millepede.algo.ignoreUndeterminedParams(True)

# Fix layer 0, wire 0 to T0=0
millepede.fixCDCTimeZero(Belle2.WireID(0, 0).getEWire())

# Note that here we replace the helper class by real Calibration class of CAF
millepede = millepede.create('cdc_t0', inputFiles)

caf = CAF()

# Uncomment following line to use different global tag for payloads (not found in local DB below)
# Default can be loaded from basf2.get_default_global_tags()
#
# millepede.use_central_database(global_tag=basf2.get_default_global_tags())


# For testing misalignment, set it up in a local DB and uncomment following (with path to your local DB)
#
# millepede.use_local_database(os.path.abspath('localdb/database.txt'), directory="")


# Uncomment following to run on batch system (KEKCC)
#
# millepede.max_files_per_collector_job = 1
# millepede.backend_args = {"queue": "s"}
# caf.backend = backends.LSF()


# Or to run with local backend with e.g. 2 processes, do:
#
# millepede.max_files_per_collector_job = 1
# caf.backend = backends.Local(2)

caf.add_calibration(millepede)

caf.output_dir = 'caf_output'
caf.run()
