# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""Plot the crate time jumps"""

from prompt import CalibrationSettings
from reconstruction import *
from caf.utils import IoV
import array as arr


##############################
# REQUIRED VARIABLE #
##############################
# Used to identify the keys in input_data that your get_calibrations function will need in order
# to assign data correctly.
# Will be used to construct the calibration in the automated system, as well as set up the submission web forms.
# You can view the available input data formats from CalibrationSettings.allowed_data_formats

#: Tells the automated system some details of this script.
#     Default is to read in "caf_ecl_time_" since we want to
#     run over cdst caf_ecl_time_ skim files.
settings = CalibrationSettings(name="ECL crystal time calibrations",
                               expert_username="ehill",
                               description=__doc__,
                               input_data_formats=["cdst", "mdst"],
                               input_data_names=["bhabha_all_calib"],
                               input_data_filters={"bhabha_all_calib": ["bhabha_all_calib"]},
                               depends_on=[])


##############################

##############################
# REQUIRED FUNCTION #
##############################
# The only function that MUST exist in this module. It should return a LIST of Calibration objects
# that have had their input files assigned and any configuration applied. The final output payload IoV(s)
# should also be set correctly to be open-ended e.g. IoV(exp_low, run_low, -1, -1)
#
# The database_chain, backend_args, backend, max_files_per_collector_job, and heartbeat of these
# calibrations will all be set/overwritten by the b2caf-prompt-run tool.


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
    # The input data should be the bhabha skim
    file_to_iov_physics = input_data["bhabha_all_calib"]

    # Could remove this limit on the number of files per run but will just
    # set to a large number in case we want to introduce it later.
    # Also, keeping it allows the crystal calibrations code to look like the
    # crates calibration code.
    max_events_per_run = 1

    # We filter addition files if there are more than [max_events_per_run] events per run.
    # I'm not sure but I hope this will speed up the collector stage.
    # The input data files are sorted alphabetically by b2caf-prompt-run
    # already. This procedure respects that ordering.
    from prompt.utils import filter_by_max_events_per_run

    reduced_file_to_iov_physics = filter_by_max_events_per_run(file_to_iov_physics, max_events_per_run)
    input_files_physics = list(reduced_file_to_iov_physics.keys())

    ###################################################
    import basf2
    from basf2 import register_module, create_path
    import ROOT
    from ROOT import Belle2
    from ROOT.Belle2 import TestCalibrationAlgorithm
    from caf.framework import Collection

    ###################################################
    # Collector setup

    # Set up the collector but with only one event per file
    root_input = register_module('RootInput', entrySequences=['0:{}'.format(1)])

    rec_path_bhabha = create_path()
    rec_path_bhabha.add_module(root_input)
    if 'Gearbox' not in rec_path_bhabha:
        rec_path_bhabha.add_module('Gearbox')
    if 'Geometry' not in rec_path_bhabha:
        rec_path_bhabha.add_module('Geometry', useDB=True)

    prepare_cdst_analysis(rec_path_bhabha)  # for new 2020 cdst format

    col_bhabha = register_module('eclTimeShiftsPlottingCollector')
    eclTCol = Collection(collector=col_bhabha,
                         # input_files=input_files_first_last,
                         input_files=input_files_physics,
                         pre_collector_path=rec_path_bhabha,
                         )

    ########################################################################
    # Algorithm setup
    # Make plots of the crate time shifts as a function of the run number

    from caf.framework import Calibration

    tShifts_alg = Belle2.ECL.eclTimeShiftsAlgorithm()
    tShifts_alg.debugFilenameBase = "eclTimeShiftsAlgorithm"

    # Define offsets so that the crysta+crate time plots are centred close to zero.
    # tShifts_alg.timeShiftForPlotStyle = arr.array('d', [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  \
    #                                       0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  \
    #                                       0,0,0,0,0,0,0,0,0,0,0,0,0])
    # tShifts_alg.timeShiftForPlotStyle = arr.array('d', [-29, -40, -9., -13, -35, -39,   \
    #                                      -42, -24, -20, -22, -16, -23, -35, -31, -10,   \
    #                                      -23, -47, -23, -21, -7., -31, -22, -30, -30,   \
    #                                      -33, 5., 9., 12., -11, -19, -30, -29, -37, -19,   \
    #                                      -20, -23, 37., 37., 57., 27., 53., 26., 69., 27.,   \
    #                                      66., 73., 52., 42., 55., 71., 110, 52.])

    # +-17ns range allows for four 8ns crate time jumps in one direction
    # +-10ns range allows for two 8ns crate time jumps in one direction

    tShifts_alg.crysCrateShift_min = -30   # in ns
    tShifts_alg.crysCrateShift_max = 30    # in ns

    # Make the algorithm loop over the runs, not just the collector
    # tShifts_alg.algorithmReadPayloads = True

    ###################################################
    # "Calibration" setup
    # Combine the collector and algorithm for execution

    cal_ecl_timeShifts = Calibration(name="ecl_t_shifts", algorithms=[tShifts_alg],
                                     input_files=input_files_physics)
    cal_ecl_timeShifts.add_collection(name="bhabha", collection=eclTCol)
    cal_ecl_timeShifts.save_payloads = False

    ###################################################
    # Finalize all calibrations
    return [cal_ecl_timeShifts]
