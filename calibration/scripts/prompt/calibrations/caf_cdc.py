# -*- coding: utf-8 -*-

"""CDC tracking calibration. Performs the T0 determination using HLT skimmed raw data."""

from prompt import CalibrationSettings
from prompt.utils import events_in_basf2_file, ExpRun
import basf2
from random import choice
from caf.framework import Calibration
from caf import strategies


#: Tells the automated system some details of this script
settings = CalibrationSettings(name="CDC Tracking",
                               expert_username="eberthol",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=["mumutight_calib", "hadron_calib", "cosmic_calib"],
                               input_data_filters={
                                   "mumutight_calib": ["mumutight_calib", "Good", "On"],
                                   "hadron_calib": ["hadron_calib", "Good", "On"],
                                   "cosmic_calib": ["cosmic_calib", "Good", "On"]},
                               depends_on=[],
                               expert_config={
                                   "max_files_per_run": 100000,
                                   "min_events_per_file": 1,
                                   "max_events_per_calibration": 200000,
                                   "max_events_per_calibration_for_xt_sr": 1000000,
                                   "max_events_per_file": 5000,
                                   "max_events_per_file_hadron": 2500,
                                   "payload_boundaries": [],
                                   "request_memory": 4
                               })


def select_files(all_input_files, min_events, max_processed_events_per_file):
    basf2.B2INFO("Attempting to choose a good subset of files")
    # Let's iterate, taking a sample of files from the total (no repeats or replacement) until we get enough events
    total_events = 0
    chosen_files = []
    while total_events < min_events:
        # If the set is empty we must have used all available files. Here we break and continue. But you may want to
        # raise an Error...
        if not all_input_files:
            break
        # Randomly select a file
        new_file_choice = choice(all_input_files)
        # Remove it from the list so it can't be chosen again
        all_input_files.remove(new_file_choice)
        # Find the number of events in the file
        total_events_in_file = events_in_basf2_file(new_file_choice)
        if not total_events_in_file:
            # Uh Oh! Zero event file, skip it
            continue
        events_contributed = 0
        if total_events_in_file < max_processed_events_per_file:
            # The file contains less than the max amount we have set (entrySequences)
            events_contributed = total_events_in_file
        else:
            events_contributed = max_processed_events_per_file
        chosen_files.append(new_file_choice)
        total_events += events_contributed

    basf2.B2INFO(f"Total chosen files = {len(chosen_files)}")
    basf2.B2INFO(f"Total events in chosen files = {total_events}")
    if total_events < min_events:
        raise ValueError(
            f"There weren't enough files events selected when max_processed_events_per_file={max_processed_events_per_file}")
    return chosen_files


################################################
# Required function called by b2caf-prompt-run #
################################################


