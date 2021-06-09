import os
import argparse
import multiprocessing

import basf2
import ROOT

from softwaretrigger import constants
from pxd import add_roi_payload_assembler, add_roi_finder

from reconstruction import add_reconstruction, add_cosmics_reconstruction
from softwaretrigger import path_utils
from geometry import check_components
from rawdata import add_unpackers


def setup_basf2_and_db(zmq=False):
    """
    Setup local database usage for HLT
    """
    parser = argparse.ArgumentParser(description='basf2 for online')

    if zmq:
        parser.add_argument("--input", required=True, type=str, help="ZMQ Address of the distributor process")
        parser.add_argument("--output", required=True, type=str, help="ZMQ Address of the collector process")
        parser.add_argument("--dqm", required=True, type=str, help="ZMQ Address of the histoserver process")
    else:
        parser.add_argument('input_buffer_name', type=str,
                            help='Input Ring Buffer names')
        parser.add_argument('output_buffer_name', type=str,
                            help='Output Ring Buffer name')
        parser.add_argument('histo_port', type=int,
                            help='Port of the HistoManager to connect to')
        parser.add_argument('--input-file', type=str,
                            help="Input sroot file, if set no RingBuffer input will be used",
                            default=None)
        parser.add_argument('--output-file', type=str,
                            help="Filename for SeqRoot output, if set no RingBuffer output will be used",
                            default=None)
        parser.add_argument('--histo-output-file', type=str,
                            help="Filename for histogram output",
                            default=None)
        parser.add_argument('--no-output',
                            help="Don't write any output files",
                            action="store_true", default=False)

    parser.add_argument('--number-processes', type=int, default=multiprocessing.cpu_count() - 5,
                        help='Number of parallel processes to use')
    parser.add_argument('--local-db-path', type=str,
                        help="set path to the local payload locations to use for the ConditionDB",
                        default=constants.DEFAULT_DB_FILE_LOCATION)
    parser.add_argument('--central-db-tag', type=str, nargs="*",
                        help="Use the central db with a specific tag (can be applied multiple times, order is relevant)")

    args = parser.parse_args()

    # Local DB specification
    basf2.conditions.override_globaltags()
    if args.central_db_tag:
        for central_tag in args.central_db_tag:
            basf2.conditions.prepend_globaltag(central_tag)
    else:
        basf2.conditions.globaltags = ["online"]
        basf2.conditions.metadata_providers = ["file://" + basf2.find_file(args.local_db_path + "/metadata.sqlite")]
        basf2.conditions.payload_locations = [basf2.find_file(args.local_db_path)]

    # Number of processes
    basf2.set_nprocesses(args.number_processes)

    # Logging
    basf2.set_log_level(basf2.LogLevel.ERROR)
    # And because reasons we want every log message to be only one line,
    # otherwise the LogFilter in daq_slc throws away the other lines
    basf2.logging.enable_escape_newlines = True

    # Online realm
    basf2.set_realm("online")

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
        path.add_module('DqmHistoManager', Port=args.histo_port, DumpInterval=1000, workDirName="/tmp/")
    else:
        workdir = os.path.dirname(args.histo_output_file)
        filename = os.path.basename(args.histo_output_file)
        path.add_module('HistoManager', histoFileName=filename, workDirName=workdir)

    return path


def start_zmq_path(args, location):
    path = basf2.Path()
    reco_path = basf2.Path()

    if location == constants.Location.expressreco:
        input_module = path.add_module("HLTZMQ2Ds", input=args.input, addExpressRecoObjects=True)
    elif location == constants.Location.hlt:
        input_module = path.add_module("HLTZMQ2Ds", input=args.input)
    else:
        basf2.B2FATAL(f"Does not know location {location}")

    input_module.if_value("==0", reco_path, basf2.AfterConditionPath.CONTINUE)
    reco_path.add_module("HLTDQM2ZMQ", output=args.dqm, sendOutInterval=30)

    return path, reco_path


