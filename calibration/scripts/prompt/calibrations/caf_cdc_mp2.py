##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
# -*- coding: utf-8 -*-

"""CDC tracking calibration. Performs the T0 determination using HLT skimmed raw data."""

from prompt import CalibrationSettings
from prompt.utils import events_in_basf2_file
import basf2
from random import choice


#: Tells the automated system some details of this script
settings = CalibrationSettings(name="CDC T0 Calibration with MP2",
                               expert_username="uchida",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=["hlt_mumu", "hlt_hadron"],
                               depends_on=[])


def fix_tw_param():
    from ROOT import Belle2
    result = []
    bad_boards = [0, 35, 37, 77, 97, 115, 133, 193, 204, 218, 247]
    for ib in range(300):
        if ib in bad_boards:
            label0 = Belle2.GlobalLabel()
            label0.construct(Belle2.CDCTimeWalks.getGlobalUniqueID(), ib, 0)
            result.append(label0.label())
            label1 = Belle2.GlobalLabel()
            label1.construct(Belle2.CDCTimeWalks.getGlobalUniqueID(), ib, 1)
            result.append(label1.label())
    return result


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
    import alignment
    import alignment.constraints
    # Gets the input files and IoV objects associated with the files.
    file_to_iov_mumu = input_data["hlt_mumu"]
    file_to_iov_hadron = input_data["hlt_hadron"]
    # file_to_iov_Bcosmics = input_data["Bcosmics"]

    max_files_per_run = 10
    min_events_per_file = 1000

    max_events_per_calibration = 50000
    max_events_per_calibration_xt = 1000000
    max_events_per_file = 2000
    max_events_per_file_had = 1000

    reduced_file_to_iov_mumu = filter_by_max_files_per_run(file_to_iov_mumu, max_files_per_run, min_events_per_file)
    input_files_mumu = list(reduced_file_to_iov_mumu.keys())
    chosen_files_mumu = select_files(input_files_mumu[:], max_events_per_calibration, max_events_per_file)
    chosen_files_mumu_xt = select_files(input_files_mumu[:], max_events_per_calibration_xt, max_events_per_file)
    basf2.B2INFO(f"Total number of hlt_mumu files actually used as input = {len(input_files_mumu)}")

    reduced_file_to_iov_hadron = filter_by_max_files_per_run(file_to_iov_hadron, max_files_per_run, min_events_per_file)
    input_files_hadron = list(reduced_file_to_iov_hadron.keys())
    chosen_files_hadron = select_files(input_files_hadron[:], max_events_per_calibration, max_events_per_file_had)
    chosen_files_hadron_xt = select_files(input_files_hadron[:], max_events_per_calibration_xt, max_events_per_file_had)
    basf2.B2INFO(f"Total number of hlt_hadron files actually used as input = {len(input_files_hadron)}")

    # Get the overall IoV we want to cover, including the end values
    requested_iov = kwargs.get("requested_iov", None)

    from caf.utils import IoV
    # The actuall IoV we want for any prompt request is open-ended
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)
    import millepede_calibration as mp2
    # Calibrate time zero and time walk simultaneously.
    cal1 = mp2.create(
        name='tzw0',
        dbobjects=['CDCTimeZeros', 'CDCTimeWalks'],
        collections=[
            mp2.make_collection('hlt_mumu', path=pre_collector(), tracks=['RecoTracks']),
            mp2.make_collection('hlt_hadron', path=pre_collector(), tracks=['RecoTracks'])],
        files=dict(hlt_mumu=chosen_files_mumu, hlt_hadron=chosen_files_hadron),
        tags=None,
        timedep=[],
        constraints=[alignment.constraints.Constraints(basf2.find_file('cdc/data/cdc-T0-constraints.txt'))],
        fixed=fix_tw_param(),
        commands=['method inversion 1 0.1',
                  'threads 25 1',
                  'chiscut 30. 6.',
                  'entries 100',
                  'scaleerrors 1. 1.'],
        params=dict(minPValue=0., externalIterations=0),
        min_entries=10000)

    basf2.set_module_parameters(cal1.collections['hlt_mumu'].pre_collector_path,
                                'RootInput', entrySequences=[f'0:{max_events_per_file}'])
    basf2.set_module_parameters(cal1.collections['hlt_hadron'].pre_collector_path,
                                'RootInput', entrySequences=[f'0:{max_events_per_file_had}'])
    cal1.max_iterations = 5

    # Calibration of XT is separated from tz/tw. in future, it will be simultaniously calibrated...
    cal2 = mp2.create(
        name='xt0',
        dbobjects=['CDCXtRelations'],
        collections=[
            mp2.make_collection('hlt_mumu', path=pre_collector(), tracks=['RecoTracks']),
            mp2.make_collection('hlt_hadron', path=pre_collector(), tracks=['RecoTracks'])],
        files=dict(hlt_mumu=chosen_files_mumu_xt, hlt_hadron=chosen_files_hadron_xt),
        tags=None,
        timedep=[],
        constraints=[],
        fixed=[],
        commands=['method sparseMINRES-QLP 3 0.01',
                  'threads 25 1',
                  'chiscut 30. 6.',
                  'entries 100',
                  'scaleerrors 1. 1.'],
        params=dict(minPValue=0., externalIterations=0),
        min_entries=10000)

    basf2.set_module_parameters(cal2.collections['hlt_mumu'].pre_collector_path,
                                'RootInput', entrySequences=[f'0:{max_events_per_file}'])
    basf2.set_module_parameters(cal2.collections['hlt_hadron'].pre_collector_path,
                                'RootInput', entrySequences=[f'0:{max_events_per_file_had}'])
    cal2.max_iterations = 2
    # Force the output payload IoV to be correct.
    # It may be different if you are using another strategy like SequentialRunByRun
    for algorithm in cal1.algorithms:
        algorithm.params = {"apply_iov": output_iov}
    for algorithm in cal2.algorithms:
        algorithm.params = {"apply_iov": output_iov}

    return [cal1, cal2]


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
    import modularAnalysis as ana
    reco_path = create_path()
    if max_events is None:
        root_input = register_module('RootInput')
    else:
        root_input = register_module('RootInput',
                                     entrySequences=['0:{}'.format(max_events)]
                                     )
    reco_path.add_module(root_input)

    gearbox = register_module('Gearbox')
    reco_path.add_module(gearbox)
    reco_path.add_module('Geometry', useDB=True)

    from rawdata import add_unpackers
    # unpack raw data
    add_unpackers(reco_path)

    from reconstruction import add_reconstruction
    add_reconstruction(reco_path,
                       components=['PXD', 'SVD', 'CDC', 'ECL'],
                       add_trigger_calculation=False,
                       trackFitHypotheses=[211, 13],
                       pruneTracks=False)

    ana.fillParticleList('mu+:qed', 'muonID > 0.1 and useCMSFrame(p) > 2.', writeOut=True, path=reco_path)
    ana.reconstructDecay('Z0:mumu -> mu-:qed mu+:qed', '', writeOut=True, path=reco_path)
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
    reco_path = create_path()
    if max_events is None:
        root_input = register_module('RootInput')
    else:
        root_input = register_module('RootInput',
                                     entrySequences=['0:{}'.format(max_events)]
                                     )
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


def collector(bField=True, is_cosmic=False):
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
                          granularity='all',
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