def get_calibrations(input_data, **kwargs):
    import basf2
    from prompt.utils import filter_by_max_files_per_run
    # Gets the input files and IoV objects associated with the files.
    file_to_iov_mumu = input_data["mumutight_calib"]
    file_to_iov_hadron = input_data["hadron_calib"]
    file_to_iov_Bcosmics = input_data["cosmic_calib"]

    # read expert_config values
    expert_config = kwargs.get("expert_config")
    max_files_per_run = expert_config["max_files_per_run"]
    min_events_per_file = expert_config["min_events_per_file"]
    max_events_per_calibration = expert_config["max_events_per_calibration"]  # for t0, tw calib.
    max_events_per_calibration_for_xt_sr = expert_config["max_events_per_calibration_for_xt_sr"]  # for xt, sr calib.
    max_events_per_file = expert_config["max_events_per_file"]
    max_events_per_file_hadron = expert_config["max_events_per_file_hadron"]
    payload_boundaries = []
    payload_boundaries.extend([ExpRun(*boundary) for boundary in expert_config["payload_boundaries"]])
    basf2.B2INFO(f"Payload boundaries from expert_config: {payload_boundaries}")

    reduced_file_to_iov_mumu = filter_by_max_files_per_run(file_to_iov_mumu, max_files_per_run, min_events_per_file)
    input_files_mumu = list(reduced_file_to_iov_mumu.keys())
    basf2.B2INFO(f"Total number of mumutight_calib files actually used as input = {len(input_files_mumu)}")
    chosen_files_mumu = select_files(input_files_mumu[:], max_events_per_calibration, max_events_per_file)
    chosen_files_mumu_for_xt_sr = select_files(input_files_mumu[:], max_events_per_calibration_for_xt_sr, max_events_per_file)

    reduced_file_to_iov_hadron = filter_by_max_files_per_run(file_to_iov_hadron, max_files_per_run, min_events_per_file)
    input_files_hadron = list(reduced_file_to_iov_hadron.keys())
    basf2.B2INFO(f"Total number of hadron_calib files actually used as input = {len(input_files_hadron)}")
    chosen_files_hadron = select_files(input_files_hadron[:], max_events_per_calibration, max_events_per_file_hadron)
    chosen_files_hadron_for_xt_sr = select_files(input_files_hadron[:],
                                                 max_events_per_calibration_for_xt_sr, max_events_per_file_hadron)

    reduced_file_to_iov_Bcosmics = filter_by_max_files_per_run(file_to_iov_Bcosmics, max_files_per_run, min_events_per_file)
    input_files_Bcosmics = list(reduced_file_to_iov_Bcosmics.keys())
    basf2.B2INFO(f"Total number of cosmic_calib files actually used as input = {len(input_files_Bcosmics)}")
    chosen_files_Bcosmics = select_files(input_files_Bcosmics[:], max_events_per_calibration, max_events_per_file)
    chosen_files_Bcosmics_for_xt_sr = select_files(
        input_files_Bcosmics[:],
        max_events_per_calibration_for_xt_sr,
        max_events_per_file)

    input_file_dict = {
        "mumutight_calib": chosen_files_mumu_for_xt_sr,
        "hadron_calib": chosen_files_hadron_for_xt_sr,
        "cosmic_calib": chosen_files_Bcosmics_for_xt_sr
    }

    chosen_file_dict = {
        "mumutight_calib": chosen_files_mumu,
        "hadron_calib": chosen_files_hadron,
        "cosmic_calib": chosen_files_Bcosmics
    }

    # Get the overall IoV we want to cover, including the end values
    requested_iov = kwargs.get("requested_iov", None)

    from caf.utils import IoV
    # The actual IoV we want for any prompt request is open-ended
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    # for SingleIOV stratrgy, it's better to set the granularity to 'all' so that the collector jobs will run faster
    collector_granularity = 'all'
    if payload_boundaries:
        basf2.B2INFO('Found payload_boundaries: set collector granularity to run')
        collector_granularity = 'run'

    # t0
    cal0 = CDCCalibration(name='tz0',
                          algorithms=[tz_algo()],
                          input_file_dict=chosen_file_dict,
                          max_iterations=4,
                          max_events=max_events_per_file,
                          collector_granularity=collector_granularity
                          )

    # tw
    cal1 = CDCCalibration(name='tw0',
                          algorithms=[tw_algo()],
                          input_file_dict=chosen_file_dict,
                          max_iterations=2,
                          max_events=max_events_per_file,
                          collector_granularity=collector_granularity,
                          dependencies=[cal0]
                          )

    cal2 = CDCCalibration(name='tz1',
                          algorithms=[tz_algo()],
                          input_file_dict=chosen_file_dict,
                          max_iterations=4,
                          max_events=max_events_per_file,
                          collector_granularity=collector_granularity,
                          dependencies=[cal1]
                          )

    # xt
    cal3 = CDCCalibration(name='xt0',
                          algorithms=[xt_algo()],
                          input_file_dict=input_file_dict,
                          max_iterations=1,
                          collector_granularity=collector_granularity,
                          dependencies=[cal2]
                          )

    # space resolution
    cal4 = CDCCalibration(name='sr0',
                          algorithms=[sr_algo()],
                          input_file_dict=input_file_dict,
                          max_iterations=1,
                          collector_granularity=collector_granularity,
                          dependencies=[cal3]
                          )
    # t0
    cal5 = CDCCalibration(name='tz2',
                          algorithms=[tz_algo()],
                          input_file_dict=chosen_file_dict,
                          max_iterations=4,
                          max_events=max_events_per_file,
                          collector_granularity=collector_granularity,
                          dependencies=[cal4]
                          )

    if payload_boundaries:
        basf2.B2INFO("Found payload_boundaries: calibration strategies set to SequentialBoundaries.")
        cal0.strategies = strategies.SequentialBoundaries
        for algorithm in cal0.algorithms:
            algorithm.params = {"iov_coverage": output_iov, "payload_boundaries": payload_boundaries}
        for algorithm in cal1.algorithms:
            algorithm.params = {"iov_coverage": output_iov, "payload_boundaries": payload_boundaries}
        for algorithm in cal2.algorithms:
            algorithm.params = {"iov_coverage": output_iov, "payload_boundaries": payload_boundaries}
        for algorithm in cal3.algorithms:
            algorithm.params = {"iov_coverage": output_iov, "payload_boundaries": payload_boundaries}
        for algorithm in cal4.algorithms:
            algorithm.params = {"iov_coverage": output_iov, "payload_boundaries": payload_boundaries}
        for algorithm in cal5.algorithms:
            algorithm.params = {"iov_coverage": output_iov, "payload_boundaries": payload_boundaries}
    else:
        # Force the output payload IoV to be correct.
        # It may be different if you are using another strategy like SequentialRunByRun
        for algorithm in cal0.algorithms:
            algorithm.params = {"apply_iov": output_iov}
        for algorithm in cal1.algorithms:
            algorithm.params = {"apply_iov": output_iov}
        for algorithm in cal2.algorithms:
            algorithm.params = {"apply_iov": output_iov}
        for algorithm in cal3.algorithms:
            algorithm.params = {"apply_iov": output_iov}
        for algorithm in cal4.algorithms:
            algorithm.params = {"apply_iov": output_iov}
        for algorithm in cal5.algorithms:
            algorithm.params = {"apply_iov": output_iov}

    return [cal0, cal1, cal2, cal3, cal4, cal5]


