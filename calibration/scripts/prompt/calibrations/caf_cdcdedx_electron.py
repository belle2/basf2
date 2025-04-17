##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Airflow script for automatic CDC dEdx calibration. It is the electron based
calibration, where at present only RunGain, injection time, Cosine, WireGain and 1D are implemented.
The remaining two 2D will be implemented in the near future.
"""

import ROOT
from ROOT import gSystem
from ROOT.Belle2 import CDCDedxRunGainAlgorithm, CDCDedxCosineAlgorithm, CDCDedxWireGainAlgorithm
from ROOT.Belle2 import CDCDedxCosEdgeAlgorithm, CDCDedxBadWireAlgorithm, CDCDedxInjectTimeAlgorithm
from ROOT.Belle2 import CDCDedx1DCellAlgorithm

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
        "adjustment": 1.00798,
        "calib_mode": "full",  # manual or predefined: full or quick
        "calibration_procedure": {"rungain0": 0, "rungain1": 0, "rungain2": 0}
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

    collector_granularity = 'all'
    if expert_config["payload_boundaries"] is not None:
        basf2.B2INFO('Found payload_boundaries: set collector granularity to run')
        collector_granularity = 'run'

    if calib_mode == "full":
        calibration_procedure = {
            "rungain0": 0,  # Run Gain trail (No Payload saving and take of effect of previous rungains)
            "timegain0": 0,  # Injection time gain Pre (No payload saving)
            "timegain1": 0,  # Injection time gain
            "rungain1": 0,  # Run Gain Pre (No Payload saving)
            "coscorr0": 0,  # Cosine Corr Gain Pre (No Payload saving)
            "cosedge0": 0,  # Cosine edge Corr Gain
            "badwire0": 0,  # Bad wire
            "wiregain0": 0,  # WireGain Gain
            "onedcell0": 0,  # OneD cell correction
            "coscorr1": 0,  # Cosine Corr Gain
            "rungain2": 0  # Final Run Gain to take Wire and Cosine correction in effect
        }
    elif calib_mode == "quick":
        calibration_procedure = {
            "rungain0": 0,
            "timegain0": 0,
            "rungain1": 0,
            "coscorr0": 0,
            "cosedge0": 0,
            "badwire0": 0,
            "wiregain0": 0,
            "rungain2": 0
        }
    elif calib_mode == "manual":
        calibration_procedure = expert_config["calibration_procedure"]
    else:
        basf2.B2FATAL(f"Calibration mode is not defined {calib_mode}, should be full, quick, or manual")

    calib_keys = list(calibration_procedure)
    cals = [None]*len(calib_keys)
    basf2.B2INFO(f"Run calibration mode = {calib_mode}:")

    for i in range(len(cals)):
        max_iter = calibration_procedure[calib_keys[i]]
        alg = None
        data_files = [input_files_rungain, input_files_coscorr, input_files_wiregain]
        cal_name = ''.join([i for i in calib_keys[i] if not i.isdigit()])
        if cal_name == "rungain":
            alg = [rungain_algo(calib_keys[i], adjustment)]
        elif cal_name == "coscorr":
            alg = [cos_algo()]
        elif cal_name == "cosedge":
            alg = [cosedge_algo()]
        elif cal_name == "timegain":
            alg = [injection_time_algo()]
        elif cal_name == "badwire":
            alg = [badwire_algo()]
        elif cal_name == "wiregain":
            alg = [wiregain_algo()]
        elif cal_name == "onedcell":
            alg = [onedcell_algo()]
        else:
            basf2.B2FATAL(f"The calibration is not defined, check spelling: calib {i}: {calib_keys[i]}")

        basf2.B2INFO(f"calibration for {calib_keys[i]} with number of iteration={max_iter}")

        cals[i] = CDCDedxCalibration(name=calib_keys[i],
                                     algorithms=alg,
                                     input_file_dict=data_files,
                                     max_iterations=max_iter,
                                     collector_granularity=collector_granularity,
                                     dependencies=[cals[i-1]] if i > 0 else None
                                     )
        if payload_boundaries:
            basf2.B2INFO("Found payload_boundaries: calibration strategies set to SequentialBoundaries.")
            if cal_name == "rungain" or cal_name == "timegain":
                cals[i].strategies = SequentialRunByRun
                for algorithm in cals[i].algorithms:
                    algorithm.params = {"iov_coverage": output_iov}
                if calib_keys[i] == "rungain0" or calib_keys[i] == "rungain1" or calib_keys[i] == "timegain0":
                    cals[i].save_payloads = False
            else:
                cals[i].strategies = SequentialBoundaries
                for algorithm in cals[i].algorithms:
                    algorithm.params = {"iov_coverage": output_iov, "payload_boundaries": payload_boundaries}
                if calib_keys[i] == "coscorr0":
                    cals[i].save_payloads = False

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
                           rungain rungain0 by Default.
    Returns:
        path : path for pre collection
    """

    reco_path = basf2.create_path()
    recon.prepare_cdst_analysis(path=reco_path)
    if (name == "timegain" or name == "onedcell"):
        trg_bhabhaskim = reco_path.add_module("TriggerSkim", triggerLines=["software_trigger_cut&skim&accept_radee"])
        trg_bhabhaskim.if_value("==0", basf2.Path(), basf2.AfterConditionPath.END)
        ps_bhabhaskim = reco_path.add_module("Prescale", prescale=0.80)
        ps_bhabhaskim.if_value("==0", basf2.Path(), basf2.AfterConditionPath.END)

    elif (name == "cosedge"):
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
    if name == "timegain":
        CollParam = {'isRun': True, 'isInjTime': True, 'granularity': 'run'}

    elif name == "coscorr" or name == "cosedge":
        CollParam = {'isCharge': True, 'isCosth': True, 'granularity': granularity}

    elif name == "badwire":
        isHit = True
        CollParam = {'isWire': True, 'isDedxhit': isHit, 'isADCcorr': not isHit, 'granularity': granularity}

    elif name == "wiregain":
        CollParam = {'isWire': True, 'isDedxhit': True, 'granularity': granularity}

    elif name == "onedcell":
        CollParam = {
            'isPt': True,
            'isCosth': True,
            'isLayer': True,
            'isDedxhit': True,
            'isEntaRS': True,
            'granularity': granularity}

    else:
        CollParam = {'isRun': True, 'granularity': 'run'}

    col.param(CollParam)
    return col

