from ROOT import Belle2

import basf2
import reconstruction


def add_packers(path):
    path.add_module("SVDPacker")
    path.add_module("CDCPacker", xmlMapFileName=Belle2.FileSystem.findFile("data/cdc/ch_map.dat"))
    path.add_module("TOPPacker")
    path.add_module("ARICHPacker")
    path.add_module("BKLMRawPacker")


def add_unpackers(path):
    # Add Gearbox or geometry to path if not already there
    if "Gearbox" not in path.modules():
        path.add_module("Gearbox")

    if "Geometry" not in path.modules():
        path.add_module("Geometry")

    # Add unpackers
    path.add_module("SVDUnpacker", shutUpFTBError=0)
    path.add_module("CDCUnpacker",
                    xmlMapFileName=Belle2.FileSystem.findFile("data/cdc/ch_map.dat"),
                    enablePrintOut=False)
    path.add_module("TOPUnpacker")
    path.add_module("ARICHUnpacker")
    path.add_module("BKLMUnpacker")

    # TODO: ECLUnpacker, EKLMUnpacker

    # add clusterizer
    path.add_module("SVDClusterizer")


def add_softwaretrigger_reconstruction(path):
    # Add fast reco reconstruction
    reconstruction.add_reconstruction(path, trigger_mode="fast_reco")

    # In the following, we will need two paths:
    # (1) A "store-metadata" path (only storing the trigger tags and some metadata)
    store_metadata_path = get_store_metadata_path()
    # (2) A "store-all" path (store the raw data with the metadata), that leads back to the store-metadata path
    store_rawdata_path = get_store_rawdata_path(store_metadata_path)

    # Add fast reco cuts
    fast_reco_cut_module = path.add_module("SoftwareTrigger", baseIdentifier="fast_reco",
                                           cutIdentifiers=["reject_ee", "accept_ee", "reject_bkg"])

    # There are three possibilities for the output of this module
    # (1) the event is dismissed -> only store the metadata
    fast_reco_cut_module.if_value("==-1", store_metadata_path)
    # (2) the event is immediately accepted -> store everything
    fast_reco_cut_module.if_value("==1", store_rawdata_path)
    # (3) we do not know what to do -> go on with the reconstruction

    path.add_module("PrintCollections")


def get_store_metadata_path():
    store_metadata_path = basf2.create_path()
    store_metadata_path.add_module("SeqRootOutput", outputFileName="metadata.sroot",
                                   saveObjs=["EventMetaData", "SoftwareTriggerResult"])

    return store_metadata_path


def get_store_rawdata_path(store_metadata_path):
    store_rawdata_path = basf2.create_path()
    store_rawdata_path.add_module("SeqRootOutput", outputFileName="rawdata.sroot",
                                  saveObjs=["EventMetaData", "SoftwareTriggerResult",
                                            "RawSVDs", "RawCDCs", "RawTOPs", "RawARICHs", "RawKLMs",
                                            "EKLMDigits", "ECLDigits"])

    basf2.B2WARNING("The ECL and EKLM raw data is not handled properly!")

    store_rawdata_path.add_path(store_metadata_path)
    return store_metadata_path


if __name__ == '__main__':
    from simulation import add_simulation
    import os

    basf2.reset_database()
    basf2.use_local_database()
    basf2.use_database_chain()
    basf2.use_central_database("software_trigger_test")
    basf2.use_central_database("production")

    sroot_file_name = "output.sroot"
    if not os.path.exists(sroot_file_name):
        path = basf2.create_path()

        path.add_module("EventInfoSetter", evtNumList=[10])
        path.add_module("Gearbox")
        path.add_module("Geometry")
        path.add_module("EvtGenInput")

        add_simulation(path)
        add_packers(path)

        path.add_module("SeqRootOutput", outputFileName=sroot_file_name,
                        saveObjs=["RawCDCs", "RawSVDs", "RawTOPs", "RawARICHs", "RawKLMs",
                                  "ECLDigits", "EKLMDigits", "EventMetaData"])

        basf2.process(path)

    path = basf2.create_path()

    path.add_module("SeqRootInput", inputFileName=sroot_file_name)
    add_unpackers(path)
    add_softwaretrigger_reconstruction(path)

    basf2.process(path)
