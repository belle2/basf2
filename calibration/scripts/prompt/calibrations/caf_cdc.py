##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""Full CDC tracking calibration."""
from prompt import CalibrationSettings, INPUT_DATA_FILTERS
from prompt.utils import events_in_basf2_file, ExpRun
import basf2
import ROOT
from ROOT import Belle2
from random import choice
from caf.framework import Calibration
from caf import strategies


#: Tells the automated system some details of this script
settings = CalibrationSettings(name="CDC Tracking",
                               expert_username="eberthol",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=["mumutight_calib", "hadron_calib", "cosmic_calib"],
                               input_data_filters={"mumutight_calib": [INPUT_DATA_FILTERS["Data Tag"]["mumutight_calib"],
                                                                       INPUT_DATA_FILTERS["Data Quality Tag"]["Good"],
                                                                       INPUT_DATA_FILTERS["Magnet"]["On"]],
                                                   "hadron_calib": [INPUT_DATA_FILTERS["Data Tag"]["hadron_calib"],
                                                                    INPUT_DATA_FILTERS["Data Quality Tag"]["Good"],
                                                                    INPUT_DATA_FILTERS["Magnet"]["On"]],
                                                   "cosmic_calib": [INPUT_DATA_FILTERS["Data Tag"]["cosmic_calib"],
                                                                    INPUT_DATA_FILTERS["Data Quality Tag"]["Good"],
                                                                    INPUT_DATA_FILTERS["Magnet"]["On"]]},
                               depends_on=[],
                               expert_config={
                                   "max_files_per_run": 1000,
                                   "min_events_per_file": 500,
                                   "max_events_per_file": 30000,
                                   "max_events_per_file_hadron_for_tz_tw": 5000,
                                   "max_events_per_file_hadron_for_xt_sr": 30000,
                                   "min_events_for_tz_tw_calibration": 500000,
                                   "max_events_for_tz_tw_calibration": 1000000,
                                   "min_events_for_xt_sr_calibration": 1000000,  # 1M
                                   "max_events_for_xt_sr_calibration": 10000000,  # 10M
                                   "fractions_for_each_type": [0.5, 1, 0.5],  # [mumu, hadron, cosmic]
                                   "payload_boundaries": [],
                                   "request_memory": 80
                               })


def select_files(all_input_files, min_events, max_events, max_processed_events_per_file, min_events_per_file=500):
    basf2.B2INFO(f"Minimum  number of events: {min_events}")
    basf2.B2INFO(f"Maximum  number of events: {max_events}")
    # Let's iterate, taking a sample of files from the total (no repeats or replacement) until we get enough events
    total_events = 0
    chosen_files = []
    while total_events < max_events:
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
        if not total_events_in_file or total_events_in_file < min_events_per_file:
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
        basf2.B2FATAL(
            f"There is not enough required events with setup max_processed_events_per_file = {max_processed_events_per_file}")
    return chosen_files


################################################
# Required function called by b2caf-prompt-run #
################################################


