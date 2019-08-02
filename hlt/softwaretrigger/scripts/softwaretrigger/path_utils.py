import basf2
from softwaretrigger import constants
import modularAnalysis

import reconstruction
from tracking import add_cdc_monopole_track_finding


def add_online_dqm(path, run_type, dqm_environment, components, dqm_mode):
    """
    Add DQM plots for a specific run type and dqm environment
    """

    # Local imports, as the dqm package is not checked out by default
    from daqdqm.collisiondqm import add_collision_dqm
    from daqdqm.cosmicdqm import add_cosmic_dqm

    if run_type == constants.RunTypes.beam:
        add_collision_dqm(path, components=components, dqm_environment=dqm_environment, dqm_mode=dqm_mode)
    elif run_type == constants.RunTypes.cosmic:
        add_cosmic_dqm(path, components=components, dqm_environment=dqm_environment, dqm_mode=dqm_mode)
    else:
        basf2.B2FATAL("Run type {} not supported.".format(run_type))

    if dqm_mode in ["dont_care", "all_events"]:
        path.add_module('DelayDQM', title=dqm_environment, histogramDirectoryName='DAQ')


def add_hlt_dqm(path, run_type, components, dqm_mode):
    """
    Add all the DQM modules for HLT to the path
    """
    add_online_dqm(
        path,
        run_type=run_type,
        dqm_environment=constants.Location.hlt.name,
        components=components,
        dqm_mode=dqm_mode.name)


def add_expressreco_dqm(path, run_type, components):
    """
    Add all the DQM modules for ExpressReco to the path
    """
    add_online_dqm(path, run_type=run_type, dqm_environment=constants.Location.expressreco.name, components=components,
                   dqm_mode=constants.DQMModes.dont_care.name)


def add_geometry_if_not_present(path):
    """
    Add the geometry and gearbox module if it was not already added to the path
    """
    if 'Gearbox' not in path:
        path.add_module('Gearbox')

    if 'Geometry' not in path:
        path.add_module('Geometry', useDB=True)


def add_store_only_metadata_path(path):
    """
    Helper function to create a path which deletes (prunes) everything from the data store except
    things that are really needed, e.g. the event meta data and the results of the software trigger module.

    After this path was processed, you can not use the data store content any more to do reconstruction (because
    it is more or less empty), but can only output it to a (S)ROOT file.
    """
    path.add_module("PruneDataStore", matchEntries=constants.ALWAYS_SAVE_OBJECTS).set_name("KeepMetaData")


def add_store_only_rawdata_path(path, additonal_store_arrays_to_keep=None):
    """
    Helper function to create a path which deletes (prunes) everything from the data store except
    raw objects from the detector and things that are really needed, e.g. the event meta data and the results of the
    software trigger module.

    After this path was processed, you can not use the data store content any more to do reconstruction (because
    it is more or less empty), but can only output it to a (S)ROOT file.
    """
    entries_to_keep = constants.ALWAYS_SAVE_OBJECTS + constants.RAWDATA_OBJECTS

    if additonal_store_arrays_to_keep:
        entries_to_keep += additonal_store_arrays_to_keep

    path.add_module("PruneDataStore", matchEntries=entries_to_keep).set_name("KeepRawData")


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


def add_filter_reconstruction(path, run_type, components, **kwargs):
    """
    Add everything needed to calculation a filter decision and if possible,
    also do the HLT filtering. This is only possible for beam runs (in the moment).

    Up to now, we add the full reconstruction, but this will change in the future.

    Please note that this function adds the HLT decision, but does not branch
    according to it.
    """
    if run_type == constants.RunTypes.beam:
        reconstruction.add_reconstruction(path, skipGeometryAdding=True, pruneTracks=False,
                                          add_trigger_calculation=False, components=components, **kwargs)

        add_filter_software_trigger(path, store_array_debug_prescale=1)
    elif run_type == constants.RunTypes.cosmic:
        reconstruction.add_cosmics_reconstruction(path, skipGeometryAdding=True, pruneTracks=False,
                                                  components=components, **kwargs)
    else:
        basf2.B2FATAL(f"Run Type {run_type} not supported.")


def add_filter_module(path):
    """
    Add and return a skim module, which has a return value dependent
    on the final HLT decision.
    """
    return path.add_module("TriggerSkim", triggerLines=["software_trigger_cut&all&total_result"])


def add_post_filter_reconstruction(path, run_type, components):
    """
    Add all modules which should run after the HLT decision is taken
    and only on the accepted events.
    Up to now, this only includes the skim part, but this will
    change in the future.
    """
    if run_type == constants.RunTypes.beam:
        add_skim_software_trigger(path, store_array_debug_prescale=1)
    elif run_type == constants.RunTypes.cosmic:
        pass
    else:
        basf2.B2FATAL(f"Run Type {run_type} not supported.")
