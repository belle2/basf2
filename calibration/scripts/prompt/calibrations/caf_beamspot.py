# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Airflow script to perform BeamSpot calibration.
"""

from prompt import CalibrationSettings, INPUT_DATA_FILTERS

#: Tells the automated system some details of this script
settings = CalibrationSettings(
    name="BeamSpot Calibrations",
    expert_username="zlebcr",
    description=__doc__,
    input_data_formats=["cdst"],
    input_data_names=["mumutight_or_highm_calib"],
    input_data_filters={
        "mumutight_or_highm_calib": [
            INPUT_DATA_FILTERS["Data Tag"]["mumutight_or_highm_calib"],
            INPUT_DATA_FILTERS["Run Type"]["physics"],
            INPUT_DATA_FILTERS["Data Quality Tag"]["Good Or Recoverable"],
            INPUT_DATA_FILTERS["Magnet"]["On"]]},
    expert_config={
        "outerLoss": "pow(rawTime - 2.0, 2) + 10 * pow(maxGap, 2)",
        "innerLoss": "pow(rawTime - 0.5, 2) + 10 * pow(maxGap, 2)"},
    depends_on=[])

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
    # Set up config options

    # In this script we want to use one sources of input data.
    # Get the input files  from the input_data variable
    file_to_iov_physics = input_data["mumutight_or_highm_calib"]

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

    from ROOT.Belle2 import BeamSpotAlgorithm
    from basf2 import create_path, register_module
    import modularAnalysis as ana

    ###################################################
    # Calibration setup

    from caf.framework import Calibration
    from caf.strategies import SingleIOV

    # module to be run prior the collector
    rec_path_1 = create_path()

    muSelection = '[p>1.0]'
    muSelection += ' and abs(dz)<2.0 and abs(dr)<0.5'
    muSelection += ' and nPXDHits >=1 and nSVDHits >= 8 and nCDCHits >= 20'
    ana.fillParticleList('mu+:BS', muSelection, path=rec_path_1)
    ana.reconstructDecay('Upsilon(4S):BS -> mu+:BS mu-:BS', '9.5<M<11.5', path=rec_path_1)

    collector_bs = register_module('BeamSpotCollector', Y4SPListName='Upsilon(4S):BS')
    algorithm_bs = BeamSpotAlgorithm()
    algorithm_bs.setOuterLoss(kwargs['expert_config']['outerLoss'])
    algorithm_bs.setInnerLoss(kwargs['expert_config']['innerLoss'])

    calibration_bs = Calibration('BeamSpot',
                                 collector=collector_bs,
                                 algorithms=algorithm_bs,
                                 input_files=input_files_physics,
                                 pre_collector_path=rec_path_1)

    calibration_bs.strategies = SingleIOV

    # Do this for the default AlgorithmStrategy to force the output payload IoV
    # It may be different if you are using another strategy like SequentialRunByRun
    for algorithm in calibration_bs.algorithms:
        algorithm.params = {"iov_coverage": output_iov}

    # Most other options like database chain and backend args will be overwritten by b2caf-prompt-run.
    # So we don't bother setting them.

    # You must return all calibrations you want to run in the prompt process, even if it's only one
    return [calibration_bs]

##############################
