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
path.add_module("Geometry")

# We assume that we start from non-recontructed data
# as we change reco-constants in each iteration, std
# reco needs to be repeated after each calibration.
reco.add_reconstruction(path, pruneTracks=False)

# Now use analysis to select alignment tracks/decays
# Select single muons for aligment...
ana.fillParticleList('mu+:bbmu', 'muonID > 0.1 and useLabFrame(p) > 0.5', True, path)
# Pre-fit with beam+vertex constraint decays for muon pairs
ana.fillParticleList('mu+:qed', 'muonID > 0.1 and useCMSFrame(p) > 2.', writeOut=True, path=path)
ana.reconstructDecay('Z0:mumu -> mu-:qed mu+:qed', '', writeOut=True, path=path)
ana.vertexRaveDaughtersUpdate('Z0:mumu', 0.0, path=path, constraint='ipprofile')

millepede = MillepedeCalibration(['VXDAlignment', 'BeamParameters'],
                                 tracks=[],
                                 particles=['mu+:bbmu'],
                                 vertices=[],
                                 primary_vertices=['Z0:mumu'],
                                 path=path)
# For simulated data:
millepede.algo.invertSign()
# Add the constraints (auto-generated from hierarchy), so you can
# play with unfixing degrees of freedom below
# millepede.algo.steering().command('Fortranfiles')
# millepede.algo.steering().command('constraints.txt')

"""
    {{"PXD.Ying"}, {Belle2::VxdID(1, 0, 0, 1)}},
    {{"PXD.Yang"}, {Belle2::VxdID(1, 0, 0, 2)}},
    {{"SVD.Pat"}, {Belle2::VxdID(3, 0, 0, 1)}},
    {{"SVD.Mat"}, {Belle2::VxdID(3, 0, 0, 2)}}
"""
# millepede.fixPXDYing()
# millepede.fixPXDYang()
millepede.fixSVDPat()
# millepede.fixSVDMat()
# Now fix everything (even some non-existing stuff:-) except half-shells
for layer in range(1, 7):
    for ladder in range(1, 17):
        # Fix also all ladders
        millepede.fixVXDid(layer, ladder, 0)
        for sensor in range(1, 6):
            # Fix all sensors
            millepede.fixVXDid(layer, ladder, sensor)
            pass


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


# Or to run with local backend with e.g. 10 processes, do:
#
# millepede.max_files_per_collector_job = 1
# caf.backend = backends.Local(10)

caf.add_calibration(millepede.create('vxd_shells_beamspot', inputFiles))

caf.output_dir = 'caf_vxdbeam'
caf.run()