def get_calibrations(input_data, **kwargs):
    import basf2
    from prompt.utils import filter_by_max_files_per_run
    # Gets the input files and IoV objects associated with the files.

    # read expert_config values
    expert_config = kwargs.get("expert_config")
    max_files_per_run = expert_config["max_files_per_run"]
    min_events_per_file = expert_config["min_events_per_file"]
    max_events_per_file = expert_config["max_events_per_file"]

    min_events_for_tz_tw = expert_config["min_events_for_tz_tw_calibration"]  # for t0, tw calib.
    max_events_for_tz_tw = expert_config["max_events_for_tz_tw_calibration"]  # for t0, tw calib.
    max_events_per_file_hadron_for_tz_tw = expert_config["max_events_per_file_hadron_for_tz_tw"]

    min_events_for_xt_sr = expert_config["min_events_for_xt_sr_calibration"]  # for xt, sr calib.
    max_events_for_xt_sr = expert_config["max_events_for_xt_sr_calibration"]  # for xt, sr calib.
    max_events_per_file_hadron_for_xt_sr = expert_config["max_events_per_file_hadron_for_xt_sr"]

    Max_events_per_file_for_tz_tw = [max_events_per_file, max_events_per_file_hadron_for_tz_tw, max_events_per_file]
    Max_events_per_file_for_xt_sr = [max_events_per_file, max_events_per_file_hadron_for_xt_sr, max_events_per_file]

    fracion_of_event_for_types = expert_config["fractions_for_each_type"]
    basf2.B2INFO(f"Fraction for [di-muon, hadron, cosmic]: {fracion_of_event_for_types}")
    if len(fracion_of_event_for_types) != 3:
        basf2.B2FATAL("fraction of event must be an array with the size of 3, with order [mumu, hadron, cosmic]")

    payload_boundaries = []
    payload_boundaries.extend([ExpRun(*boundary) for boundary in expert_config["payload_boundaries"]])
    basf2.B2INFO(f"Payload boundaries from expert_config: {payload_boundaries}")

    files_for_xt_sr_dict = {}
    files_for_tz_tw_dict = {}

    if fracion_of_event_for_types[0] > 0:
        basf2.B2INFO("***********************Select di-muon data for calibration ***************")
        min_mumu_events_for_xt_sr = fracion_of_event_for_types[0] * min_events_for_xt_sr
        max_mumu_events_for_xt_sr = fracion_of_event_for_types[0] * max_events_for_xt_sr
        min_mumu_events_for_tz_tw = fracion_of_event_for_types[0] * min_events_for_tz_tw
        max_mumu_events_for_tz_tw = fracion_of_event_for_types[0] * max_events_for_tz_tw

        file_to_iov_mumu = input_data["mumutight_calib"]
        # select data file for t0 and tw calibration
        basf2.B2INFO("----> For T0 and Time walk correction")
        chosen_files_mumu_for_tz_tw = select_files(
            # input_files_mumu[:],
            list(file_to_iov_mumu.keys()),
            min_mumu_events_for_tz_tw,
            max_mumu_events_for_tz_tw,
            max_events_per_file)

        # select data file for xt, sigma calibration
        basf2.B2INFO("----> For XT, space resolution calib")
        chosen_files_mumu_for_xt_sr = select_files(list(file_to_iov_mumu.keys()),  # input_files_mumu[:],
                                                   min_mumu_events_for_xt_sr,
                                                   max_mumu_events_for_xt_sr,
                                                   max_events_per_file)

        files_for_xt_sr_dict["mumutight_calib"] = chosen_files_mumu_for_xt_sr
        files_for_tz_tw_dict["mumutight_calib"] = chosen_files_mumu_for_tz_tw

    if fracion_of_event_for_types[1] > 0:
        basf2.B2INFO("*********************** Select Hadron data for calibration ****************")
        min_hadron_events_for_tz_tw = fracion_of_event_for_types[1] * min_events_for_tz_tw
        max_hadron_events_for_tz_tw = fracion_of_event_for_types[1] * max_events_for_tz_tw
        min_hadron_events_for_xt_sr = fracion_of_event_for_types[1] * min_events_for_xt_sr
        max_hadron_events_for_xt_sr = fracion_of_event_for_types[1] * max_events_for_xt_sr

        file_to_iov_hadron = input_data["hadron_calib"]
        # select data file for tw, t0
        basf2.B2INFO("----> For T0 and Time walk correction")
        chosen_files_hadron_for_tz_tw = select_files(list(file_to_iov_hadron.keys()),
                                                     min_hadron_events_for_tz_tw,
                                                     max_hadron_events_for_tz_tw,
                                                     max_events_per_file_hadron_for_tz_tw)
        # select data file for xt, sigma
        basf2.B2INFO("----> For XT, space resolution calib")
        chosen_files_hadron_for_xt_sr = select_files(list(file_to_iov_hadron.keys()),
                                                     min_hadron_events_for_xt_sr,
                                                     max_hadron_events_for_xt_sr,
                                                     max_events_per_file_hadron_for_xt_sr)

        files_for_xt_sr_dict["hadron_calib"] = chosen_files_hadron_for_xt_sr
        files_for_tz_tw_dict["hadron_calib"] = chosen_files_hadron_for_tz_tw

        '''    For cosmic data '''
    if fracion_of_event_for_types[2] > 0:
        basf2.B2INFO("********************* Select cosmic data for calibration *******************")
        min_cosmic_events_for_tz_tw = fracion_of_event_for_types[2] * min_events_for_tz_tw
        max_cosmic_events_for_tz_tw = fracion_of_event_for_types[2] * max_events_for_tz_tw
        min_cosmic_events_for_xt_sr = fracion_of_event_for_types[2] * min_events_for_xt_sr
        max_cosmic_events_for_xt_sr = fracion_of_event_for_types[2] * max_events_for_xt_sr

        file_to_iov_cosmic = input_data["cosmic_calib"]

        # Select cosmic data for tw and t0 calibration
        basf2.B2INFO("---->For T0 and Time walk correction")
        chosen_files_cosmic_for_tz_tw = select_files(list(file_to_iov_cosmic.keys()),
                                                     min_cosmic_events_for_tz_tw,
                                                     max_cosmic_events_for_tz_tw,
                                                     max_events_per_file)

        # select cosmics data for xt and sigma calibration
        basf2.B2INFO("----> For T0 and Time walk correction")
        chosen_files_cosmic_for_xt_sr = select_files(list(file_to_iov_cosmic.keys()),
                                                     min_cosmic_events_for_xt_sr,
                                                     max_cosmic_events_for_xt_sr,
                                                     max_events_per_file)
        files_for_xt_sr_dict["cosmic_calib"] = chosen_files_cosmic_for_xt_sr
        files_for_tz_tw_dict["cosmic_calib"] = chosen_files_cosmic_for_tz_tw

    basf2.B2INFO("Complete input data selection.")

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
    tz0 = CDCCalibration(name='tz0',
                         algorithms=[tz_algo()],
                         input_file_dict=files_for_tz_tw_dict,
                         max_iterations=4,
                         max_events=Max_events_per_file_for_tz_tw,
                         use_badWires=True,
                         collector_granularity=collector_granularity
                         )
    # 1
    # xt
    xt0 = CDCCalibration(name='xt0',
                         algorithms=[xt_algo()],
                         input_file_dict=files_for_xt_sr_dict,
                         max_events=Max_events_per_file_for_xt_sr,
                         collector_granularity=collector_granularity,
                         dependencies=[tz0]
                         )

    # space resolution
    sr_tz0 = CDCCalibration(name='sr_tz0',
                            algorithms=[sr_algo(), tz_algo()],
                            input_file_dict=files_for_xt_sr_dict,
                            max_events=Max_events_per_file_for_xt_sr,
                            collector_granularity=collector_granularity,
                            max_iterations=0,
                            dependencies=[xt0]
                            )
    # 2
    # xt
    xt1 = CDCCalibration(name='xt1',
                         algorithms=[xt_algo()],
                         input_file_dict=files_for_xt_sr_dict,
                         max_events=Max_events_per_file_for_xt_sr,
                         collector_granularity=collector_granularity,
                         dependencies=[sr_tz0]
                         )

    # space resolution
    sr_tz1 = CDCCalibration(name='sr_tz1',
                            algorithms=[sr_algo(), tz_algo()],
                            input_file_dict=files_for_xt_sr_dict,
                            max_events=Max_events_per_file_for_xt_sr,
                            collector_granularity=collector_granularity,
                            max_iterations=0,
                            dependencies=[xt1]
                            )
    # tw
    tw0 = CDCCalibration(name='tw0',
                         algorithms=[tw_algo()],
                         input_file_dict=files_for_tz_tw_dict,
                         max_iterations=1,
                         max_events=Max_events_per_file_for_tz_tw,
                         collector_granularity=collector_granularity,
                         dependencies=[sr_tz1]
                         )

    tz1 = CDCCalibration(name='tz1',
                         algorithms=[tz_algo()],
                         input_file_dict=files_for_tz_tw_dict,
                         max_iterations=1,
                         max_events=Max_events_per_file_for_tz_tw,
                         collector_granularity=collector_granularity,
                         use_badWires=True,
                         dependencies=[tw0]
                         )
    # 2
    xt2 = CDCCalibration(name='xt2',
                         algorithms=[xt_algo()],
                         input_file_dict=files_for_xt_sr_dict,
                         max_events=Max_events_per_file_for_xt_sr,
                         collector_granularity=collector_granularity,
                         dependencies=[tz1]
                         )

    # space resolution
    sr_tz2 = CDCCalibration(name='sr_tz2',
                            algorithms=[sr_algo(), tz_algo()],
                            input_file_dict=files_for_xt_sr_dict,
                            max_events=Max_events_per_file_for_xt_sr,
                            collector_granularity=collector_granularity,
                            max_iterations=0,
                            dependencies=[xt2]
                            )
    tz2 = CDCCalibration(name='tz2',
                         algorithms=[tz_algo()],
                         input_file_dict=files_for_tz_tw_dict,
                         max_events=Max_events_per_file_for_tz_tw,
                         collector_granularity=collector_granularity,
                         max_iterations=0,
                         use_badWires=True,
                         dependencies=[sr_tz2])

    algs = [tz0, xt0, sr_tz0, xt1, sr_tz1, tw0, tz1, xt2, sr_tz2, tz2]

    if payload_boundaries:
        basf2.B2INFO("Found payload_boundaries: calibration strategies set to SequentialBoundaries.")
        algs[0].strategies = strategies.SequentialBoundaries
        for alg in algs:
            for algorithm in alg.algorithms:
                algorithm.params = {"iov_coverage": output_iov, "payload_boundaries": payload_boundaries}

    else:
        for alg in algs:
            for algorithm in alg.algorithms:
                algorithm.params = {"apply_iov": output_iov}

    return algs


