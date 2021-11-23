# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Airflow script for automatic CDC dEdx calibration. It is currently for the electron based
calibration, where at present only RunGain, Cosine and WireGain are implimented. The remaining
two 2D and 1D will be implimented in the near future.

Second part called "Hadron calibration" are not compitable with CAF/AirFlow
and will be done offline for a while.
"""

import ROOT
from ROOT import gSystem
from ROOT.Belle2 import CDCDedxRunGainAlgorithm, CDCDedxCosineAlgorithm, CDCDedxWireGainAlgorithm
from caf.framework import Calibration
from caf.strategies import SequentialRunByRun, SequentialBoundaries
from prompt import CalibrationSettings, INPUT_DATA_FILTERS
import reconstruction as recon
from random import seed

gSystem.Load('libreconstruction.so')
ROOT.gROOT.SetBatch(True)

settings = CalibrationSettings(
    name="CDC dedx",
    expert_username="jikumar",
    description=__doc__,
    input_data_formats=["cdst"],
    input_data_names=["bhabha_all_calib"],
    expert_config={
        "payload_boundaries": [],
        "calib_datamode": False,
        "maxevt_rg": 75000,
        "maxevt_cc": 18e6,
        "maxevt_wg": 18e6,
        "adjustment": 1.00798},
    input_data_filters={
        "bhabha_all_calib": [
            INPUT_DATA_FILTERS['Run Type']['physics'],
            INPUT_DATA_FILTERS['Data Tag']['bhabha_all_calib'],
            INPUT_DATA_FILTERS['Data Quality Tag']['Good Or Recoverable'],
            INPUT_DATA_FILTERS['Magnet']['On'],
            INPUT_DATA_FILTERS['Beam Energy']['4S'],
            INPUT_DATA_FILTERS['Beam Energy']['Continuum'],
            INPUT_DATA_FILTERS['Beam Energy']['Scan']]},
    depends_on=[])


def get_calibrations(input_data, **kwargs):
    """ REQUIRED FUNCTION used by b2caf-prompt-run tool
        This function return a list of Calibration
        objects we assign to the CAF process
    """

    import basf2
    file_to_iov_physics = input_data["bhabha_all_calib"]

    expert_config = kwargs.get("expert_config")

    # extracting parameters
    fulldataMode = expert_config["calib_datamode"]
    adjustment = expert_config["adjustment"]

    if fulldataMode:
        input_files_rungain = list(file_to_iov_physics.keys())
        input_files_coscorr = list(file_to_iov_physics.keys())
        input_files_wiregain = list(file_to_iov_physics.keys())
    else:
        seed(271492)

        maxevt_rg = expert_config["maxevt_rg"]
        maxevt_cc = expert_config["maxevt_cc"]
        maxevt_wg = expert_config["maxevt_wg"]

        from prompt.utils import filter_by_max_events_per_run, filter_by_select_max_events_from_files
        # collection for rungains
        max_files_for_maxevents = maxevt_rg  # allevents to accp bhabha event ratio = 0.60
        reduced_file_to_iov_rungain = filter_by_max_events_per_run(file_to_iov_physics, max_files_for_maxevents, True)
        input_files_rungain = list(reduced_file_to_iov_rungain.keys())
        basf2.B2INFO(f"Total number of files used for rungains = {len(input_files_rungain)}")

        # collection for cosinecorr
        input_files_coscorr = filter_by_select_max_events_from_files(list(file_to_iov_physics.keys()), maxevt_cc)
        basf2.B2INFO(f"Total number of files used for cosine = {len(input_files_coscorr)}")
        if not input_files_coscorr:
            raise ValueError(
                f"Cosine: all requested ({maxevt_cc}) events not found")

        # collection for wiregain
        if maxevt_wg == maxevt_cc:
            input_files_wiregain = input_files_coscorr
        else:
            input_files_wiregain = filter_by_select_max_events_from_files(list(file_to_iov_physics.keys()), maxevt_wg)

        basf2.B2INFO(f"Total number of files used for wiregains = {len(input_files_wiregain)}")
        if not input_files_wiregain:
            raise ValueError(
                f"WireGain: all requested ({maxevt_wg}) events not found")

    requested_iov = kwargs.get("requested_iov", None)
    from caf.utils import ExpRun, IoV
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    payload_boundaries = [ExpRun(output_iov.exp_low, output_iov.run_low)]
    payload_boundaries.extend([ExpRun(*boundary) for boundary in expert_config["payload_boundaries"]])
    basf2.B2INFO(f"Expert set payload boundaries are: {expert_config['payload_boundaries']}")

    # ----------1a. Run Gain Pre (No Payload saving and take of effect of previous rungains)
    # Rungain Precollector path
    Calibrate_RGTrial = basf2.create_path()
    recon.prepare_cdst_analysis(path=Calibrate_RGTrial)
    trg_bhabhaskim = Calibrate_RGTrial.add_module("TriggerSkim", triggerLines=["software_trigger_cut&skim&accept_bhabha"])
    trg_bhabhaskim.if_value("==0", basf2.Path(), basf2.AfterConditionPath.END)

    Calibrate_RGTrial.add_module(
        'CDCDedxCorrection',
        relativeCorrections=False,
        scaleCor=True,
        runGain=True,
        cosineCor=True,
        wireGain=True,
        twoDCell=True,
        oneDCell=True)

    # Rungain Collector setup
    Collector_RGTrial = basf2.register_module('CDCDedxElectronCollector')
    CollParamTrial = {'cleanupCuts': True, 'IsRun': True, 'granularity': 'run', }
    Collector_RGTrial.param(CollParamTrial)

    # Rungain Algorithm setup
    Algorithm_RGTrial = CDCDedxRunGainAlgorithm()
    Algorithm_RGTrial.setMonitoringPlots(True)

    # Rungain Calibration setup
    Calibration_RGTrial = Calibration(
        name="RunGainCalibrationTrial",
        algorithms=[Algorithm_RGTrial],
        collector=Collector_RGTrial,
        input_files=input_files_rungain)
    Calibration_RGTrial.strategies = SequentialRunByRun
    Calibration_RGTrial.pre_collector_path = Calibrate_RGTrial
    Calibration_RGTrial.algorithms[0].params = {"iov_coverage": output_iov}
    Calibration_RGTrial.save_payloads = False

    # ----------1b. Run Gain Pre (No Payload saving)
    # Rungain Precollector path
    Calibrate_RGPre = basf2.create_path()
    recon.prepare_cdst_analysis(path=Calibrate_RGPre)
    trg_bhabhaskim = Calibrate_RGPre.add_module("TriggerSkim", triggerLines=["software_trigger_cut&skim&accept_bhabha"])
    trg_bhabhaskim.if_value("==0", basf2.Path(), basf2.AfterConditionPath.END)
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
    CollParamPre = {'cleanupCuts': True, 'IsRun': True, 'granularity': 'run'}
    Collector_RGPre.param(CollParamPre)

    # Rungain Algorithm setup
    Algorithm_RGPre = CDCDedxRunGainAlgorithm()
    Algorithm_RGPre.setMonitoringPlots(True)

    # Rungain Calibration setup
    Calibration_RGPre = Calibration(
        name="RunGainCalibrationPre",
        algorithms=[Algorithm_RGPre],
        collector=Collector_RGPre,
        input_files=input_files_rungain)
    Calibration_RGPre.strategies = SequentialRunByRun
    Calibration_RGPre.pre_collector_path = Calibrate_RGPre
    Calibration_RGPre.depends_on(Calibration_RGTrial)
    Calibration_RGPre.algorithms[0].params = {"iov_coverage": output_iov}
    Calibration_RGPre.save_payloads = False

    # ----------2. CosineCorr Gain
    # Cosine Precollector path
    Calibrate_CC = basf2.create_path()
    recon.prepare_cdst_analysis(path=Calibrate_CC)
    trg_bhabhaskim = Calibrate_CC.add_module("TriggerSkim", triggerLines=["software_trigger_cut&skim&accept_bhabha"])
    trg_bhabhaskim.if_value("==0", basf2.Path(), basf2.AfterConditionPath.END)

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
    CollParam_CC = {'cleanupCuts': True, 'IsCharge': True, 'IsCosth': True, 'granularity': 'all'}
    Collector_CC.param(CollParam_CC)
    if expert_config["payload_boundaries"] is not None:
        Collector_CC.param("granularity", "run")

    # Cosine Algorithm setup
    Algorithm_CC = CDCDedxCosineAlgorithm()
    Algorithm_CC.setMonitoringPlots(True)

    # Cosine Calibration setup
    Calibration_CC = Calibration(
        name="CosineCorrCalibration",
        algorithms=[Algorithm_CC],
        collector=Collector_CC,
        input_files=input_files_coscorr)
    Calibration_CC.pre_collector_path = Calibrate_CC
    Calibration_CC.depends_on(Calibration_RGPre)
    if expert_config["payload_boundaries"] is not None:
        Calibration_CC.strategies = SequentialBoundaries
        basf2.B2INFO("Calibration_CC: Found payload_boundaries: calibration strategies set to SequentialBoundaries.")
        Calibration_CC.algorithms[0].params = {"iov_coverage": output_iov, "payload_boundaries": payload_boundaries}
    else:
        Calibration_CC.algorithms[0].params = {"apply_iov": output_iov}

    # ----------3. WireGain Gain
    # WireGain Precollector path
    Calibrate_WG = basf2.create_path()
    recon.prepare_cdst_analysis(path=Calibrate_WG)
    trg_bhabhaskim = Calibrate_WG.add_module("TriggerSkim", triggerLines=["software_trigger_cut&skim&accept_bhabha"])
    trg_bhabhaskim.if_value("==0", basf2.Path(), basf2.AfterConditionPath.END)

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
    CollParam_WG = {'cleanupCuts': True, 'IsWire': True, 'Isdedxhit': True, 'granularity': 'all'}
    Collector_WG.param(CollParam_WG)
    if expert_config["payload_boundaries"] is not None:
        Collector_WG.param("granularity", "run")

    # WireGain Algorithm setup
    Algorithm_WG = CDCDedxWireGainAlgorithm()
    Algorithm_WG.setMonitoringPlots(True)

    # WireGain Calibration setup
    Calibration_WG = Calibration(
        name="WireGainCalibration",
        algorithms=[Algorithm_WG],
        collector=Collector_WG,
        input_files=input_files_wiregain)
    Calibration_WG.pre_collector_path = Calibrate_WG
    Calibration_WG.depends_on(Calibration_RGPre)
    Calibration_WG.depends_on(Calibration_CC)
    if expert_config["payload_boundaries"] is not None:
        Calibration_WG.strategies = SequentialBoundaries
        basf2.B2INFO("Calibration_WG: Found payload_boundaries: calibration strategies set to SequentialBoundaries.")
        Calibration_WG.algorithms[0].params = {"iov_coverage": output_iov, "payload_boundaries": payload_boundaries}
    else:
        Calibration_WG.algorithms[0].params = {"apply_iov": output_iov}

    # ----------4. Final Run Gain to take Wire and Cosine correction in effect
    # Rungain Precollector path
    Calibrate_RG = basf2.create_path()
    recon.prepare_cdst_analysis(path=Calibrate_RG)
    trg_bhabhaskim = Calibrate_RG.add_module("TriggerSkim", triggerLines=["software_trigger_cut&skim&accept_bhabha"])
    trg_bhabhaskim.if_value("==0", basf2.Path(), basf2.AfterConditionPath.END)

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
    CollParamFinal = {'cleanupCuts': True, 'IsRun': True, 'granularity': 'run', }
    Collector_RG.param(CollParamFinal)

    # Rungain Algorithm setup
    Algorithm_RG = CDCDedxRunGainAlgorithm()
    Algorithm_RG.setMonitoringPlots(True)
    Algorithm_RG.setAdjustment(adjustment)

    # Rungain Calibration setup
    Calibration_RG = Calibration(
        name="RunGainCalibration",
        algorithms=[Algorithm_RG],
        collector=Collector_RG,
        input_files=input_files_rungain)
    Calibration_RG.strategies = SequentialRunByRun
    Calibration_RG.depends_on(Calibration_RGPre)
    Calibration_RG.depends_on(Calibration_CC)
    Calibration_RG.depends_on(Calibration_WG)
    Calibration_RG.pre_collector_path = Calibrate_RG
    Calibration_RG.algorithms[0].params = {"iov_coverage": output_iov}

    return [Calibration_RGTrial, Calibration_RGPre, Calibration_CC, Calibration_WG, Calibration_RG]
