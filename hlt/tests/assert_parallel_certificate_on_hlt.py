##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Test for the parallel certified flag in all modules in the default, hlt and expressreco
# reconstruction chains.
# It creates a path and fills it with all reconstruction modules, without anything else
# (this path can never be executed, but we do not want that anyway).
# Then it goes through all modules and checks it flag.

import basf2
import reconstruction
from softwaretrigger.processing import add_hlt_processing, add_expressreco_processing

from softwaretrigger.constants import SoftwareTriggerModes, RunTypes


def test_path(path):
    # Assert that all modules have a parallel processing certified flag.
    modules = path.modules()
    for m in modules:
        if m.name() == "HistoManager":
            continue
        assert m.has_properties(basf2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED), \
            f'{m} is missing c_ParallelProcessingCertified flag!'

        for sub_path in m.get_all_condition_paths():
            test_path(sub_path)


if __name__ == "__main__":

    # Add only the reconstruction
    path = basf2.create_path()
    reconstruction.add_reconstruction(path)
    test_path(path)

    # Add various modes of HLT
    for trigger in SoftwareTriggerModes:
        path = basf2.create_path()
        path.add_module("HistoManager")
        add_hlt_processing(path, run_type=RunTypes.beam, softwaretrigger_mode=trigger)
        test_path(path)

    path = basf2.create_path()
    path.add_module("HistoManager")
    add_hlt_processing(path, run_type=RunTypes.cosmic, softwaretrigger_mode=SoftwareTriggerModes.monitor,
                       data_taking_period="phase3")
    test_path(path)

    # Add various modes of express reco
    for do_reconstruction in [True, False]:
        path = basf2.create_path()
        path.add_module("HistoManager")
        add_expressreco_processing(path, run_type=RunTypes.beam, do_reconstruction=do_reconstruction)
        test_path(path)

    for do_reconstruction in [True, False]:
        path = basf2.create_path()
        path.add_module("HistoManager")
        add_expressreco_processing(path, run_type=RunTypes.cosmic, do_reconstruction=do_reconstruction,
                                   data_taking_period="phase3")
        test_path(path)
