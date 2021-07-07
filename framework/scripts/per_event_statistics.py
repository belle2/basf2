##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import basf2
import ROOT
from ROOT import Belle2
import numpy as np

# circumvent BII-1264
ROOT.gROOT.ProcessLine("#include <framework/utilities/MakeROOTCompatible.h>")


class PerEventStatisticsGetterModule(basf2.Module):
    """
    A basf2 python module to export *all* module time statistics (of every event, not just averaged)
     into a ROOT TTree written to a file.
    """

    def __init__(self, output_file_name):
        """
        Create a new PerEventStatisticsGetterModule. You have to give the name of the
        file, where the TTree with the full event statistics will be saved.
        """
        super().__init__()

        #: Name of the output file.
        self.output_file_name = output_file_name
        #: Will host the pointer to the opened TFile later.
        self.tfile = None
        #: Will host the TTree later.
        self.statistics = None

        #: The columns to store the event number.
        self.event_number = np.zeros(3, dtype=float)
        #: The columns for the statistics TTree (they will be filled in the event function).
        self.ttree_inputs = None
        #: Last recored sum of event calls for all modules
        self.last_time_sum = None

        #: A flag to indicate that we have already added the Branches to the TTree (which we will do in the first event)
        self.branches_added = False

        #: The event meta data
        self.event_meta_data = Belle2.PyStoreObj("EventMetaData")

    def initialize(self):
        """
        Create the needed store object pointer in the DataStore and the TFile with the TTree.
        """
        self.event_meta_data.isRequired()

        # try to avoid side effects ...
        old = ROOT.gDirectory
        # Before creating the TTree, open the output file.
        self.tfile = ROOT.TFile.Open(self.output_file_name, "RECREATE")
        # Create the TTree.
        self.statistics = ROOT.TTree("statistics", "Per event execution Statistics for all modules")
        # and reset stupid global directory pointer
        ROOT.gDirectory = old

        # Add branches identifying the event. The reason these are [i:] instead
        # of just [i] is that the latter just returns a single float object which
        # doesn't work as a buffer to TTree::Branch. We need a buffer with an
        # address where it can write the value. So we take a reference to the
        # array starting at element i with [i:]
        self.statistics.Branch("expNo", self.event_number[0:], "expNo/D")
        self.statistics.Branch("runNo", self.event_number[1:], "runNo/D")
        self.statistics.Branch("evtNo", self.event_number[2:], "evtNo/D")

    def event(self):
        """
        The event loop: Store the statistics as a new row in the TTree.
        """
        # TTree reference (for shorter code)
        ttree = self.statistics

        # Get the statistics
        module_stats = basf2.statistics.modules

        # Fill in the event information
        self.event_number[0] = self.event_meta_data.getExperiment()
        self.event_number[1] = self.event_meta_data.getRun()
        self.event_number[2] = self.event_meta_data.getEvent()

        # Again, change the directory
        old = ROOT.gDirectory
        self.tfile.cd()

        if not self.branches_added:

            # make sure all this is only done in the output process
            if Belle2.ProcHandler.parallelProcessingUsed() and not Belle2.ProcHandler.isOutputProcess():
                basf2.B2FATAL("PerEventStatisticsGetterModule can only be used in single processing mode or in the output process")

            self.ttree_inputs = np.zeros(len(module_stats), dtype=float)
            self.last_time_sum = np.zeros(len(module_stats), dtype=float)

            for i, stat in enumerate(module_stats):
                # escape the module names in ROOT-safe manner. Otherwise weird stuff happens like
                # sub-branches get created or the branch cannot be opened in the TBrowser
                module_name = "{name}_{i}".format(name=Belle2.makeROOTCompatible(stat.name), i=i)
                # Create the branch. See initialize() for why it's [i:] and not [i]
                ttree.Branch(module_name, self.ttree_inputs[i:], f"{module_name}/D")

            self.branches_added = True

        # Fill in the module statistics into the branches
        time_sum = np.array([m.time_sum(basf2.statistics.EVENT) for m in module_stats], dtype=float)
        # We need to change the contents of the ttree_inputs array **without**
        # changing its address so we cannot just assign. But we can assign to
        # to the full array itself using the empty slice syntax.
        self.ttree_inputs[:] = time_sum - self.last_time_sum
        self.last_time_sum[:] = time_sum

        # Send the branches to the TTree.
        ttree.Fill()

        # and back again
        ROOT.gDirectory = old

    def terminate(self):
        """
        Write out the merged statistics to the ROOT file.
        This should only be called once, as we would end up with different versions otherwise.
        """
        # Change the path a last time
        old = ROOT.gDirectory
        self.tfile.cd()

        self.statistics.Write()
        self.tfile.Close()

        # and back again
        ROOT.gDirectory = old
