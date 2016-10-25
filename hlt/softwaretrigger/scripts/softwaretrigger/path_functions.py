import basf2
from softwaretrigger import (
    SOFTWARE_TRIGGER_GLOBAL_TAG_NAME
)

import rawdata
import reconstruction
from hlt.softwaretrigger.scripts.softwaretrigger import add_fast_reco_software_trigger, add_hlt_software_trigger, \
    add_calibration_software_trigger

RAW_SAVE_STORE_ARRAYS = ["RawCDCs", "RawSVDs", "RawTOPs", "RawARICHs", "RawKLMs", "RawECLs",
                         "EKLMDigits"]
ALWAYS_SAVE_REGEX = ["EventMetaData", "SoftwareTrigger.*"]


def add_packers(path):
    """
    Slightly modified version of rawdata.add_packers to not include the packer for PXD and add the Geometry/Gearbox
     when needed.
    :param path: The path to which the packers will be added.
    """
    # Add Gearbox or geometry to path if not already there
    if "Gearbox" not in path:
        path.add_module("Gearbox")

    if "Geometry" not in path:
        path.add_module("Geometry")

    # Exclude PXD
    rawdata.add_packers(path, components=["SVD", "CDC", "ECL", "TOP", "ARICH", "BKLM", "EKLM"])

    # TODO: EKLMPacker
    basf2.B2WARNING("The EKLM raw data is not handled properly, because of missing (Un)packer modules!")


def add_unpackers(path):
    """
    Slightly modified version of rawdata.add_unpackers to not include the unpacker for PXD and add the Geometry/Gearbox
     when needed.
    :param path: The path to which the unpackers will be added.
    """
    # Add Gearbox or geometry to path if not already there
    if "Gearbox" not in path:
        path.add_module("Gearbox")

    if "Geometry" not in path:
        path.add_module("Geometry")

    # Exclude PXD
    rawdata.add_unpackers(path, components=["SVD", "CDC", "ECL", "TOP", "ARICH", "BKLM", "EKLM"])

    # TODO: EKLMUnpacker
    basf2.B2WARNING("The EKLM raw data is not handled properly, because of missing (Un)packer modules!")

    # add clusterizer
    path.add_module("SVDClusterizer")


def add_softwaretrigger_reconstruction(path, store_array_debug_prescale=None):
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
    do anything sensible eny more (all the information of the reconstruction is lost).

    :param path: The path to which the ST modules will be added.
    :param store_array_debug_prescale: Set to an finite value, to control for how many events the variables should
        be written out to the data store.
    """
    # In the following, we will need some paths:
    # (1) A "store-metadata" path (deleting everything except the trigger tags and some metadata)
    store_only_metadata_path = get_store_only_metadata_path()
    # (3) A path doing the fast reco reconstruction
    fast_reco_reconstruction_path = basf2.create_path()
    # (4) A path doing the hlt reconstruction
    hlt_reconstruction_path = basf2.create_path()
    # (5) A path doing the calibration reconstruction with a "store-all" path, which deletes everything except
    # raw data, trigger tags and the meta data.
    calibration_and_store_only_rawdata_path = basf2.create_path()

    # Add fast reco reconstruction
    reconstruction.add_reconstruction(fast_reco_reconstruction_path, trigger_mode="fast_reco", skipGeometryAdding=True)

    # Add fast reco cuts
    fast_reco_cut_module = add_fast_reco_software_trigger(fast_reco_reconstruction_path, store_array_debug_prescale)
    # There are three possibilities for the output of this module
    # (1) the event is dismissed -> only store the metadata
    fast_reco_cut_module.if_value("==-1", store_only_metadata_path, basf2.AfterConditionPath.CONTINUE)
    # (2) we do not know what to do or the event is accepted -> go on with the hlt reconstruction
    fast_reco_cut_module.if_value("!=-1", hlt_reconstruction_path, basf2.AfterConditionPath.CONTINUE)

    # second possibility
    # # (2) the event is immediately accepted -> store everything
    # fast_reco_cut_module.if_value("==1", store_only_rawdata_path, basf2.AfterConditionPath.CONTINUE)
    # # (3) we do not know what to do -> go on with the reconstruction
    # fast_reco_cut_module.if_value("==0", hlt_reconstruction_path, basf2.AfterConditionPath.CONTINUE)

    # Add hlt reconstruction
    reconstruction.add_reconstruction(hlt_reconstruction_path, trigger_mode="hlt", skipGeometryAdding=True)
    hlt_cut_module = add_hlt_software_trigger(hlt_reconstruction_path, store_array_debug_prescale)

    # Fill the calibration_and_store_only_rawdata_path path
    add_calibration_software_trigger(calibration_and_store_only_rawdata_path, store_array_debug_prescale)
    calibration_and_store_only_rawdata_path.add_path(get_store_only_rawdata_path())

    # There are two possibilities for the output of this module
    # (1) the event is accepted -> store everything
    hlt_cut_module.if_value("==1", calibration_and_store_only_rawdata_path, basf2.AfterConditionPath.CONTINUE)
    # (2) we do not know what to do or the event is rejected -> only store the metadata
    hlt_cut_module.if_value("!=1", store_only_metadata_path, basf2.AfterConditionPath.CONTINUE)

    path.add_path(fast_reco_reconstruction_path)


def get_store_only_metadata_path():
    """
    Helper function to create a path which deletes (prunes) everything from the data store except
    things that are really needed, e.g. the event meta data and the results of the software trigger module.

    After this path was processed, you can not use the data store content any more to do reconstruction (because
    it is more or less empty), but can only output it to a (S)ROOT file.
    :return: The created path.
    """
    store_metadata_path = basf2.create_path()
    store_metadata_path.add_module("PruneDataStore", keepEntries=ALWAYS_SAVE_REGEX). \
        set_name("KeepMetaData")

    return store_metadata_path


def get_store_only_rawdata_path():
    """
    Helper function to create a path which deletes (prunes) everything from the data store except
    raw objects from the detector and things that are really needed, e.g. the event meta data and the results of the
    software trigger module.

    After this path was processed, you can not use the data store content any more to do reconstruction (because
    it is more or less empty), but can only output it to a (S)ROOT file.
    :return: The created path.
    """
    store_rawdata_path = basf2.create_path()
    store_rawdata_path.add_module("PruneDataStore", keepEntries=ALWAYS_SAVE_REGEX + RAW_SAVE_STORE_ARRAYS) \
        .set_name("KeepRawData")

    return store_rawdata_path


def setup_softwaretrigger_database_access(software_trigger_global_tag_name=SOFTWARE_TRIGGER_GLOBAL_TAG_NAME,
                                          production_global_tag_name="production"):
    """
    Helper function to set up the database chain, needed for typical software trigger applications. This chains
    consists of:
    * access to the local database store in localdb/database.txt in the current folder.
    * global database access with the given software trigger global tag (probably the default one).
    * global database access with the "production" tag, which is the standard global database.

    :param software_trigger_global_tag_name: controls the name of the software trigger global tag in the database.
    :param production_global_tag_name: controls the name of the general global tag in the database.
    """
    basf2.reset_database()
    basf2.use_database_chain()
    basf2.use_local_database()
    basf2.use_central_database(software_trigger_global_tag_name)
    basf2.use_central_database(production_global_tag_name)