#################################################

def pre_collector(max_events=None):
    """
    Define pre collection (reconstruction in our purpose).
    Probably, we need only CDC and ECL data.
    Parameters:
        max_events [int] : number of events to be processed.
                           All events by Default.
    Returns:
        path : path for pre collection
    """
    from basf2 import create_path, register_module
    from softwaretrigger.constants import HLT_INPUT_OBJECTS
    reco_path = create_path()
    if max_events is None:
        root_input = register_module(
            'RootInput',
            branchNames=HLT_INPUT_OBJECTS
        )
    else:
        root_input = register_module(
            'RootInput',
            branchNames=HLT_INPUT_OBJECTS,
            entrySequences=[
                '0:{}'.format(max_events)])
    reco_path.add_module(root_input)

    gearbox = register_module('Gearbox')
    reco_path.add_module(gearbox)
    reco_path.add_module('Geometry', useDB=True)

    from rawdata import add_unpackers
    # unpack raw data
    add_unpackers(reco_path)

    from reconstruction import add_reconstruction
    add_reconstruction(reco_path,
                       add_trigger_calculation=False,
                       trackFitHypotheses=[211, 13],
                       pruneTracks=False)

    return reco_path


def pre_collector_cr(max_events=None):
    """
    Define pre collection (reconstruction in our purpose).
    Probably, we need only CDC and ECL data.
    Parameters:
        max_events [int] : number of events to be processed.
                           All events by Default.
    Returns:
        path : path for pre collection
    """
    from basf2 import create_path, register_module
    from softwaretrigger.constants import HLT_INPUT_OBJECTS
    reco_path = create_path()
    if max_events is None:
        root_input = register_module(
            'RootInput',
            branchNames=HLT_INPUT_OBJECTS
        )
    else:
        root_input = register_module(
            'RootInput',
            branchNames=HLT_INPUT_OBJECTS,
            entrySequences=[
                '0:{}'.format(max_events)])
    reco_path.add_module(root_input)

    gearbox = register_module('Gearbox')
    reco_path.add_module(gearbox)
    reco_path.add_module('Geometry', useDB=True)

    from rawdata import add_unpackers
    # unpack raw data
    add_unpackers(reco_path)

    from reconstruction import add_cosmics_reconstruction
    add_cosmics_reconstruction(reco_path,
                               components=['CDC', 'ECL'],
                               merge_tracks=False,
                               pruneTracks=False,
                               data_taking_period='normal'
                               )
    return reco_path