# Rungain Algorithm setup


def rungain_algo(name, adjustment):
    """
    Create a rungain calibration algorithm.
    Returns:
        algo : rungain algorithm
    """
    algo = CDCDedxRunGainAlgorithm()
    algo.setMonitoringPlots(True)
    if name == "rungain2":
        algo.setAdjustment(adjustment)
    return algo

# Injection Algorithm setup


def injection_time_algo():
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


def wiregain_algo():
    """
    Create a wire gain calibration algorithm.
    Returns:
        algo : wiregain algorithm
    """
    algo = CDCDedxWireGainAlgorithm()
    algo.enableExtraPlots(True)
    return algo


def onedcell_algo():
    """
    Create oned cell calibration algorithm.
    Returns:
        algo : oned cell correction algorithm
    """
    algo = CDCDedx1DCellAlgorithm()
    algo.enableExtraPlots(True)
    algo.setMergePayload(True)
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
            algorithms: algorithm of calibration
            input_file_dict: input files list
            max_iterations: maximum number of iterations
            dependencies: depends on the previous calibration
            collector_granularity: granularity : all or run
        '''
        super().__init__(name=name,
                         algorithms=algorithms
                         )

        from caf.framework import Collection
        cal_name = ''.join([i for i in name if not i.isdigit()])
        if cal_name == "badwire" or cal_name == "wiregain":
            collection = Collection(collector=collector(granularity=collector_granularity, name=cal_name),
                                    input_files=input_file_dict[2],
                                    pre_collector_path=pre_collector(cal_name)
                                    )
        elif cal_name == "coscorr" or cal_name == "cosedge" or cal_name == "onedcell":
            collection = Collection(collector=collector(granularity=collector_granularity, name=cal_name),
                                    input_files=input_file_dict[1],
                                    pre_collector_path=pre_collector(cal_name)
                                    )
        else:
            collection = Collection(collector=collector(granularity=collector_granularity, name=cal_name),
                                    input_files=input_file_dict[0],
                                    pre_collector_path=pre_collector(cal_name)
                                    )
        self.add_collection(name=cal_name, collection=collection)

        #: maximum iterations
        self.max_iterations = max_iterations

        if dependencies is not None:
            for dep in dependencies:
                self.depends_on(dep)
