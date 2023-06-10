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
calibration, where at present only RunGain, injection time, Cosine and WireGain are implimented.
The remaining two 2D and 1D will be implimented in the near future.

Second part called "Hadron calibration" are not compitable with CAF/AirFlow
and will be done offline for a while.
"""

import ROOT
from ROOT import gSystem
from ROOT.Belle2 import CDCDedxRunGainAlgorithm, CDCDedxCosineAlgorithm, CDCDedxWireGainAlgorithm
from ROOT.Belle2 import CDCDedxCosEdgeAlgorithm, CDCDedxBadWireAlgorithm, CDCDedxInjectTimeAlgorithm
from caf.framework import Calibration
from caf.strategies import SequentialRunByRun, SequentialBoundaries
from prompt import CalibrationSettings, INPUT_DATA_FILTERS
import reconstruction as recon
from random import seed
import basf2

gSystem.Load('libreconstruction.so')
ROOT.gROOT.SetBatch(True)

settings = CalibrationSettings(
    name="CDC dedx",
    expert_username="renu",
    description=__doc__,
    input_data_formats=["cdst"],
    input_data_names=["bhabha_all_calib"],
    expert_config={
        "payload_boundaries": [],
        "calib_datamode": False,
        "maxevt_rg": 75000,
        "maxevt_cc": 18e6,
        "maxevt_wg": 18e6,
        "calib_mode": "full",  # manual or predefined: quick or full
        "calibration_procedure": {"rgtrail0": 0, "rgpre0": 0, "rg0": 0}
         },
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
    calib_mode = expert_config["calib_mode"]
    # extracting parameters
    fulldataMode = expert_config["calib_datamode"]

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

    collector_granularity = 'all'
    if expert_config["payload_boundaries"] is not None:
        basf2.B2INFO('Found payload_boundaries: set collector granularity to run')
        collector_granularity = 'run'

    if calib_mode == "full":
        calibration_procedure = {
            "rgtrail0": 0,
            "tg0": 0,
            "cc0": 0,
            "ce0": 0,
            "bd0": 0,
            "wg0": 0,
            "rgpre0": 0,
            "rg0": 0
        }
    elif calib_mode == "quick":
        calibration_procedure = {
            "rgtrail0": 0,
            "tg0": 0,
            "cc0": 0,
            "wg0": 0,
            "rgpre0": 0,
            "rg0": 0
        }
    elif calib_mode == "manual":
        calibration_procedure = expert_config["calibration_procedure"]
    else:
        basf2.B2FATAL(f"Calibration mode is not defined {calib_mode}, should be quick, full, or manual")

    calib_keys = list(calibration_procedure)
    cals = [None]*len(calib_keys)
    basf2.B2INFO(f"Run calibration mode = {calib_mode}:")
    print(calib_keys)

    for i in range(len(cals)):
        max_iter = calibration_procedure[calib_keys[i]]
        alg = None
        data_files = [input_files_rungain, input_files_coscorr, input_files_wiregain]
        cal_name = ''.join([i for i in calib_keys[i] if not i.isdigit()])
        if cal_name == "rg" or cal_name == "rgtrail" or cal_name == "rgpre":
            alg = [rg_algo()]
        elif cal_name == "cc":
            alg = [cos_algo()]
        elif cal_name == "ce":
            alg = [cosedge_algo()]
        elif cal_name == "tg":
            alg = [time_algo()]
        elif cal_name == "bd":
            alg = [badwire_algo()]
        elif cal_name == "wg":
            alg = [wg_algo()]
        else:
            basf2.B2FATAL(f"The calibration is not defined, check spelling: calib {i}: {calib_keys[i]}")

        basf2.B2INFO(f"calibration for {calib_keys[i]} with number of iteration={max_iter}")

        cals[i] = CDCDedxCalibration(name=cal_name,
                                     algorithms=alg,
                                     input_file_dict=data_files,
                                     max_iterations=max_iter,
                                     collector_granularity=collector_granularity,
                                     dependencies=[cals[i-1]] if i > 0 else None
                                     )
        if payload_boundaries:
            basf2.B2INFO("Found payload_boundaries: calibration strategies set to SequentialBoundaries.")
            if cal_name == "rg" or cal_name == "rgtrail" or cal_name == "rgpre" or cal_name == "tg":
                cals[i].strategies = SequentialRunByRun
                for algorithm in cals[i].algorithms:
                    algorithm.params = {"iov_coverage": output_iov}
                if cal_name == "rgtrail" or cal_name == "rgpre":
                    cals[i].save_payloads = False
            else:
                cals[i].strategies = SequentialBoundaries
                for algorithm in cals[i].algorithms:
                    algorithm.params = {"iov_coverage": output_iov, "payload_boundaries": payload_boundaries}

        else:
            for algorithm in cals[i].algorithms:
                algorithm.params = {"apply_iov": output_iov}

    return cals


# Precollector path
def pre_collector(name='rg'):
    """
    Define pre collection.
    Parameters:
        name : name of the calibration
                           rungain rg0 by Default.
    Returns:
        path : path for pre collection
    """

    reco_path = basf2.create_path()
    recon.prepare_cdst_analysis(path=reco_path)
    if(name == "tg"):
        trg_bhabhaskim = reco_path.add_module("TriggerSkim", triggerLines=["software_trigger_cut&skim&accept_radee"])
        trg_bhabhaskim.if_value("==0", basf2.Path(), basf2.AfterConditionPath.END)
        # ps_bhabhaskim = reco_path.add_module("Prescale", prescale=0.80)
        # ps_bhabhaskim.if_value("==0", basf2.Path(), basf2.AfterConditionPath.END)

    elif (name == "ce"):
        trg_bhabhaskim = reco_path.add_module(
            "TriggerSkim",
            triggerLines=[
                "software_trigger_cut&skim&accept_bhabha",
                "software_trigger_cut&filter&ee_flat_90_180",
                "software_trigger_cut&filter&ee_flat_0_19"])
        trg_bhabhaskim.if_value("==0", basf2.Path(), basf2.AfterConditionPath.END)
    else:
        trg_bhabhaskim = reco_path.add_module("TriggerSkim", triggerLines=["software_trigger_cut&skim&accept_bhabha"])
        trg_bhabhaskim.if_value("==0", basf2.Path(), basf2.AfterConditionPath.END)

    reco_path.add_module(
        'CDCDedxCorrection',
        relativeCorrections=False,
        scaleCor=True,
        runGain=True,
        timeGain=True,
        cosineCor=True,
        wireGain=True,
        twoDCell=True,
        oneDCell=True)
    return reco_path

# Collector setup


def collector(granularity='all', name=''):
    """
    Create a cdcdedx calibration collector
    Parameters:
        name : name of calibration
        granularity : granularity : all or run
    Returns:
        collector : collector module
    """

    from basf2 import register_module
    col = register_module('CDCDedxElectronCollector', cleanupCuts=True)
    if name == "tg":
        CollParam = {'isRun': True, 'isInjTime': True, 'granularity': 'run'}

    elif name == "cc" or name == "ce":
        CollParam = {'isCharge': True, 'isCosth': True, 'granularity': granularity}

    elif name == "bd":
        isHit = True
        CollParam = {'isWire': True, 'isDedxhit': isHit, 'isADCcorr': not isHit, 'granularity': granularity}

    elif name == "wg":
        CollParam = {'isWire': True, 'isDedxhit': True, 'granularity': granularity}

    else:
        CollParam = {'isRun': True, 'granularity': 'run'}

    col.param(CollParam)
    return col

# Rungain Algorithm setup


def rg_algo():
    """
    Create a rungain calibration algorithm.
    Returns:
        algo : rungain algorithm
    """
    algo = CDCDedxRunGainAlgorithm()
    algo.setMonitoringPlots(True)
    return algo

# Injection Algorithm setup


def time_algo():
    """
    Create a injection time calibration algorithm.
    Returns:
        algo : injection time algorithm
    """
    algo = CDCDedxInjectTimeAlgorithm()
    algo.setMonitoringPlots(True)
    return algo

# Cosine Algorithm setup


def cos_algo():
    """
    Create a cosine calibration algorithm.
    Returns:
        algo : cosine algorithm
    """
    algo = CDCDedxCosineAlgorithm()
    algo.setMonitoringPlots(True)
    return algo

# CosineEdge Algorithm setup


def cosedge_algo():
    """
    Create a cosine edge calibration algorithm.
    Returns:
        algo : cosine edge algorithm
    """
    algo = CDCDedxCosEdgeAlgorithm()
    algo.setMonitoringPlots(True)
    return algo

# Badwire Algorithm setup


def badwire_algo():
    """
    Create a badwire calibration algorithm.
    Returns:
        algo : badwire algorithm
    """
    algo = CDCDedxBadWireAlgorithm()
    # threshold (mean and rms) pars for dedx
    algo.setHighFracThres(0.2)
    algo.setMeanThres(0.4)
    algo.setRMSThres(0.4)
    algo.setHistPars(150, 0, 5)
    algo.setMonitoringPlots(True)
    return algo

# WireGain Algorithm setup


def wg_algo():
    """
    Create a wire gain calibration algorithm.
    Returns:
        algo : wiregain algorithm
    """
    algo = CDCDedxWireGainAlgorithm()
    algo.enableExtraPlots(True)
    return algo


class CDCDedxCalibration(Calibration):
    '''
    CDCDedxCalibration is a specialized calibration for cdcdedx.
    '''

    def __init__(self,
                 name,
                 algorithms,
                 input_file_dict,
                 max_iterations=5,
                 dependencies=None,
                 collector_granularity='All'):
        '''
        parameters:
            name: name of calibration
            algorithims: algorithm of calibration
            input_file_dict: input files list
            max_iterations: maximum number of iterations
            dependenices: depends on the previous calibration
            collector_granularity: granularity : all or run
        '''
        super().__init__(name=name,
                         algorithms=algorithms
                         )

        from caf.framework import Collection

        if name == "bd" or name == "wg":
            collection = Collection(collector=collector(granularity=collector_granularity, name=name),
                                    input_files=input_file_dict[2],
                                    pre_collector_path=pre_collector(name)
                                    )
        elif name == "cc" or name == "ce":
            collection = Collection(collector=collector(granularity=collector_granularity, name=name),
                                    input_files=input_file_dict[1],
                                    pre_collector_path=pre_collector(name)
                                    )
        else:
            collection = Collection(collector=collector(granularity=collector_granularity, name=name),
                                    input_files=input_file_dict[0],
                                    pre_collector_path=pre_collector(name)
                                    )
        self.add_collection(name=name, collection=collection)

        #: maximum iterations
        self.max_iterations = max_iterations

        if dependencies is not None:
            for dep in dependencies:
                self.depends_on(dep)