#################################################

def pre_collector(max_events=None, is_cosmic=False, use_badWires=False):
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
    Components = ['CDC', 'ECL']

    from rawdata import add_unpackers
    add_unpackers(reco_path, components=Components)

    if is_cosmic:
        from reconstruction import add_cosmics_reconstruction
        # Add cdc tracking reconstruction modules
        add_cosmics_reconstruction(path=reco_path,
                                   components=Components,
                                   pruneTracks=False,
                                   merge_tracks=True,
                                   posttracking=False)
    else:
        from reconstruction import default_event_abort
        from tracking import add_tracking_reconstruction, add_prefilter_tracking_reconstruction

        # Do not even attempt at reconstructing events w/ abnormally large occupancy.
        doom = reco_path.add_module("EventsOfDoomBuster")
        default_event_abort(doom, ">=1", Belle2.EventMetaData.c_ReconstructionAbort)
        reco_path.add_module('StatisticsSummary').set_name('Sum_EventsofDoomBuster')

        # Add tracking reconstruction modules
        add_prefilter_tracking_reconstruction(path=reco_path,
                                              components=Components,
                                              trackFitHypotheses=[211],
                                              prune_temporary_tracks=False, fit_tracks=True)
        reco_path.add_module('StatisticsSummary').set_name('Sum_Tracking')

    reco_path.add_module('Progress')

    for module in reco_path.modules():
        if module.name() == "TFCDC_WireHitPreparer":
            module.param({"useBadWires": use_badWires})

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


