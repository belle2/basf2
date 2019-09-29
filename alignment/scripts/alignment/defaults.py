from basf2 import *
set_log_level(LogLevel.INFO)

import os
import sys
import multiprocessing

import ROOT
from ROOT import Belle2
from ROOT.Belle2 import MillepedeAlgorithm

from caf.framework import Calibration, CAF, Collection, LocalDatabase, CentralDatabase
from caf import backends
from caf import strategies

import rawdata as raw
import reconstruction as reco
import modularAnalysis as ana

import copy


def create_algorithm(db_components, ignore_undetermined=True, min_entries=10):
    algorithm = Belle2.MillepedeAlgorithm()

    std_components = ROOT.vector('string')()
    for component in db_components:
        std_components.push_back(component)
    algorithm.setComponents(std_components)

    algorithm.ignoreUndeterminedParams(ignore_undetermined)
    algorithm.setMinEntries(min_entries)

    algorithm.invertSign(True)

    return algorithm


def create_stdreco_path(components):
    path = create_path()

    path.add_module('Progress')
    # Remove all non-raw data to run the full reco again
    path.add_module('RootInput')  # , branchNames=input_branches, entrySequences=['0:5000'])
    path.add_module('Gearbox')
    path.add_module('Geometry')
    import rawdata as raw
    raw.add_unpackers(path)

    reco.add_reconstruction(path, pruneTracks=False, components=components)

    tmp = create_path()
    for m in path.modules():
        if m.name() == "PXDPostErrorChecker":
            continue
        if m.name() in ["PXDUnpacker", "CDCHitBasedT0Extraction", "TFCDC_WireHitPreparer"]:
            m.set_log_level(LogLevel.ERROR)
        if m.name() == "SVDSpacePointCreator":
            m.param("MinClusterTime", -999)
        tmp.add_module(m)
    path = tmp
    path.add_module('DAFRecoFitter')
    return path


def create_cosmics_path(components):
    path = create_path()
    path.add_module('Progress')
    # Remove all non-raw data to run the full reco again
    path.add_module('RootInput')  # , branchNames=input_branches, entrySequences=['0:5000'])
    path.add_module('Gearbox')
    path.add_module('Geometry')
    import rawdata as raw
    raw.add_unpackers(path)
    path.add_module('SetupGenfitExtrapolation')
    reco.add_cosmics_reconstruction(
        path,
        pruneTracks=False,
        skipGeometryAdding=True,
        addClusterExpertModules=False,
        data_taking_period='early_phase3',
        merge_tracks=True
    )
    tmp = create_path()
    for m in path.modules():
        if m.name() == "PXDPostErrorChecker":
            continue
        if m.name() in ["PXDUnpacker", "CDCHitBasedT0Extraction", "TFCDC_WireHitPreparer"]:
            m.set_log_level(LogLevel.ERROR)
        if m.name() == "SVDSpacePointCreator":
            m.param("MinClusterTime", -999)
        tmp.add_module(m)
    path.add_module('SetRecoTrackMomentum', automatic=True)
    path.add_module('DAFRecoFitter', pdgCodesToUseForFitting=[13])
    return path


def create_collector(db_components, **argk):
    m = register_module('MillepedeCollector')

    m.param('granularity', 'all')
    # Let's enable this always - will be in effect only if BeamSpot is in db_components
    m.param('calibrateVertex', True)
    # Not yet implemeted -> alwas OFF for now
    m.param('calibrateKinematics', False)
    m.param('minUsedCDCHitFraction', 0.85)
    m.param('minPValue', 0.0)
    m.param('externalIterations', 0)
    m.param('tracks', [])
    m.param('fitTrackT0', True)
    m.param('components', db_components)
    m.param('useGblTree', False)
    m.param('absFilePaths', True)

    m.param(argk)

    return m
