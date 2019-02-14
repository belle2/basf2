import basf2

import reconstruction
from softwaretrigger import constants
from softwaretrigger.path_utils import get_store_only_metadata_path, add_filter_software_trigger, \
    add_skim_software_trigger


def add_cosmic_softwaretrigger_reconstruction(path, components, **kwargs):
    # no filtering, don't prune RecoTracks so the Tracking DQM module has access to all hits
    reconstruction.add_cosmics_reconstruction(path, skipGeometryAdding=True,
                                              components=components, pruneTracks=False, **kwargs)
    return path


def add_softwaretrigger_reconstruction(
        path,
        components=constants.DEFAULT_HLT_COMPONENTS,
        softwaretrigger_mode=constants.SoftwareTriggerModes.filter,
        store_array_debug_prescale=0,
        **kwargs):
    """
    Add all modules, conditions and conditional paths to the given path, that are needed for a full
    reconstruction stack in the HLT using the software trigger modules.

    If the trigger is turned on (softwaretrigger_mode = filter), all events that are dismissed
    by the trigger module are removed from the datastore - except the event meta data
    and the software trigger result.
    All accepted events also go through the "skim" path, where additional skimming results are calculated.

    If the trigger is turned off, both trigger and skimming results are calculated but no events are dismissed.

    Before calling this function, make sure that your database setup is suited to download software trigger cuts
    from the database (local or global) and that you unpacked raw data in your data store (e.g. call the add_unpacker
    function) and the geometry is setup.
    After this part of the reconstruction is processes, you rather want to store the output, as you can not
    do anything sensible any more (all the information of the reconstruction is lost).

    :param path: The path to which the ST modules will be added.
    :param store_array_debug_prescale: Set to an finite value, to control for how many events the variables should
        be written out to the data store.
    :param components: the detector components to do reconstruction on
    :param softwaretrigger_mode: which mode (filter/monitor) the trigger should have
    """
    # In the following, we will need some paths:
    # (1) A "store-metadata" path (deleting everything except the trigger tags and some metadata)
    store_only_metadata_path = get_store_only_metadata_path()

    # (2) A path doing the skim reconstruction, which is only executed if the trigger decision is positive
    skim_reconstruction_path = basf2.create_path()
    add_skim_software_trigger(skim_reconstruction_path, store_array_debug_prescale)

    # Add the default reconstruction for now
    reconstruction.add_reconstruction(path, skipGeometryAdding=True, pruneTracks=False, components=components, **kwargs)

    # Add the module doing the filter decision
    hlt_filter_module = add_filter_software_trigger(path, store_array_debug_prescale)

    # Only turn on the filtering (by branching the path) if filtering is turned on
    if softwaretrigger_mode == constants.SoftwareTriggerModes.filter:
        # There are two possibilities for the output of this module
        # (1) the event is dismissed -> only store the metadata
        hlt_filter_module.if_value("==0", store_only_metadata_path, basf2.AfterConditionPath.CONTINUE)
        # (2) the event is accepted -> go on with the hlt reconstruction
        hlt_filter_module.if_value("==1", skim_reconstruction_path, basf2.AfterConditionPath.CONTINUE)
    elif softwaretrigger_mode == constants.SoftwareTriggerModes.monitor:
        path.add_path(skim_reconstruction_path)
    else:
        basf2.B2FATAL(f"The software trigger mode {softwaretrigger_mode} is not supported.")

    return skim_reconstruction_path
