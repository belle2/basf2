#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import set_random_seed, create_path, process, Module
from ROOT import Belle2
from simulation import add_simulation


def main():
    """Main function to be executed if this script is run to avoid running if it's just imported."""
    set_random_seed(12345)

    path = create_path()
    path.add_module("EvtGenInput")
    path.add_module("EventInfoSetter", evtNumList=[5], expList=[0], runList=[0])
    path.add_module("Gearbox")
    path.add_module("Geometry", useDB=False, excludedComponents=['CDC'], additionalComponents=['CDCReducedNoSL0SL1'])

    add_simulation(path, components=['CDC'])

    class testCDCLayers(Module):
        """
        Test that with the reduced CDC no hits on layers 0-13 and superlayers 0-1 are present
        """

        def initialize(self):
            """Initialise the required StoreArrays"""
            self.CDCHits = Belle2.PyStoreArray('CDCHits')
            self.CDCHits.isRequired()

        def event(self):
            """Event loop"""

            for cdcHit in self.CDCHits:
                assert cdcHit.getICLayer() >= 14, "Detected CDCHit in layer < 14 which must not be with the reduced CDC"
                assert cdcHit.getISuperLayer() >= 2, "Detected CDCHit in Superlayer < 2 which must not be with the reduced CDC"

    path.add_module(testCDCLayers())

    path.add_module("Progress")
    process(path, calculateStatistics=True)


if __name__ == "__main__":
    main()
