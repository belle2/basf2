#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2
from ROOT import Belle2

import generators as gen
import simulation as sim
import rawdata as raw
import reconstruction as reco
import modularAnalysis as ana
import vertex as vtx

import alignment.parameters
import alignment.constraints

import os

import millepede_calibration as mpc
from caf.framework import CAF
from caf import backends


def generate_test_data(filename):
    main = basf2.create_path()

    main.add_module("EventInfoSetter", evtNumList=[200])
    # add_beamparameters(main, "Y4S")
    main.add_module('Gearbox')
    main.add_module('Geometry')

    gen.add_kkmc_generator(main, 'mu-mu+')

    sim.add_simulation(main)

    main.add_module("RootOutput", outputFileName=filename)
    main.add_module("Progress")

    basf2.process(main)
    print(basf2.statistics)
    return os.path.abspath(filename)


def diMuonCollection(name="diMuonCollection", add_unpackers=True):
    path = basf2.create_path()

    path.add_module('Progress')
    # Remove all non-raw data to run the full reco again
    path.add_module('RootInput')  # , branchNames=input_branches, entrySequences=['0:5000'])
    path.add_module('Gearbox')
    path.add_module('Geometry')

    if add_unpackers:
        raw.add_unpackers(path)

    reco.add_reconstruction(path, pruneTracks=False)

    tmp = basf2.create_path()
    for m in path.modules():
        if m.name() == "PXDPostErrorChecker":
            m.param('CriticalErrorMask', 0)
        if m.name() in ["PXDUnpacker", "CDCHitBasedT0Extraction", "TFCDC_WireHitPreparer"]:
            m.set_log_level(basf2.LogLevel.ERROR)
        if m.name() == "SVDSpacePointCreator":
            m.param("MinClusterTime", -999)
        tmp.add_module(m)
    path = tmp
    path.add_module('DAFRecoFitter')

    ana.fillParticleList('mu+:mu_dimuon', 'abs(formula(z0)) < 0.5 and abs(d0) < 0.5 and nTracks == 2', writeOut=True, path=path)
    ana.reconstructDecay('Z0:mumu -> mu-:mu_dimuon mu+:mu_dimuon', '', writeOut=True, path=path)
    vtx.raveFit('Z0:mumu', 0.001, daughtersUpdate=True, silence_warning=True, path=path)

    return mpc.make_collection(name, path=path, primaryVertices=['Z0:mumu'])


def get_calibration(files, tags):

    cal = mpc.create(
      name='alignment',
      dbobjects=['VXDAlignment', 'BeamSpot'],
      collections=[
        diMuonCollection(name="dimuon_skim", add_unpackers=False)
        ],
      constraints=[
        alignment.constraints.VXDHierarchyConstraints(),
        ],
      fixed=alignment.parameters.vxd_sensors() + alignment.parameters.vxd_ladders(),
      commands=[
        'method diagonalization 3 0.1',
        'scaleerrors 1. 1.',
        ('printcounts', None)

        ],

      tags=tags,
      files=files,

      timedep=[([], [(0, 0, 0)])],
      params=dict(minPValue=0.001, externalIterations=0))

    return cal


if __name__ == '__main__':
    input_files = [os.path.abspath(file) for file in Belle2.Environment.Instance().getInputFilesOverride()]

    if not len(input_files):
        outfile = Belle2.Environment.Instance().getOutputFileOverride()
        if not outfile:
            outfile = 'cafBeamSpotVXDHalfShells_TestData.root'
        print("No input file provided. This will now generate test data into {} and run over it.".format(outfile))
        print("It will take couple of minutes.")
        print("You can set this file as input next time using -i option of basf2")

        input_files = [generate_test_data(outfile)]

    cal = get_calibration(
      files={'dimuon_skim': input_files},
      tags=[tag for tag in basf2.conditions.default_globaltags])

    cal.max_iterations = 0

    cal_fw = CAF()
    cal_fw.add_calibration(cal)
    cal_fw.backend = backends.Local(1)

    cal_fw.run()
