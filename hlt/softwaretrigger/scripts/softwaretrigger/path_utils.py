import basf2
from softwaretrigger import constants
import modularAnalysis

from daqdqm.collisiondqm import add_collision_dqm
from daqdqm.cosmicdqm import add_cosmic_dqm

from rawdata import add_unpackers
from tracking import add_cdc_monopole_track_finding


def add_online_dqm(path, run_type, dqm_environment, components):
    """
    Add DQM plots for a specific run type and dqm environment
    """
    if run_type == constants.RunTypes.beam:
        add_collision_dqm(path, components=components, dqm_environment=dqm_environment)
    elif run_type == constants.RunTypes.cosmic:
        add_cosmic_dqm(path, components=components, dqm_environment=dqm_environment)
    else:
        basf2.B2FATAL("Run type {} not supported.".format(run_type))
    path.add_module('DelayDQM', title=dqm_environment, histogramDirectoryName='DAQ')


def add_hlt_dqm(path, run_type, components=constants.DEFAULT_HLT_COMPONENTS, standalone=False):
    """
    Add all the DQM modules for HLT to the path
    """
    if standalone:
        add_geometry_if_not_present(path)
        add_unpackers(path, components=components)

    add_online_dqm(path, run_type=run_type, dqm_environment=constants.Location.hlt, components=components)


def add_expressreco_dqm(path, run_type, components=constants.DEFAULT_EXPRESSRECO_COMPONENTS, standalone=False):
    """
    Add all the DQM modules for ExpressReco to the path
    """
    if standalone:
        add_geometry_if_not_present(path)
        add_unpackers(path, components=components)

    add_online_dqm(path, run_type=run_type, dqm_environment=constants.Location.expressreco, components=components)


def add_geometry_if_not_present(path):
    if 'Gearbox' not in path:
        path.add_module('Gearbox')

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
    store_metadata_path.add_module("PruneDataStore", matchEntries=constants.ALWAYS_SAVE_OBJECTS).set_name("KeepMetaData")

    return store_metadata_path


def get_store_only_rawdata_path(additonal_store_arrays_to_keep=None):
    """
    Helper function to create a path which deletes (prunes) everything from the data store except
    raw objects from the detector and things that are really needed, e.g. the event meta data and the results of the
    software trigger module.

    After this path was processed, you can not use the data store content any more to do reconstruction (because
    it is more or less empty), but can only output it to a (S)ROOT file.
    :return: The created path.
    """
    entries_to_keep = constants.ALWAYS_SAVE_OBJECTS + constants.RAWDATA_OBJECTS

    if additonal_store_arrays_to_keep:
        entries_to_keep += additonal_store_arrays_to_keep

    store_rawdata_path = basf2.create_path()
    store_rawdata_path.add_module("PruneDataStore", matchEntries=entries_to_keep).set_name("KeepRawData")

    return store_rawdata_path


def add_filter_software_trigger(path, store_array_debug_prescale=0):
    """
    Add the SoftwareTrigger for the filter cuts to the given path.

    Only the calculation of the cuts is implemented here - the cut logic has to be done
    using the module return value.

    :param path: The path to which the module should be added.
    :param store_array_debug_prescale: When not 0, store each N events the content of the variables needed for the
     cut calculations in the data store.
    :return: the software trigger module
    """

    hlt_cut_module = path.add_module("SoftwareTrigger", baseIdentifier="filter",
                                     preScaleStoreDebugOutputToDataStore=store_array_debug_prescale)

    return hlt_cut_module


def add_skim_software_trigger(path, store_array_debug_prescale=0):
    """
    Add the SoftwareTrigger for the skimming (after the filtering) to the given path.

    Only the calculation of the cuts is implemented here - the cut logic has to be done

    :param path: The path to which the module should be added.
    :param store_array_debug_prescale: When not 0, store each N events the content of the variables needed for the
     cut calculations in the data store.
    :return: the software trigger module
    """
    # ECL cluster and track particle lists
    modularAnalysis.fillParticleList("pi+:skim", 'pt>0.2 and abs(d0) < 2 and abs(z0) < 4', path=path)
    modularAnalysis.fillParticleList("gamma:skim", 'E>0.1', path=path)

    # monopoles
    add_cdc_monopole_track_finding(path)

    path.add_module("SoftwareTrigger", baseIdentifier="skim",
                    preScaleStoreDebugOutputToDataStore=store_array_debug_prescale)
