# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Airflow script to perform eCMS calibration.
"""

from prompt import CalibrationSettings, input_data_filters
from prompt.calibrations.caf_boostvector import settings as boostvector


#: Tells the automated system some details of this script
settings = CalibrationSettings(
    name="eCMS Calibrations",
    expert_username="zlebcr",
    description=__doc__,
    input_data_formats=["cdst"],
    input_data_names=["hadron_calib"],
    input_data_filters={
        "hadron_calib": [
            input_data_filters["Data Tag"]["hadron_calib"],
            input_data_filters["Run Type"]["physics"],
            input_data_filters["Data Quality Tag"]["Good Or Recoverable"],
            input_data_filters["Magnet"]["On"]]},
    expert_config={
        "outerLoss": "pow(rawTime - 8.0, 2) + 10 * pow(maxGap, 2)",
        "innerLoss": "pow(rawTime - 8.0, 2) + 10 * pow(maxGap, 2)"},
    depends_on=[boostvector])

##############################


def get_calibrations(input_data, **kwargs):
    """
    Parameters:
      input_data (dict): Should contain every name from the 'input_data_names' variable as a key.
        Each value is a dictionary with {"/path/to/file_e1_r5.root": IoV(1,5,1,5), ...}. Useful for
        assigning to calibration.files_to_iov

      **kwargs: Configuration options to be sent in. Since this may change we use kwargs as a way to help prevent
        backwards compatibility problems. But you could use the correct arguments in b2caf-prompt-run for this
        release explicitly if you want to.

        Currently only kwargs["output_iov"] is used. This is the output IoV range that your payloads should
        correspond to. Generally your highest ExpRun payload should be open ended e.g. IoV(3,4,-1,-1)

    Returns:
      list(caf.framework.Calibration): All of the calibration objects we want to assign to the CAF process
    """
    import basf2
    import stdCharged
    import stdPi0s

    # Set up config options

    # In this script we want to use one sources of input data.
    # Get the input files  from the input_data variable
    file_to_iov_physics = input_data["hadron_calib"]

    # We might have requested an enormous amount of data across a run range.
    # There's a LOT more files than runs!
    # Lets set some limits because this calibration doesn't need that much to run.
    max_files_per_run = 1000000

    # We filter out any more than 100 files per run. The input data files are sorted alphabetically by b2caf-prompt-run
    # already. This procedure respects that ordering
    from prompt.utils import filter_by_max_files_per_run

    reduced_file_to_iov_physics = filter_by_max_files_per_run(file_to_iov_physics, max_files_per_run)
    input_files_physics = list(reduced_file_to_iov_physics.keys())
    basf2.B2INFO(f"Total number of files actually used as input = {len(input_files_physics)}")

    # Get the overall IoV we our process should cover. Includes the end values that we may want to ignore since our output
    # IoV should be open ended. We could also use this as part of the input data selection in some way.
    requested_iov = kwargs.get("requested_iov", None)

    from caf.utils import IoV
    # The actual value our output IoV payload should have. Notice that we've set it open ended.
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    ###################################################
    # Algorithm setup

    from ROOT.Belle2 import InvariantMassAlgorithm
    from basf2 import create_path, register_module
    import modularAnalysis as ma
    import vertex

    ###################################################
    # Calibration setup

    from caf.framework import Calibration
    from caf.strategies import SingleIOV
    from reconstruction import add_pid_module

    # module to be run prior the collector
    rec_path_1 = create_path()
    add_pid_module(rec_path_1)

    # vertex.treeFit('Upsilon(4S):BV', updateAllDaughters=True, ipConstraint=True, path=rec_path_1)

    stdCharged.stdPi(listtype='loose', path=rec_path_1)
    stdCharged.stdK(listtype='good', path=rec_path_1)
    stdPi0s.stdPi0s(listtype='eff30_May2020', path=rec_path_1)

    ma.cutAndCopyList("pi+:my", "pi+:loose", "[abs(dz)<2.0] and [abs(dr)<0.5]", path=rec_path_1)
    ma.cutAndCopyList("K+:my", "K+:good", "[abs(dz)<2.0] and [abs(dr)<0.5]", path=rec_path_1)

    #####################################################
    # Reconstructs the signal B0 candidates from Dstar
    #####################################################

    # Reconstructs D0s and sets decay mode identifiers
    ma.reconstructDecay(decayString='D0:Kpi -> K-:my pi+:my', cut='1.7 < M < 2.1', dmID=1, path=rec_path_1)
    ma.reconstructDecay(decayString='D0:Kpipi0 -> K-:my pi+:my pi0:eff30_May2020',
                        cut='1.7 < M < 2.1', dmID=2, path=rec_path_1)
    ma.reconstructDecay(decayString='D0:Kpipipi -> K-:my pi+:my pi-:my pi+:my',
                        cut='1.7 < M < 2.1', dmID=3, path=rec_path_1)

    # Performs mass constrained fit for all D0 candidates
    # vertex.kFit(list_name='D0:Kpi',     conf_level=0.0, fit_type='mass', path=rec_path_1)
    # vertex.kFit(list_name='D0:Kpipi0',  conf_level=0.0, fit_type='mass', path=rec_path_1)
    # vertex.kFit(list_name='D0:Kpipipi', conf_level=0.0, fit_type='mass', path=rec_path_1)

    # Reconstructs D*-s and sets decay mode identifiers
    ma.reconstructDecay(decayString='D*+:D0pi_Kpi -> D0:Kpi pi+:my', cut='massDifference(0) < 0.16', dmID=1, path=rec_path_1)
    ma.reconstructDecay(decayString='D*+:D0pi_Kpipi0 -> D0:Kpipi0 pi+:my',
                        cut='massDifference(0) < 0.16', dmID=2, path=rec_path_1)
    ma.reconstructDecay(decayString='D*+:D0pi_Kpipipi -> D0:Kpipipi pi+:my',
                        cut='massDifference(0) < 0.16', dmID=3, path=rec_path_1)

    Bcut = '[5.2 < Mbc < 5.3] and [abs(deltaE) < 0.2]'

    # Reconstructs the signal B0 candidates from Dstar
    ma.reconstructDecay(decayString='B0:Dstpi_D0pi_Kpi -> D*-:D0pi_Kpi pi+:my',
                        cut=Bcut,
                        dmID=1, path=rec_path_1)
    ma.reconstructDecay(decayString='B0:Dstpi_D0pi_Kpipi0 -> D*-:D0pi_Kpipi0 pi+:my',
                        cut=Bcut,
                        dmID=2, path=rec_path_1)
    ma.reconstructDecay(decayString='B0:Dstpi_D0pi_Kpipipi -> D*-:D0pi_Kpipipi pi+:my',
                        cut=Bcut,
                        dmID=3, path=rec_path_1)

    #####################################################
    # Reconstructs the signal B0 candidates from D-
    #####################################################

    # Reconstructs charged D mesons and sets decay mode identifiers
    ma.reconstructDecay(decayString='D-:Kpipi -> K+:my pi-:my pi-:my',
                        cut='1.844 < M < 1.894', dmID=4, path=rec_path_1)

    # vx.massKFit(list_name='D-:Kpipi', conf_level=0.0, path=rec_path_1)

    # Reconstructs the signal B candidates
    ma.reconstructDecay(decayString='B0:Dpi_Kpipi -> D-:Kpipi pi+:my',
                        cut=Bcut, dmID=4, path=rec_path_1)

    #####################################################
    # Reconstruct the signal B- candidates
    #####################################################

    # Reconstructs D0s and sets decay mode identifiers
    # ma.reconstructDecay(decayString='D0:Kpi -> K-:good pi+:my', cut='1.84 < M < 1.89', dmID=1, path=main_path)
    # ma.reconstructDecay(decayString='D0:Kpipi0 -> K-:good pi+:my pi0:looseFit', cut='1.84 < M < 1.89', dmID=2, path=main_path)
    # ma.reconstructDecay(decayString='D0:Kpipipi -> K-:good pi+:my pi-:my pi+:my', cut='1.84 < M < 1.89', dmID=3, path=main_path)

    # Performs mass constrained fit for all D0 candidates
    # vx.massKFit(list_name='D0:Kpi', conf_level=0.0, path=main_path)
    # vx.massKFit(list_name='D0:Kpipi0', conf_level=0.0, path=main_path)
    # vx.massKFit(list_name='D0:Kpipipi', conf_level=0.0, path=main_path)

    # Reconstructs the signal B- candidates
    ma.reconstructDecay(decayString='B-:D0pi_Kpi -> D0:Kpi pi-:my',
                        cut=Bcut,
                        dmID=5, path=rec_path_1)
    ma.reconstructDecay(decayString='B-:D0pi_Kpipi0 -> D0:Kpipi0 pi-:my',
                        cut=Bcut,
                        dmID=6, path=rec_path_1)
    ma.reconstructDecay(decayString='B-:D0pi_Kpipipi -> D0:Kpipipi pi-:my',
                        cut=Bcut,
                        dmID=7, path=rec_path_1)

    ma.copyLists(
        outputListName='B0:merged',
        inputListNames=[
            'B0:Dstpi_D0pi_Kpi',
            'B0:Dstpi_D0pi_Kpipi0',
            'B0:Dstpi_D0pi_Kpipipi',
            'B0:Dpi_Kpipi'
        ],
        path=rec_path_1)

    ma.copyLists(
        outputListName='B-:merged',
        inputListNames=[
            'B-:D0pi_Kpi',
            'B-:D0pi_Kpipi0',
            'B-:D0pi_Kpipipi',
        ],
        path=rec_path_1)

    # Builds the rest of event object, which contains all particles not used in the reconstruction of B0 candidates.
    ma.buildRestOfEvent(target_list_name='B0:merged', path=rec_path_1)

    # Calculates the continuum suppression variables
    cleanMask = ('cleanMask', 'nCDCHits > 0 and useCMSFrame(p)<=3.2', 'p >= 0.05 and useCMSFrame(p)<=3.2')
    ma.appendROEMasks(list_name='B0:merged', mask_tuples=[cleanMask], path=rec_path_1)
    ma.buildContinuumSuppression(list_name='B0:merged', roe_mask='cleanMask', path=rec_path_1)

    # Builds the rest of event object, which contains all particles not used in the reconstruction of B- candidates.
    ma.buildRestOfEvent(target_list_name='B-:merged', path=rec_path_1)

    # Calculates the continuum suppression variables
    cleanMask = ('cleanMask', 'nCDCHits > 0 and useCMSFrame(p)<=3.2', 'p >= 0.05 and useCMSFrame(p)<=3.2')
    ma.appendROEMasks(list_name='B-:merged', mask_tuples=[cleanMask], path=rec_path_1)
    ma.buildContinuumSuppression(list_name='B-:merged', roe_mask='cleanMask', path=rec_path_1)

    ma.applyCuts("B0:merged", "R2 < 0.9", path=rec_path_1)
    ma.applyCuts("B-:merged", "R2 < 0.9", path=rec_path_1)

    collector_ecms = register_module('eCmsCollector')
    algorithm_ecms = InvariantMassAlgorithm()
    algorithm_ecms.setOuterLoss(kwargs['expert_config']['outerLoss'])
    algorithm_ecms.setInnerLoss(kwargs['expert_config']['innerLoss'])

    calibration_ecms = Calibration('eCMS',
                                   collector=collector_ecms,
                                   algorithms=algorithm_ecms,
                                   input_files=input_files_physics,
                                   pre_collector_path=rec_path_1)

    calibration_ecms.strategies = SingleIOV

    # Do this for the default AlgorithmStrategy to force the output payload IoV
    # It may be different if you are using another strategy like SequentialRunByRun
    for algorithm in calibration_ecms.algorithms:
        algorithm.params = {"iov_coverage": output_iov}

    # Most other options like database chain and backend args will be overwritten by b2caf-prompt-run.
    # So we don't bother setting them.

    # You must return all calibrations you want to run in the prompt process, even if it's only one
    return [calibration_ecms]

##############################
