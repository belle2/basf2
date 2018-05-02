"""
Script to be called by gridcontrol to reconstruct already simulated events.
"""

import basf2
import os

from softwaretrigger.path_functions import add_softwaretrigger_reconstruction, DEFAULT_HLT_COMPONENTS, \
    RAWDATA_OBJECTS, ALWAYS_SAVE_OBJECTS

from rawdata import add_unpackers, add_packers
from simulation import add_roiFinder


def main():
    """Reconstruct the already generated events and store the results to disk"""
    # Get all parameters for this calculation
    input_file = os.environ.get("input_file")
    output_file = os.environ.get("output_file")
    phase = int(os.environ.get("phase"))
    roi_filter = bool(os.environ.get("roi_filter"))

    print("input_file:", input_file)
    print("output_file:", output_file)
    print("phase:", phase)

    raw_output_file = output_file.replace(".root", "_raw.root")
    raw_output_file = raw_output_file.replace("reconstructed_", "_")

    log_file = output_file.replace(".root", ".log")

    raw_save_store_arrays_without_rois = RAWDATA_OBJECTS + ALWAYS_SAVE_OBJECTS

    # Now start the real basf2 calculation
    path = basf2.create_path()
    path.add_module("RootInput", inputFileName=input_file)
    path.add_module("HistoManager", histoFileName="dqm_out.root")

    add_unpackers(path, components=DEFAULT_HLT_COMPONENTS)

    # Add the ST and also write out all variables connected to it. Also, do not cut, but just write out the variables
    add_softwaretrigger_reconstruction(
        path,
        store_array_debug_prescale=1,
        softwaretrigger_mode="monitoring",
        pruneDataStore=False,
        calcROIs=True)

    if roi_filter:
        # todo: this creates a second, filtered PXD digit list and does not overrive the PXD Digits which are
        # packed one line below
        path.add_module('PXDdigiFilter', PXDDigitsInsideROIName='PXDDigitsInsideROI', ROIidsName='ROIs')

        # add PXD packer and make sure it reads for the StoreArray which has been
        # filtered
        add_packers(path, components=["PXD"])
        for m in path.modules():
            if m.name() == "PXDPacker":
                m.param({"PXDDigitsName": "PXDDigitsInsideROI", "RawPXDsName": "RawPXDsFiltered"})
        raw_save_store_arrays_without_rois.append("RawPXDsFiltered")

    # add PXD packer again which will fill RawPXDs StoreArray with the unfiltered PXD hits
    add_packers(path, components=["PXD"])

    path.add_module("RootOutput", outputFileName=output_file,
                    branchNames=["EventMetaData", "SoftwareTriggerResult", "SoftwareTriggerVariables", "TRGSummary"])

    raw_save_store_arrays_without_rois.pop(raw_save_store_arrays_without_rois.index("ROIs"))

    path.add_module("RootOutput", outputFileName=raw_output_file,
                    branchNames=raw_save_store_arrays_without_rois)

    basf2.log_to_file(log_file)
    basf2.print_path(path)
    basf2.process(path)


if __name__ == "__main__":
    main()