def add_hlt_processing(path,
                       run_type=constants.RunTypes.beam,
                       softwaretrigger_mode=constants.SoftwareTriggerModes.filter,
                       prune_input=True,
                       prune_output=True,
                       unpacker_components=None,
                       reco_components=None,
                       create_hlt_unit_histograms=True,
                       **kwargs):
    """
    Add all modules for processing on HLT filter machines
    """
    path.add_module('StatisticsSummary').set_name('Sum_Wait')

    if unpacker_components is None:
        unpacker_components = constants.DEFAULT_HLT_COMPONENTS
    if reco_components is None:
        reco_components = constants.DEFAULT_HLT_COMPONENTS

    check_components(unpacker_components)
    check_components(reco_components)

    # ensure that only DataStore content is present that we expect in
    # in the HLT configuration. If ROIpayloads or tracks are present in the
    # input file, this can be a problem and lead to crashes
    if prune_input:
        path.add_module("PruneDataStore", matchEntries=constants.HLT_INPUT_OBJECTS)

    # Add the geometry (if not already present)
    path_utils.add_geometry_if_not_present(path)
    path.add_module('StatisticsSummary').set_name('Sum_Initialization')

    # Unpack the event content
    add_unpackers(path, components=unpacker_components, writeKLMDigitRaws=True)
    path.add_module('StatisticsSummary').set_name('Sum_Unpackers')

    # Build one path for all accepted events...
    accept_path = basf2.Path()

    # Do the reconstruction needed for the HLT decision
    path_utils.add_filter_reconstruction(path, run_type=run_type, components=reco_components, **kwargs)

    # Add the part of the dqm modules, which should run after every reconstruction
    path_utils.add_hlt_dqm(path, run_type=run_type, components=reco_components, dqm_mode=constants.DQMModes.before_filter,
                           create_hlt_unit_histograms=create_hlt_unit_histograms)

    # Only turn on the filtering (by branching the path) if filtering is turned on
    if softwaretrigger_mode == constants.SoftwareTriggerModes.filter:
        # Now split up the path according to the HLT decision
        hlt_filter_module = path_utils.add_filter_module(path)

        # There are two possibilities for the output of this module
        # (1) the event is dismissed -> only store the metadata
        path_utils.hlt_event_abort(hlt_filter_module, "==0", ROOT.Belle2.EventMetaData.c_HLTDiscard)
        # (2) the event is accepted -> go on with the hlt reconstruction
        hlt_filter_module.if_value("==1", accept_path, basf2.AfterConditionPath.CONTINUE)
    elif softwaretrigger_mode == constants.SoftwareTriggerModes.monitor:
        # Otherwise just always go with the accept path
        path.add_path(accept_path)
    else:
        basf2.B2FATAL(f"The software trigger mode {softwaretrigger_mode} is not supported.")

    # For accepted events we continue the reconstruction
    path_utils.add_post_filter_reconstruction(accept_path, run_type=run_type, components=reco_components)

    # Only create the ROIs for accepted events
    add_roi_finder(accept_path)
    accept_path.add_module('StatisticsSummary').set_name('Sum_ROI_Finder')

    # Add the HLT DQM modules only in case the event is accepted
    path_utils.add_hlt_dqm(
        accept_path,
        run_type=run_type,
        components=reco_components,
        dqm_mode=constants.DQMModes.filtered,
        create_hlt_unit_histograms=create_hlt_unit_histograms)

    # Make sure to create ROI payloads for sending them to ONSEN
    # Do this for all events
    # Normally, the payload assembler marks the event with the software trigger decision to inform the hardware to
    # drop the data for the event in case the decision is "no"
    # However, if we are running in monitoring mode, we ignore the decision
    pxd_ignores_hlt_decision = (softwaretrigger_mode == constants.SoftwareTriggerModes.monitor)
    add_roi_payload_assembler(path, ignore_hlt_decision=pxd_ignores_hlt_decision)
    path.add_module('StatisticsSummary').set_name('Sum_ROI_Payload_Assembler')

    # Add the part of the dqm modules, which should run on all events, not only on the accepted onces
    path_utils.add_hlt_dqm(path, run_type=run_type, components=reco_components, dqm_mode=constants.DQMModes.all_events,
                           create_hlt_unit_histograms=create_hlt_unit_histograms)

    if prune_output:
        # And in the end remove everything which should not be stored
        path_utils.add_store_only_rawdata_path(path)
    path.add_module('StatisticsSummary').set_name('Sum_Close_Event')


