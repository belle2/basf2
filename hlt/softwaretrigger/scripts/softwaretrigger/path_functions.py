import sys
import basf2
import argparse
from softwaretrigger import (
    SOFTWARE_TRIGGER_GLOBAL_TAG_NAME
)
import ROOT
import softwaretrigger.hltdqm as hltdqm

import reconstruction
from softwaretrigger import add_fast_reco_software_trigger, add_hlt_software_trigger, \
    add_calibration_software_trigger, add_calcROIs_software_trigger

from daqdqm.collisiondqm import add_collision_dqm
from daqdqm.cosmicdqm import add_cosmic_dqm

from rawdata import add_unpackers

RAW_SAVE_STORE_ARRAYS = ["RawCDCs", "RawSVDs", "RawTOPs", "RawARICHs", "RawKLMs", "RawECLs", "RawFTSWs", "ROIs"]
ALWAYS_SAVE_REGEX = ["EventMetaData", "SoftwareTrigger.*", "TRGSummary"]
DEFAULT_HLT_COMPONENTS = ["CDC", "SVD", "ECL", "TOP", "ARICH", "BKLM", "EKLM"]
DEFAULT_EXPRESSRECO_COMPONENTS = DEFAULT_HLT_COMPONENTS + ["PXD"]


def setup_basf2_and_db(dbfile=None):
    """
    Setupl local database usage for HLT
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
                        default="hlt/examples/LocalDB/database.txt")

    args = parser.parse_args()

    basf2.set_log_level(basf2.LogLevel.ERROR)
    ##########
    # Local DB specification
    ##########
    basf2.reset_database()
    if dbfile is None:
        basf2.use_local_database(ROOT.Belle2.FileSystem.findFile(args.local_db_path))
    else:
        basf2.use_local_database(ROOT.Belle2.FileSystem.findFile(dbfile))

    basf2.set_nprocesses(args.number_processes)

    return args


def create_hlt_path(args, inputfile='DAQ', dqmfile='DAQ'):
    """
    Create and return a path used for HLT and ExpressReco running
    """
    path = basf2.create_path()
    # todo: insert crash handling path
    # crashsafe_path = basf2.create_path()

    ##########
    # Input
    ##########
    if inputfile == 'DAQ':
        # Input from ringbuffer (for raw data)
        input = basf2.register_module('Raw2Ds')
        input.param("RingBufferName", args.input_buffer_name)
    else:
        # Input from SeqRootInput
        input = basf2.register_module('SeqRootInput')
        input.param('inputFileName', inputfile)
    path.add_module(input)

    ##########
    # Histogram Handling
    ##########
    if dqmfile == 'DAQ':
        # HistoManager for real HLT
        histoman = basf2.register_module('DqmHistoManager')
        histoman.param("Port", args.histo_port)
        histoman.param("DumpInterval", 1000)
    else:
        histoman = basf2.register_module('HistoManager')
        histoman.param('histoFileName', dqmfile)
    path.add_module(histoman)

    return path


def finalize_hlt_path(path, args, show_progress_bar=False, outputfile='DAQ'):
    """
    Add the required output modules for HLT
    """
    ##########
    # Output
    ##########
    if outputfile == 'DAQ':
        # Output to RingBuffer
        output = basf2.register_module("Ds2Rbuf")
        output.param("RingBufferName", args.output_buffer_name)
    else:
        # Output to SeqRoot
        output = basf2.register_module("SeqRootOutput")
        output.param('outputFileName', outputfile)
        # output file name should be specified with -o option

    # Specification of output objects
    output.param("saveObjs", ALWAYS_SAVE_REGEX + RAW_SAVE_STORE_ARRAYS)

    path.add_module(output)

    ##########
    # Other utilities
    ##########
    if show_progress_bar:
        progress = basf2.register_module('Progress')
        path.add_module(progress)

    etime = basf2.register_module('ElapsedTime')
    path.add_module(etime)


def add_hlt_processing(path, run_type="collision",
                       with_bfield=True,
                       pruneDataStore=True,
                       additonal_store_arrays_to_keep=[],
                       components=DEFAULT_HLT_COMPONENTS,
                       reco_components=None,
                       softwaretrigger_mode='hlt_filter', **kwargs):
    """
    Add all modules for processing on HLT filter machines
    """
    add_unpackers(path, components=components)

    # if not set, just assume to reuse the normal compontents list
    if reco_components is None:
        reco_components = components

    if run_type == "collision":
        # todo: forward the the mag field and run_type mode into this method call
        add_softwaretrigger_reconstruction(path,
                                           components=reco_components,
                                           softwaretrigger_mode=softwaretrigger_mode,
                                           run_type=run_type,
                                           addDqmModules=True, **kwargs)
    elif run_type == "cosmics":
        # no filtering,
        reconstruction.add_cosmics_reconstruction(path, components=reco_components, **kwargs)
        add_hlt_dqm(path, run_type, components=components)
        if pruneDataStore:
            path.add_module(
                "PruneDataStore",
                matchEntries=ALWAYS_SAVE_REGEX +
                RAW_SAVE_STORE_ARRAYS +
                additonal_store_arrays_to_keep)

    else:
        basf2.B2FATAL("Run Type {} not supported.".format(run_type))


def add_expressreco_processing(path, run_type="collision",
                               with_bfield=True,
                               components=DEFAULT_EXPRESSRECO_COMPONENTS,
                               reco_components=None,
                               do_reconstruction=True, **kwargs):
    """
    Add all modules for processing on the ExpressReco machines
    """
    if not do_reconstruction:
        add_geometry_if_not_present(path)

    add_unpackers(path, components=components)

    # if not set, just assume to reuse the normal compontents list
    if reco_components is None:
        reco_components = components

    if do_reconstruction:
        if run_type == "collision":
            reconstruction.add_reconstruction(path, components=reco_components)
        elif run_type == "cosmics":
            reconstruction.add_cosmics_reconstruction(path, components=reco_components, **kwargs)
        else:
            basf2.B2FATAL("Run Type {} not supported.".format(run_type))

    add_expressreco_dqm(path, run_type, components=components)


def add_softwaretrigger_reconstruction(
        path,
        store_array_debug_prescale=0,
        components=DEFAULT_HLT_COMPONENTS,
        trackFitHypotheses=[211],
        softwaretrigger_mode='hlt_filter',
        addDqmModules=False,
        run_type="collision",
        additonal_store_arrays_to_keep=[],
        pruneDataStore=True,
        calcROIs=True):
    """
    Add all modules, conditions and conditional paths to the given path, that are needed for a full
    reconstruction stack in the HLT using the software trigger modules. Several steps are performed:

    * Use FastReco to pre cut on certain types of background events
    * Use the SoftwareTriggerModule to actually calculate FastReco variables and do the cuts (the cuts are downloaded
      from the database).
    * Depending on the output of the module, either dismiss everything except the EventMetaData and the SoftwareTrigger
      results or call the remaining full reconstruction.
    * Create two particle lists pi+:HLT and gamma:HLT
    * Use the SoftwareTriggerModule again to calculate more advanced variables and do HLT cuts (again downloaded
      from the database).
    * Depending on the output of this module, dismiss everything except the meta or except the raw data from the
      data store.

    The whole setup looks like this:

                                                 -- [ Calibration ] -- [ Raw Data ] ---
                                               /                                        \
             In -- [ Fast Reco ] -- [ HLT ] --                                            -- Out
                                 \             \                                        /
                                  ----------------- [ Meta Data ] ---------------------


    Before calling this function, make sure that your database setup is suited to download software trigger cuts
    from the database (local or global) and that you unpacked raw data in your data store (e.g. call the add_unpacker
    function). After this part of the reconstruction is processes, you rather want to store the output, as you can not
    do anything sensible any more (all the information of the reconstruction is lost).

    :param path: The path to which the ST modules will be added.
    :param store_array_debug_prescale: Set to an finite value, to control for how many events the variables should
        be written out to the data store.
    :param components: the detector components
    :param softwaretrigger_mode: softwaretrigger_off: disable all software trigger activity, no reconstruction, no filter
                                 monitoring: enable reconstruction, fast_reco filter is off, hlt filter is off
                                 fast_reco_filter: enable reconstruction, fast_reco filter is on, hlt filter is off
                                 hlt_filter: default mode, enable all software activities
                                             including reconstruction, fast_reco and hlt filters.
    :param additonal_store_arrays_to_keep: StoreArray names which will kept together with the Raw objects after
                                           the HLT processing is complete. The content of the StoreArray will only
                                           be kept if the event is not filtered or the monitoring mode is used.
    :param pruneDataStore: If this is false, none of the reconstruction content will be removed from the datestore
                           after the reconstruction and software trigger is complete. Default is true.
    """
    # In the following, we will need some paths:
    # (1) A "store-metadata" path (deleting everything except the trigger tags and some metadata)
    if pruneDataStore:
        store_only_metadata_path = get_store_only_metadata_path()
    else:
        store_only_metadata_path = basf2.create_path()
    # (3) A path doing the fast reco reconstruction
    fast_reco_reconstruction_path = basf2.create_path()
    # (4) A path doing the hlt reconstruction
    hlt_reconstruction_path = basf2.create_path()
    # (5) A path doing the calibration reconstruction with a "store-all" path, which deletes everything except
    # raw data, trigger tags and the meta data.
    calibration_and_store_only_rawdata_path = basf2.create_path()

    # If softwaretrigger_mode is not any of the trigger mode, force the mode
    # to the default mode 'hlt_filter', and print out a warning
    if softwaretrigger_mode not in ['softwaretrigger_off', 'monitoring', 'fast_reco_filter', 'hlt_filter']:
        basf2.B2WARNING("The trigger mode " + softwaretrigger_mode +
                        " is not a standard software trigger" +
                        "mode [softwaretrigger_off, monitoring, fast_reco_filter, hlt_filter]" +
                        "the default mode hlt_filter is forced to be applied")
        softwaretrigger_mode = 'hlt_filter'

    # Add fast reco reconstruction
    if softwaretrigger_mode in ['monitoring', 'fast_reco_filter', 'hlt_filter']:
        reconstruction.add_reconstruction(fast_reco_reconstruction_path, trigger_mode="fast_reco", skipGeometryAdding=True,
                                          components=components, trackFitHypotheses=trackFitHypotheses)
        # Add fast reco cuts
        fast_reco_cut_module = add_fast_reco_software_trigger(fast_reco_reconstruction_path, store_array_debug_prescale)
        if softwaretrigger_mode in ['fast_reco_filter', 'hlt_filter']:
            # There are three possibilities for the output of this module
            # (1) the event is dismissed -> only store the metadata
            fast_reco_cut_module.if_value("==0", store_only_metadata_path, basf2.AfterConditionPath.CONTINUE)
            # (2) the event is accepted -> go on with the hlt reconstruction
            fast_reco_cut_module.if_value("==1", hlt_reconstruction_path, basf2.AfterConditionPath.CONTINUE)

        elif softwaretrigger_mode == 'monitoring':
            fast_reco_reconstruction_path.add_path(hlt_reconstruction_path)

        # Add hlt reconstruction
        reconstruction.add_reconstruction(hlt_reconstruction_path, trigger_mode="hlt", skipGeometryAdding=True,
                                          components=components, trackFitHypotheses=trackFitHypotheses)

        hlt_cut_module = add_hlt_software_trigger(hlt_reconstruction_path, store_array_debug_prescale)

        # preserve the reconstruction information which is needed for ROI calculation.
        add_calcROIs_software_trigger(calibration_and_store_only_rawdata_path, calcROIs=calcROIs)
        # Fill the calibration_and_store_only_rawdata_path path
        add_calibration_software_trigger(calibration_and_store_only_rawdata_path, store_array_debug_prescale)
        if pruneDataStore:
            calibration_and_store_only_rawdata_path.add_path(get_store_only_rawdata_path(additonal_store_arrays_to_keep))
        if softwaretrigger_mode == 'hlt_filter':
            # There are two possibilities for the output of this module
            # (1) the event is rejected -> only store the metadata
            hlt_cut_module.if_value("==0", store_only_metadata_path, basf2.AfterConditionPath.CONTINUE)
            # (2) the event is accepted -> go on with the calibration
            hlt_cut_module.if_value("==1", calibration_and_store_only_rawdata_path, basf2.AfterConditionPath.CONTINUE)
        elif softwaretrigger_mode in ['monitoring', 'fast_reco_filter']:
            hlt_reconstruction_path.add_path(calibration_and_store_only_rawdata_path)

        # currently, dqm plots are only shown for event accepted by the HLT filters
        add_hlt_dqm(hlt_reconstruction_path, run_type, components=components)

    elif softwaretrigger_mode == 'softwaretrigger_off':
        # make sure to still add the DQM modules, they can give at least some FW runtime info
        # and some unpacked hit information
        add_hlt_dqm(path, run_type, components=components)
        if pruneDataStore:
            fast_reco_reconstruction_path.add_module(
                "PruneDataStore",
                matchEntries=ALWAYS_SAVE_REGEX +
                RAW_SAVE_STORE_ARRAYS +
                additonal_store_arrays_to_keep)

    path.add_path(fast_reco_reconstruction_path)


def add_online_dqm(path, run_type, dqm_environment, components=None):
    """
    Add DQM plots for a specific run type and dqm environment
    """
    if run_type == "collision":
        add_collision_dqm(path, components=components, dqm_environment=dqm_environment)
    elif run_type == "cosmics":
        add_cosmic_dqm(path, components=components, dqm_environment=dqm_environment)
    else:
        basf2.B2FATAL("Run type {} not supported.".format(run_type))


def add_hlt_dqm(path, run_type, standalone=False, components=None):
    """
    Add all the DQM modules for HLT to the path
    """
    if standalone:
        add_geometry_if_not_present(path)

    add_online_dqm(path, run_type, "hlt", components)


def add_expressreco_dqm(path, run_type, standalone=False, components=None):
    """
    Add all the DQM modules for ExpressReco to the path
    """
    if standalone:
        add_geometry_if_not_present(path)

    add_online_dqm(path, run_type, "expressreco", components)


def add_geometry_if_not_present(path):
    # geometry parameter database
    if 'Gearbox' not in path:
        path.add_module('Gearbox')

    # detector geometry
    if 'Geometry' not in path:
        path.add_module('Geometry', useDB=True)


def get_store_only_metadata_path():
    """
    Helper function to create a path which deletes (prunes) everything from the data store except
    things that are really needed, e.g. the event meta data and the results of the software trigger module.

    After this path was processed, you can not use the data store content any more to do reconstruction (because
    it is more or less empty), but can only output it to a (S)ROOT file.
    :return: The created path.
    """
    store_metadata_path = basf2.create_path()
    store_metadata_path.add_module("PruneDataStore", matchEntries=ALWAYS_SAVE_REGEX). \
        set_name("KeepMetaData")

    return store_metadata_path


def get_store_only_rawdata_path(additonal_store_arrays_to_keep=[]):
    """
    Helper function to create a path which deletes (prunes) everything from the data store except
    raw objects from the detector and things that are really needed, e.g. the event meta data and the results of the
    software trigger module.

    After this path was processed, you can not use the data store content any more to do reconstruction (because
    it is more or less empty), but can only output it to a (S)ROOT file.
    :return: The created path.
    """
    store_rawdata_path = basf2.create_path()
    store_rawdata_path.add_module(
        "PruneDataStore",
        matchEntries=ALWAYS_SAVE_REGEX +
        RAW_SAVE_STORE_ARRAYS +
        additonal_store_arrays_to_keep) .set_name("KeepRawData")

    return store_rawdata_path


def setup_softwaretrigger_database_access(software_trigger_global_tag_name=SOFTWARE_TRIGGER_GLOBAL_TAG_NAME,
                                          production_global_tag_name="development"):
    """
    Helper function to set up the database chain, needed for typical software trigger applications. This chains
    consists of:
    * access to the local database store in localdb/database.txt in the current folder.
    * global database access with the given software trigger global tag (probably the default one).
    * global database access with the "development" tag, which is the standard global database.

    :param software_trigger_global_tag_name: controls the name of the software trigger global tag in the database.
    :param production_global_tag_name: controls the name of the general global tag in the database.
    """
    basf2.reset_database()
    basf2.use_database_chain()
    basf2.use_local_database("localdb/dbcache.txt")
    basf2.use_central_database(software_trigger_global_tag_name)
    basf2.use_central_database(production_global_tag_name)
