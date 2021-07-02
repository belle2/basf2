##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
# Test for some modules in the current HLT reconstruction path
#
# Author: Nils Braun
import basf2
from softwaretrigger.processing import add_hlt_processing

from softwaretrigger.constants import SoftwareTriggerModes, RunTypes


def yield_module_names(path):
    for m in path.modules():
        yield m.name()
        for sub_path in m.get_all_condition_paths():
            yield from yield_module_names(sub_path)


if __name__ == "__main__":
    path = basf2.create_path()
    add_hlt_processing(path, run_type=RunTypes.beam, softwaretrigger_mode=SoftwareTriggerModes.filter)

    module_names = list(yield_module_names(path))
    assert "SoftwareTriggerHLTDQM" in module_names
    assert "SoftwareTrigger" in module_names
    assert "Geometry" in module_names
    assert "Gearbox" in module_names
    assert "HistoManager" not in module_names
