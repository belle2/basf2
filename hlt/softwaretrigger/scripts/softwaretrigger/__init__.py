from ROOT import Belle2

import basf2
import reconstruction
import modularAnalysis

raw_save_objects = ["EventMetaData", "RawCDCs", "RawSVDs", "RawTOPs", "RawARICHs", "RawKLMs", "RawECLs",
                    "EKLMDigits"]


def add_packers(path):
    # Add Gearbox or geometry to path if not already there
    if "Gearbox" not in path:
        path.add_module("Gearbox")

    if "Geometry" not in path:
        path.add_module("Geometry")

    path.add_module("SVDPacker")
    path.add_module("CDCPacker", xmlMapFileName=Belle2.FileSystem.findFile("data/cdc/ch_map.dat"))
    path.add_module("TOPPacker")
    path.add_module("ARICHPacker")
    path.add_module("BKLMRawPacker")
    path.add_module("ECLPacker", InitFileName=Belle2.FileSystem.findFile("ecl/utility/include/crpsch.dat"),
                    RawCOPPERsName="RawECLs")

    # TODO: EKLMPacker
    basf2.B2WARNING("The EKLM raw data is not handled properly, because of missing (Un)packer modules!")


def add_unpackers(path):
    # Add Gearbox or geometry to path if not already there
    if "Gearbox" not in path:
        path.add_module("Gearbox")

    if "Geometry" not in path:
        path.add_module("Geometry")

    # Add unpackers
    path.add_module("SVDUnpacker", shutUpFTBError=0)
    path.add_module("CDCUnpacker",
                    xmlMapFileName=Belle2.FileSystem.findFile("data/cdc/ch_map.dat"),
                    enablePrintOut=False)
    path.add_module("TOPUnpacker")
    path.add_module("ARICHUnpacker")
    path.add_module("BKLMUnpacker")
    path.add_module("ECLUnpacker", InitFileName=Belle2.FileSystem.findFile("ecl/utility/include/crpsch.dat"))

    # TODO: EKLMUnpacker
    basf2.B2WARNING("The EKLM raw data is not handled properly, because of missing (Un)packer modules!")

    # add clusterizer
    path.add_module("SVDClusterizer")


def add_softwaretrigger_reconstruction(path, store_array_debug_prescale=None):
    # In the following, we will need some paths:
    # (1) A "store-metadata" path (deleting everything except the trigger tags and some metadata)
    store_only_metadata_path = get_store_only_metadata_path()
    # (2) A "store-all" path (delete everything except the raw data with the metadata)
    store_only_rawdata_path = get_store_only_rawdata_path()
    # (3) A path doing the fast reco reconstruction
    fast_reco_reconstruction_path = basf2.create_path()
    # (4) A path doing the hlt reconstruction
    hlt_reconstruction_path = basf2.create_path()

    # Add fast reco reconstruction
    reconstruction.add_reconstruction(fast_reco_reconstruction_path, trigger_mode="fast_reco", skipGeometryAdding=True)

    # Add fast reco cuts
    fast_reco_cut_module = fast_reco_reconstruction_path.add_module("SoftwareTrigger", baseIdentifier="fast_reco",
                                                                    cutIdentifiers=[])

    if store_array_debug_prescale is not None:
        fast_reco_cut_module.param("preScaleStoreDebugOutputToDataStore", store_array_debug_prescale)

    # There are three possibilities for the output of this module
    # (1) the event is dismissed -> only store the metadata
    fast_reco_cut_module.if_value("==-1", store_only_metadata_path, basf2.AfterConditionPath.CONTINUE)
    # (2) the event is immediately accepted -> store everything
    fast_reco_cut_module.if_value("==1", store_only_rawdata_path, basf2.AfterConditionPath.CONTINUE)
    # (3) we do not know what to do -> go on with the reconstruction
    fast_reco_cut_module.if_value("==0", hlt_reconstruction_path, basf2.AfterConditionPath.CONTINUE)

    # Add hlt reconstruction
    reconstruction.add_reconstruction(hlt_reconstruction_path, trigger_mode="hlt", skipGeometryAdding=True)
    modularAnalysis.fillParticleList("pi+:HLT", 'pt>0.2', path=hlt_reconstruction_path)
    modularAnalysis.fillParticleList("gamma:HLT", 'E>0.1', path=hlt_reconstruction_path)

    # Add fast reco cuts
    hlt_cut_module = hlt_reconstruction_path.add_module("SoftwareTrigger", baseIdentifier="hlt",
                                                        # TODO
                                                        cutIdentifiers=[])

    if store_array_debug_prescale is not None:
        hlt_cut_module.param("preScaleStoreDebugOutputToDataStore", store_array_debug_prescale)

    # There are two possibilities for the output of this module
    # (1) the event is accepted -> store everything
    hlt_cut_module.if_value("==1", store_only_rawdata_path, basf2.AfterConditionPath.CONTINUE)
    # (2) we do not know what to do or the event is rejected -> only store the metadata
    hlt_cut_module.if_value("!=1", store_only_metadata_path, basf2.AfterConditionPath.CONTINUE)

    path.add_path(fast_reco_reconstruction_path)


def get_store_only_metadata_path():
    store_metadata_path = basf2.create_path()
    store_metadata_path.add_module("PruneDataStore", keepEntries=["EventMetaData", "SoftwareTrigger.*"]).\
        set_name("KeepMetaData")

    return store_metadata_path


def get_store_only_rawdata_path():
    store_rawdata_path = basf2.create_path()
    store_rawdata_path.add_module("PruneDataStore", keepEntries=raw_save_objects).set_name("KeepRawData")

    return store_rawdata_path


def setup_softwaretrigger_database_access(software_trigger_global_tag_name="software_trigger",
                                          production_global_tag_name="production"):
    basf2.reset_database()
    basf2.use_local_database()
    basf2.use_database_chain()
    basf2.use_central_database(software_trigger_global_tag_name)
    basf2.use_central_database(production_global_tag_name)


if __name__ == '__main__':
    from simulation import add_simulation
    import os

    setup_softwaretrigger_database_access()

    sroot_file_name = "generated_events_raw.sroot"

    path = basf2.create_path()

    if not os.path.exists(sroot_file_name):
        basf2.B2WARNING("No input file found! We will generate one. You have to call this script again afterwards, "
                        "to do the construction.")

        path.add_module("EventInfoSetter", evtNumList=[10])
        path.add_module("EvtGenInput")

        add_simulation(path)
        add_packers(path)

        path.add_module("SeqRootOutput", outputFileName=sroot_file_name,
                        saveObjs=raw_save_objects)  # TODO: EKLM is not Raw
    else:
        path.add_module("SeqRootInput", inputFileName=sroot_file_name)

        add_unpackers(path)
        add_softwaretrigger_reconstruction(path, store_array_debug_prescale=1)

        path.add_module("RootOutput", outputFileName="output.root")

    basf2.print_path(path)
    basf2.process(path)

    print(basf2.statistics)