def collector(bField=True, is_cosmic=False, granularity='all'):
    """
    Create a cdc calibration collector
    Parameters:
        bField [bool] : True if B field is on, else False
        isCosmic [bool] : True if cosmic events,
                          else (collision) False.
    Returns:
        collector : collector module
    """
    from basf2 import register_module
    col = register_module('CDCCalibrationCollector',
                          granularity=granularity,
                          calExpectedDriftTime=True,
                          eventT0Extraction=True,
                          bField=bField,
                          isCosmic=is_cosmic
                          )
    return col


def tz_algo():
    """
    Create a T0 calibration algorithm.
    Returns:
        algo : T0 algorithm
    """
    from ROOT import Belle2
    algo = Belle2.CDC.T0CalibrationAlgorithm()
    algo.storeHisto(True)
    algo.setMaxMeanDt(0.5)
    algo.setMaxRMSDt(0.1)
    algo.setMinimumNDF(20)
    return algo


def tw_algo():
    """
    Create a time walk calibration algorithm.
    Returns:
        algo : TW algorithm
    """
    from ROOT import Belle2
    algo = Belle2.CDC.TimeWalkCalibrationAlgorithm()
    algo.setStoreHisto(True)
    algo.setMode(1)
    return algo


def xt_algo():
    """
    Create a XT calibration algorithm.
    Parameters:
        prefix : prefixed name for algorithm,
                 which should be consistent with one of collector..
    Returns:
        algo : XT algorithm
    """
    from ROOT import Belle2
    algo = Belle2.CDC.XTCalibrationAlgorithm()
    algo.setStoreHisto(True)
    algo.setLRSeparate(True)
    algo.setThreshold(0.55)
    return algo


def sr_algo():
    """
    Create a Spacial resolution calibration algorithm.
    Parameters:
        prefix : prefixed name for algorithm,
                 which should be consistent with one of collector..
    Returns:
        algo : Spacial algorithm
    """
    from ROOT import Belle2
    algo = Belle2.CDC.SpaceResolutionCalibrationAlgorithm()
    algo.setStoreHisto(True)
    algo.setThreshold(0.4)
    return algo


class CDCCalibration(Calibration):
    '''
    CDCCalibration is a specialized calibration class for cdc.
    Since collector is same in all elements, no need to specify it.
    '''

    def __init__(self,
                 name,
                 algorithms,
                 input_file_dict,
                 max_iterations=5,
                 dependencies=None,
                 max_events=5000,
                 collector_granularity='All'):
        for algo in algorithms:
            algo.setHistFileName(name)

        super().__init__(name=name,
                         algorithms=algorithms
                         )

        from caf.framework import Collection

        for skim_type, file_list in input_file_dict.items():
            if skim_type == "Bcosmics":
                collection = Collection(collector=collector(granularity=collector_granularity),
                                        input_files=file_list,
                                        pre_collector_path=pre_collector_cr(max_events=max_events),
                                        )
            else:
                collection = Collection(collector=collector(granularity=collector_granularity),
                                        input_files=file_list,
                                        pre_collector_path=pre_collector(max_events=max_events),
                                        )
            self.add_collection(name=skim_type, collection=collection)

        self.max_iterations = max_iterations

        if dependencies is not None:
            for dep in dependencies:
                self.depends_on(dep)