def add_expressreco_processing(path,
                               run_type=constants.RunTypes.beam,
                               select_only_accepted_events=False,
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

    check_components(unpacker_components)
    check_components(reco_components)

    # If turned on, only events selected by the HLT will go to ereco.
    # this is needed as by default also un-selected events will get passed to ereco,
    # however they are empty.
    if select_only_accepted_events:
        skim_module = path.add_module("TriggerSkim", triggerLines=["software_trigger_cut&all&total_result"], resultOnMissing=0)
        skim_module.if_value("==0", basf2.Path(), basf2.AfterConditionPath.END)

    # ensure that only DataStore content is present that we expect in
    # in the ExpressReco configuration. If tracks are present in the
    # input file, this can be a problem and lead to crashes
    if prune_input:
        path.add_module("PruneDataStore", matchEntries=constants.EXPRESSRECO_INPUT_OBJECTS)

    path_utils.add_geometry_if_not_present(path)
    add_unpackers(path, components=unpacker_components, writeKLMDigitRaws=True)

    # dont filter/prune pxd for partly broken events, as we loose diagnostics in DQM
    basf2.set_module_parameters(path, "PXDPostErrorChecker", CriticalErrorMask=0)

    if do_reconstruction:
        if run_type == constants.RunTypes.beam:
            add_reconstruction(path, components=reco_components, pruneTracks=False,
                               skipGeometryAdding=True, add_trigger_calculation=False, **kwargs)
        elif run_type == constants.RunTypes.cosmic:
            add_cosmics_reconstruction(path, components=reco_components, pruneTracks=False,
                                       skipGeometryAdding=True, **kwargs)
        else:
            basf2.B2FATAL("Run Type {} not supported.".format(run_type))

    path_utils.add_expressreco_dqm(path, run_type, components=reco_components)

    # Will be removed later if not going to be used:
    # Build one path for all events coming from L1 passthrough...
    # l1_passthrough_path = basf2.Path()

    # Find if the event is triggered in L1_trigger filter line, if yes, send through l1_passthrough_path
    # l1_passthrough_module = path.add_module(
    #     "TriggerSkim",
    #     triggerLines=["software_trigger_cut&filter&L1_trigger"],
    #     resultOnMissing=0)
    # l1_passthrough_module.if_value("==1", l1_passthrough_path, basf2.AfterConditionPath.CONTINUE)

    # path_utils.add_expressreco_dqm(
    #     l1_passthrough_path,
    #     run_type,
    #     components=reco_components,
    #     dqm_mode=constants.DQMModes.l1_passthrough.name)

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


def finalize_zmq_path(path, args, location):
    """
    Add the required output modules for expressreco/HLT
    """
    save_objects = constants.ALWAYS_SAVE_OBJECTS + constants.RAWDATA_OBJECTS
    if location == constants.Location.expressreco:
        save_objects += constants.PROCESSED_OBJECTS

    # Limit streaming objects for parallel processing
    basf2.set_streamobjs(save_objects)

    if location == constants.Location.expressreco:
        path.add_module("HLTDs2ZMQ", output=args.output, raw=False)
    elif location == constants.Location.hlt:
        path.add_module("HLTDs2ZMQ", output=args.output, raw=True)
    else:
        basf2.B2FATAL(f"Does not know location {location}")
