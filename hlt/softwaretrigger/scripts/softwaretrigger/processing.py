import argparse

import basf2
import ROOT

from softwaretrigger import constants
from softwaretrigger.path_utils import add_geometry_if_not_present, add_expressreco_dqm, add_hlt_dqm
from pxd import add_roi_payload_assembler, add_roi_finder

from reconstruction import add_reconstruction, add_cosmics_reconstruction
from softwaretrigger.softwaretrigger_reconstruction import add_softwaretrigger_reconstruction, \
    add_cosmic_softwaretrigger_reconstruction

from rawdata import add_unpackers


def setup_basf2_and_db():
    """
    Setup local database usage for HLT
    """
    parser = argparse.ArgumentParser(description='basf2 for online')

    parser.add_argument('input_buffer_name', type=str,
                        help='Input Ring Buffer names')
    parser.add_argument('output_buffer_name', type=str,
                        help='Output Ring Buffer name')
    parser.add_argument('histo_port', type=int,
                        help='Port of the HistoManager to connect to')
    parser.add_argument('number_processes', type=int, default=0,
                        help='Number of parallel processes to use')
    parser.add_argument('--local-db-path', type=str,
                        help="set path to the local database.txt to use for the ConditionDB",
                        default=constants.DEFAULT_DB_FILE_LOCATION)
    parser.add_argument('--input-file', type=str,
                        help="Input sroot file, if set no RingBuffer input will be used",
                        default=None)
    parser.add_argument('--output-file', type=str,
                        help="Filename for SeqRoot output, if set no RingBuffer output will be used",
                        default=None)
    parser.add_argument('--histo-output-file', type=str,
                        help="Filename for histogram output",
                        default=None)
    parser.add_argument('--central-db-tag', type=str,
                        help="Use the central db with a specific tag",
                        default=None)
    parser.add_argument('--no-output',
                        help="Don't write any output files",
                        action="store_true", default=False)

    args = parser.parse_args()

    # Local DB specification
    basf2.reset_database()
    if args.central_db_tag:
        basf2.use_central_database(args.central_db_tag)
    else:
        basf2.use_local_database(ROOT.Belle2.FileSystem.findFile(args.local_db_path))

    # Number of processes
    basf2.set_nprocesses(args.number_processes)

    # Logging
    basf2.set_log_level(basf2.LogLevel.ERROR)

    return args


def start_path(args, location):
    """
    Create and return a path used for HLT and ExpressReco running
    """
    path = basf2.create_path()

    input_buffer_module_name = ""
    if location == constants.Location.expressreco:
        input_buffer_module_name = "Rbuf2Ds"
    elif location == constants.Location.hlt:
        input_buffer_module_name = "Raw2Ds"
    else:
        basf2.B2FATAL(f"Does not know location {location}")

    # Input
    if not args.input_file:
        path.add_module(input_buffer_module_name, RingBufferName=args.input_buffer_name)
    else:
        if args.input_file.endswith(".sroot"):
            path.add_module('SeqRootInput', inputFileName=args.input_file)
        else:
            path.add_module('RootInput', inputFileName=args.input_file)

    # Histogram Handling
    if not args.histo_output_file:
        path.add_module('DqmHistoManager', Port=args.histo_port, DumpInterval=1000)
    else:
        path.add_module('HistoManager', histoFileName=args.histo_output_file)

    return path


