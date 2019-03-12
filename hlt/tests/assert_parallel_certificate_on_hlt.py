# Test for the parallel certified flag in all modules in the default, hlt and expressreco
# reconstruction chains.
# It creates a path and fills it with all reconstruction modules, without anything else
# (this path can never be executed, but we do not want that anyway).
# Then it goes through all modules and checks it flag.
#
#
# Author: Nils Braun

import basf2
import reconstruction
from softwaretrigger.path_functions import add_hlt_processing, add_expressreco_processing


def test_path(path):
    # Assert that all modules have a parallel processing certified flag.
    modules = path.modules()
    for m in modules:
        if m.name() == "HistoManager":
            continue
        assert m.has_properties(
            basf2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED), '%s is missing c_ParallelProcessingCertified flag!' % (m)

        for sub_path in m.get_all_condition_paths():
            test_path(sub_path)


if __name__ == "__main__":

    # Add only the reconstruction
    path = basf2.create_path()
    reconstruction.add_reconstruction(path)
    test_path(path)

    # Add various modes of HLT
    for trigger in ["monitoring", "hlt_filter"]:
        path = basf2.create_path()
        path.add_module("HistoManager")
        add_hlt_processing(path, run_type="collision", softwaretrigger_mode=trigger)
        test_path(path)

    for trigger in ["monitoring", "hlt_filter"]:
        path = basf2.create_path()
        path.add_module("HistoManager")
        add_hlt_processing(path, run_type="cosmics", softwaretrigger_mode=trigger, data_taking_period="phase3")
        test_path(path)

    # Add various modes of express reco
    for do_reconstruction in [True, False]:
        path = basf2.create_path()
        path.add_module("HistoManager")
        add_expressreco_processing(path, run_type="collision", do_reconstruction=do_reconstruction)
        test_path(path)

    for do_reconstruction in [True, False]:
        path = basf2.create_path()
        path.add_module("HistoManager")
        add_expressreco_processing(path, run_type="cosmics", do_reconstruction=do_reconstruction, data_taking_period="phase3")
        test_path(path)
