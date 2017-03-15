import basf2

from simulation import add_simulation
import os


from softwaretrigger.path_functions import (
    setup_softwaretrigger_database_access,
    add_packers,
    add_unpackers,
    add_softwaretrigger_reconstruction,
    RAW_SAVE_STORE_ARRAYS,
    DEFAULT_HLT_COMPONENTS,
)

if __name__ == '__main__':
    # Create a path to generate some raw-data samples and then use the software trigger path(s) to reconstruct them.
    setup_softwaretrigger_database_access()

    sroot_file_name = "generated_events_raw.sroot"

    # You could use your own components here or just use the default for the HLT (everything except PXD)
    # e.g. without SVD
    #     components = ["CDC", "ECL", "TOP", "ARICH", "BKLM", "EKLM"]
    # if you leave out the components in all calls, the default will be used
    components = DEFAULT_HLT_COMPONENTS

    main_path = basf2.create_path()

    if not os.path.exists(sroot_file_name):
        basf2.B2WARNING("No input file found! We will generate one. You have to call this script again afterwards, "
                        "to do the construction.")

        main_path.add_module("EventInfoSetter", evtNumList=[10])
        main_path.add_module("EvtGenInput")

        add_simulation(main_path)
        add_packers(main_path, components=components)

        main_path.add_module("SeqRootOutput", outputFileName=sroot_file_name,
                             saveObjs=["EventMetaData"] + RAW_SAVE_STORE_ARRAYS)
    else:
        main_path.add_module("SeqRootInput", inputFileName=sroot_file_name)

        add_unpackers(main_path, components=components)
        add_softwaretrigger_reconstruction(main_path, store_array_debug_prescale=1, components=components)

        main_path.add_module("RootOutput", outputFileName="output.root")

    basf2.print_path(main_path)
    basf2.process(main_path)

    print(basf2.statistics)