def add_hlt_processing(path,
                       run_type=constants.RunTypes.beam,
                       softwaretrigger_mode=constants.SoftwareTriggerModes.filter,
                       prune_input=True,
                       prune_output=True,
                       unpacker_components=None,
                       reco_components=None,
                       do_reconstruction=True,
                       **kwargs):
    """
    Add all modules for processing on HLT filter machines
    """
    if unpacker_components is None:
        unpacker_components = constants.DEFAULT_HLT_COMPONENTS
    if reco_components is None:
        reco_components = constants.DEFAULT_HLT_COMPONENTS

    # ensure that only DataStore content is present that we expect in
    # in the HLT configuration. If ROIpayloads or tracks are present in the
    # input file, this can be a problem and lead to crashes
    if prune_input:
        path.add_module("PruneDataStore", matchEntries=constants.HLT_INPUT_OBJECTS)

    add_geometry_if_not_present(path)
    add_unpackers(path, components=unpacker_components)

    if do_reconstruction:
        if run_type == constants.RunTypes.beam:
            accept_path = add_softwaretrigger_reconstruction(path, components=reco_components,
                                                             softwaretrigger_mode=softwaretrigger_mode,
                                                             **kwargs)
        elif run_type == constants.RunTypes.cosmic:
            basf2.B2ASSERT("There is no trigger menu for the run type cosmic!", softwaretrigger_mode == "monitor")

            accept_path = add_cosmic_softwaretrigger_reconstruction(path, components=reco_components,
                                                                    **kwargs)
        else:
            basf2.B2FATAL("Run Type {} not supported.".format(run_type))

        # Only create the ROIs for accepted events
        add_roi_finder(accept_path)

        # Add the HLT DQM modules only in case the event is accepted
        add_hlt_dqm(accept_path, run_type=run_type, components=reco_components)
    else:
        # Add the HLT DQM modules always
        add_hlt_dqm(path, run_type=run_type, components=reco_components)

    # Make sure to create ROI payloads for sending them to ONSEN
    # Do this for all events
    # Normally, the payload assembler dismisses the event if the software trigger says "no"
    # However, if (a) there is is software trigger (because there is no reconstruction) or
    # (b) we are running in monitoring mode, we ignore the decision
    pxd_ignores_hlt_decision = softwaretrigger_mode == "monitor" or not do_reconstruction
    add_roi_payload_assembler(path, ignore_hlt_decision=pxd_ignores_hlt_decision)

    if prune_output:
        path.add_module("PruneDataStore", matchEntries=constants.ALWAYS_SAVE_OBJECTS + constants.RAWDATA_OBJECTS)


def add_expressreco_processing(path,
                               run_type=constants.RunTypes.beam,
                               prune_input=True,
                               prune_output=True,
                               unpacker_components=None,
                               reco_components=None,
                               do_reconstruction=True,
                               **kwargs):
    """
    Add all modules for processing on the ExpressReco machines
    """
    if unpacker_components is None:
        unpacker_components = constants.DEFAULT_EXPRESSRECO_COMPONENTS
    if reco_components is None:
        reco_components = constants.DEFAULT_EXPRESSRECO_COMPONENTS

    # ensure that only DataStore content is present that we expect in
    # in the ExpressReco configuration. If tracks are present in the
    # input file, this can be a problem and lead to crashes
    if prune_input:
        path.add_module("PruneDataStore", matchEntries=constants.EXPRESSRECO_INPUT_OBJECTS)

    add_geometry_if_not_present(path)
    add_unpackers(path, components=unpacker_components)

    if do_reconstruction:
        if run_type == constants.RunTypes.beam:
            add_reconstruction(path, components=reco_components, pruneTracks=False,
                               skipGeometryAdding=True, **kwargs)
        elif run_type == constants.RunTypes.cosmic:
            add_cosmics_reconstruction(path, components=reco_components, pruneTracks=False,
                                       skipGeometryAdding=True, **kwargs)
        else:
            basf2.B2FATAL("Run Type {} not supported.".format(run_type))

    add_expressreco_dqm(path, run_type, components=reco_components)

    if prune_output:
        path.add_module("PruneDataStore", matchEntries=constants.ALWAYS_SAVE_OBJECTS + constants.RAWDATA_OBJECTS +
                        constants.PROCESSED_OBJECTS)


def finalize_path(path, args, location, show_progress_bar=True):
    """
    Add the required output modules for expressreco/HLT
    """
    save_objects = constants.ALWAYS_SAVE_OBJECTS + constants.RAWDATA_OBJECTS
    if location == constants.Location.expressreco:
        save_objects += constants.PROCESSED_OBJECTS

    if show_progress_bar:
        path.add_module("Progress")

    # Limit streaming objects for parallel processing
    basf2.set_streamobjs(save_objects)

    if args.no_output:
        return

    output_buffer_module_name = ""
    if location == constants.Location.expressreco:
        output_buffer_module_name = "Ds2Sample"
    elif location == constants.Location.hlt:
        output_buffer_module_name = "Ds2Rbuf"
    else:
        basf2.B2FATAL(f"Does not know location {location}")

    if not args.output_file:
        path.add_module(output_buffer_module_name, RingBufferName=args.output_buffer_name,
                        saveObjs=save_objects)
    else:
        if args.output_file.endswith(".sroot"):
            path.add_module("SeqRootOutput", saveObjs=save_objects, outputFileName=args.output_file)
        else:
            # We are storing everything on purpose!
            path.add_module("RootOutput", outputFileName=args.output_file)
