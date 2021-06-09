# -*- coding: utf-8 -*-

"""ECL timing self-consistency check (not orthogonal events so not a great validation)."""

from prompt import CalibrationSettings
from reconstruction import *
from caf.utils import IoV


##############################
# REQUIRED VARIABLE #
##############################
# Used to identify the keys in input_data that your get_calibrations function will need in order
# to assign data correctly.
# Will be used to construct the calibration in the automated system, as well as set up the submission web forms.
# You can view the available input data formats from CalibrationSettings.allowed_data_formats

#: Tells the automated system some details of this script.
#     Run over cdst bhabha_all_calib skim files and
#     the hadron skim for validations.
settings = CalibrationSettings(
    name="ECL time validations - bhabha and hadronic selections",
    expert_username="ehill",
    description=__doc__,
    input_data_formats=["cdst"],
    input_data_names=["bhabha_all_calib", "hadron_calib"],
    input_data_filters={
        "bhabha_all_calib": [
           "bhabha_all_calib",
           "4S",
           "Continuum",
           "Scan",
           "Good",
           "physics",
           "On"],
        "hadron_calib": [
           "hadron_calib",
           "4S",
           "Continuum",
           "Scan",
           "Good",
           "physics",
           "On"]},
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
    # The input data should be the bhabha and hadron skims
    file_to_iov_bhabha = input_data["bhabha_all_calib"]
    file_to_iov_hadron = input_data["hadron_calib"]

    max_events_per_run = 3000

    # We filter addition files if there are more than [max_events_per_run] files per run.
    # The input data files are sorted alphabetically by b2caf-prompt-run
    # already. This procedure respects that ordering
    from prompt.utils import filter_by_max_events_per_run

    reduced_file_to_iov_bhabha = filter_by_max_events_per_run(file_to_iov_bhabha, max_events_per_run)
    input_files_bhabha = list(reduced_file_to_iov_bhabha.keys())
    basf2.B2INFO(f"Total number of bhabha files actually used as input = {len(input_files_bhabha)}")

    reduced_file_to_iov_hadron = filter_by_max_events_per_run(file_to_iov_hadron, max_events_per_run)
    input_files_hadron = list(reduced_file_to_iov_hadron.keys())
    basf2.B2INFO(f"Total number of hadron files actually used as input = {len(input_files_hadron)}")

    ###################################################
    import basf2
    from basf2 import register_module, create_path
    import ROOT
    from ROOT import Belle2
    from ROOT.Belle2 import TestCalibrationAlgorithm
    from caf.framework import Collection

    #############################################################
    # Collector setup for ecl timing validation with bhabhas
    root_input = register_module('RootInput')
    rec_path_bhabha = create_path()
    rec_path_bhabha.add_module(root_input)
    if 'Gearbox' not in rec_path_bhabha:
        rec_path_bhabha.add_module('Gearbox')
    if 'Geometry' not in rec_path_bhabha:
        rec_path_bhabha.add_module('Geometry', useDB=True)

    prepare_user_cdst_analysis(rec_path_bhabha)    # for new 2020 cdst format

    col_bhabha = register_module('eclBhabhaTimeCalibrationValidationCollector')
    col_bhabha.param('timeAbsMax', 70)
    col_bhabha.param('saveTree', False)

    eclValTCol = Collection(collector=col_bhabha,
                            input_files=input_files_bhabha,
                            pre_collector_path=rec_path_bhabha)

    ##############################################################
    # Algorithm setup for ecl timing validation with bhabhas

    # Give the collector name to the algorithm since one algorithm
    # is used to analyse the results from several possible collectors
    eclValTAlgBhabha = Belle2.ECL.eclTValidationAlgorithm("eclBhabhaTimeCalibrationValidationCollector")

    # Define the CAF algorithm arguments
    # eclValTAlgBhabha.cellIDLo= 3
    # eclValTAlgBhabha.cellIDHi = 2
    eclValTAlgBhabha.meanCleanRebinFactor = 3
    eclValTAlgBhabha.meanCleanCutMinFactor = 0.4
    eclValTAlgBhabha.debugFilenameBase = "eclBhabhaTValidationAlgorithm"

    ####################################################################
    # Calibration setup base for ecl timing validation with bhabhas

    from caf.framework import Calibration

    valid_cal_bhabha = Calibration("ECLcrystalTimeCalValidation_bhabhaPhysics")
    valid_cal_bhabha.add_collection(name="bhabha", collection=eclValTCol)
    valid_cal_bhabha.algorithms = [eclValTAlgBhabha]

    # Here we set the AlgorithmStrategy for our algorithm
    from caf.strategies import SingleIOV

    # The default value is SingleIOV, you don't have to set this, it is done automatically.
    # SingleIOV just takes all of the runs as one big IoV and executes the algorithm once on all of their data.
    # You can use granularity='run' or granularity='all' for the collector when using this strategy.

    valid_cal_bhabha.strategies = SingleIOV

    #############################################################
    # Collector setup for ecl timing validation with hadrons
    root_input = register_module('RootInput')
    rec_path_hadron = create_path()
    rec_path_hadron.add_module(root_input)
    if 'Gearbox' not in rec_path_hadron:
        rec_path_hadron.add_module('Gearbox')
    if 'Geometry' not in rec_path_hadron:
        rec_path_hadron.add_module('Geometry', useDB=True)

    prepare_user_cdst_analysis(rec_path_hadron)    # for new 2020 cdst format

    col_hadron = register_module('eclHadronTimeCalibrationValidationCollector')
    col_hadron.param('timeAbsMax', 70)
    col_hadron.param('saveTree', False)

    eclValTCol = Collection(collector=col_hadron,
                            input_files=input_files_hadron,
                            pre_collector_path=rec_path_hadron)

    ##############################################################
    # Algorithm setup for ecl timing validation with hadrons

    # Give the collector name to the algorithm since one algorithm
    # is used to analyse the results from several possible collectors
    eclValTAlgHadronic = Belle2.ECL.eclTValidationAlgorithm("eclHadronTimeCalibrationValidationCollector")

    # Define the CAF algorithm arguments
    # eclValTAlgHadronic.cellIDLo= 3
    # eclValTAlgHadronic.cellIDHi = 2
    eclValTAlgHadronic.meanCleanRebinFactor = 3
    eclValTAlgHadronic.meanCleanCutMinFactor = 0.4
    eclValTAlgHadronic.debugFilenameBase = "eclHadronTValidationAlgorithm"

    ####################################################################
    # Calibration setup base for ecl timing validation with hadrons

    from caf.framework import Calibration

    valid_cal_hadron = Calibration("ECLcrystalTimeCalValidation_hadronPhysics")
    valid_cal_hadron.add_collection(name="hadron", collection=eclValTCol)
    valid_cal_hadron.algorithms = [eclValTAlgHadronic]

    # Here we set the AlgorithmStrategy for our algorithm
    from caf.strategies import SingleIOV

    # The default value is SingleIOV, you don't have to set this, it is done automatically.
    # SingleIOV just takes all of the runs as one big IoV and executes the algorithm once on all of their data.
    # You can use granularity='run' or granularity='all' for the collector when using this strategy.

    valid_cal_hadron.strategies = SingleIOV

    ###################################################
    # Finalize all "calibrations", i.e. validations

    # You must return all calibrations you want to run in the prompt process, even if it's only one
    return [valid_cal_bhabha, valid_cal_hadron]