def tz_algo(max_rmsDt=0.25, max_badChannel=50):
    """
    Create a T0 calibration algorithm.
    Returns:
        algo : T0 algorithm
    """
    from ROOT import Belle2
    algo = Belle2.CDC.T0CalibrationAlgorithm()
    algo.storeHisto(True)
    algo.setMaxMeanDt(0.2)
    algo.setMaxRMSDt(max_rmsDt)
    algo.setMaxBadChannel(max_badChannel)
    algo.setMinimumNDF(25)

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
    algo.setThreshold(0.1)
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
    algo.setThreshold(0.1)
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
                 max_events=[20000, 10000, 20000],
                 use_badWires=False,
                 collector_granularity='All'):
        for algo in algorithms:
            algo.setHistFileName(name)

        super().__init__(name=name,
                         algorithms=algorithms
                         )

        from caf.framework import Collection

        for skim_type, file_list in input_file_dict.items():
            if skim_type == "cosmic_calib":
                collection = Collection(collector=collector(is_cosmic=True,
                                                            granularity=collector_granularity),
                                        input_files=file_list,
                                        pre_collector_path=pre_collector(max_events=max_events[2],
                                                                         is_cosmic=True,
                                                                         use_badWires=use_badWires))
            elif skim_type == "hadron_calib":
                collection = Collection(collector=collector(granularity=collector_granularity),
                                        input_files=file_list,
                                        pre_collector_path=pre_collector(max_events=max_events[1],
                                                                         use_badWires=use_badWires))
            else:
                collection = Collection(collector=collector(granularity=collector_granularity),
                                        input_files=file_list,
                                        pre_collector_path=pre_collector(max_events=max_events[0],
                                                                         use_badWires=use_badWires))

            self.add_collection(name=skim_type, collection=collection)

        self.max_iterations = max_iterations

        if dependencies is not None:
            for dep in dependencies:
                self.depends_on(dep)
