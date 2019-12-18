# -*- coding: utf-8 -*-

"""
Airflow script for automatic CDC dEdx calibration. It is currently for the electron based
calibration, where at present only RunGain, Cosine and WireGain are implimented. The remaining
two 2D and 1D will be implimented in the near future.

Second part called "Hadron calibration" are not compitable with CAF/AirFlow
and will be done offline for a while.

Author: Jitendra Kumar
Contact: jkumar@andrew.cmu.edu
"""
import basf2
import os
import sys
import ROOT
from ROOT import gSystem
from ROOT import Belle2
from ROOT.Belle2 import CDCDedxRunGainAlgorithm, CDCDedxCosineAlgorithm, CDCDedxWireGainAlgorithm
from caf.framework import Calibration, CAF, Collection
from caf.strategies import SequentialRunByRun, SingleIOV
from prompt import CalibrationSettings
gSystem.Load('libreconstruction.so')
ROOT.gROOT.SetBatch(True)

settings = CalibrationSettings(name="CDC dedx",
                               expert_username="jikumar",
                               description=__doc__,
                               input_data_formats=["cdst"],
                               input_data_names=["hlt_bhabha"],
                               depends_on=[])

# REQUIRED FUNCTION used b2caf-prompt-run tool #


def get_calibrations(input_data, **kwargs):

    import basf2
    file_to_iov_physics = input_data["hlt_bhabha"]

    fulldataMode = False

    if fulldataMode:
        input_files_physics = file_to_iov_physics
    else:
        max_files_for_maxevents = 100000  # 100k events max (around 5-6 max files)
        from prompt.utils import filter_by_max_events_per_run
        reduced_file_to_iov_physics = filter_by_max_events_per_run(file_to_iov_physics, max_files_for_maxevents)
        input_files_physics = list(reduced_file_to_iov_physics.keys())
        basf2.B2INFO(f"Total number of files actually used as input = {len(input_files_physics)}")

    requested_iov = kwargs.get("requested_iov", None)
    from caf.utils import IoV
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    # ----------1. Run Gain Pre (No Payload saving)
    # Rungain Precollector path
    Calibrate_RGPre = basf2.create_path()
    Calibrate_RGPre.add_module(
        'CDCDedxCorrection',
        relativeCorrections=False,
        scaleCor=True,
        runGain=True,
        cosineCor=True,
        wireGain=True,
        twoDCell=True,
        oneDCell=True)

    # Rungain Collector setup
    Collector_RGPre = basf2.register_module('CDCDedxElectronCollector')
    CollParamPre = {'cleanupCuts': True, 'Isrun': True, 'granularity': 'run', }
    Collector_RGPre.param(CollParamPre)

    # Rungain Algorithm setup
    Algorithm_RGPre = CDCDedxRunGainAlgorithm()
    Algorithm_RGPre.setMonitoringPlots(True)

    # Rungain Calibration setup
    Calibration_RGPre = Calibration(
        name="RunGainCalibrationPre",
        algorithms=[Algorithm_RGPre],
        collector=Collector_RGPre,
        input_files=input_files_physics)
    Calibration_RGPre.strategies = SequentialRunByRun
    Calibration_RGPre.pre_collector_path = Calibrate_RGPre
    Calibration_RGPre.algorithms[0].params = {"iov_coverage": output_iov}
    Calibration_RGPre.save_payloads = False

    # ----------2. CosineCorr Gain
    # Cosine Precollector path
    Calibrate_CC = basf2.create_path()
    Calibrate_CC.add_module(
        'CDCDedxCorrection',
        relativeCorrections=False,
        scaleCor=True,
        runGain=True,
        cosineCor=True,
        wireGain=True,
        twoDCell=True,
        oneDCell=True)
    # Cosine Collector setup
    Collector_CC = basf2.register_module('CDCDedxElectronCollector')
    CollParam_CC = {'cleanupCuts': True, 'Ischarge': True, 'Iscosth': True,  'granularity': 'all', }
    Collector_CC.param(CollParam_CC)

    # Cosine Algorithm setup
    Algorithm_CC = CDCDedxCosineAlgorithm()
    Algorithm_CC.setMonitoringPlots(True)

    # Cosine Calibration setup
    Calibration_CC = Calibration(
        name="CosineCorrCalibration",
        algorithms=[Algorithm_CC],
        collector=Collector_CC,
        input_files=input_files_physics)
    Calibration_CC.pre_collector_path = Calibrate_CC
    Calibration_CC.depends_on(Calibration_RGPre)
    Calibration_CC.algorithms[0].params = {"apply_iov": output_iov}

    # ----------3. WireGain Gain
    # WireGain Precollector path
    Calibrate_WG = basf2.create_path()
    Calibrate_WG.add_module(
        'CDCDedxCorrection',
        relativeCorrections=False,
        scaleCor=True,
        runGain=True,
        cosineCor=True,
        wireGain=True,
        twoDCell=True,
        oneDCell=True)

    # WireGain Collector setup
    Collector_WG = basf2.register_module('CDCDedxElectronCollector')
    CollParam_WG = {'cleanupCuts': True, 'Iswire': True, 'Isdedxhit': True, 'granularity': 'all', }
    Collector_WG.param(CollParam_WG)

    # WireGain Algorithm setup
    Algorithm_WG = CDCDedxWireGainAlgorithm()
    Algorithm_WG.setMonitoringPlots(False)

    # WireGain Calibration setup
    Calibration_WG = Calibration(
        name="WireGainCalibration",
        algorithms=[Algorithm_WG],
        collector=Collector_WG,
        input_files=input_files_physics)
    Calibration_WG.pre_collector_path = Calibrate_WG
    Calibration_WG.depends_on(Calibration_RGPre)
    Calibration_WG.depends_on(Calibration_CC)
    Calibration_WG.algorithms[0].params = {"apply_iov": output_iov}

    # ----------4. Final Run Gain to take Wire and Cosine correction in effect
    # Rungain Precollector path
    Calibrate_RG = basf2.create_path()
    Calibrate_RG.add_module(
        'CDCDedxCorrection',
        relativeCorrections=False,
        scaleCor=True,
        runGain=True,
        cosineCor=True,
        wireGain=True,
        twoDCell=True,
        oneDCell=True)

    # Rungain Collector setup
    Collector_RG = basf2.register_module('CDCDedxElectronCollector')
    CollParamFinal = {'cleanupCuts': True, 'Isrun': True, 'granularity': 'run', }
    Collector_RG.param(CollParamFinal)

    # Rungain Algorithm setup
    Algorithm_RG = CDCDedxRunGainAlgorithm()
    Algorithm_RG.setMonitoringPlots(True)

    # Rungain Calibration setup
    Calibration_RG = Calibration(
        name="RunGainCalibration",
        algorithms=[Algorithm_RG],
        collector=Collector_RG,
        input_files=input_files_physics)
    Calibration_RG.strategies = SequentialRunByRun
    Calibration_RG.depends_on(Calibration_RGPre)
    Calibration_RG.depends_on(Calibration_CC)
    Calibration_RG.depends_on(Calibration_WG)
    Calibration_RG.pre_collector_path = Calibrate_RG
    Calibration_RG.algorithms[0].params = {"iov_coverage": output_iov}

    return [Calibration_RGPre, Calibration_CC, Calibration_WG, Calibration_RG]
