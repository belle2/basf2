
import os
import tempfile
import basf2
import generators
import shutil
from simulation import add_simulation

from softwaretrigger.path_functions import get_store_only_rawdata_path, DEFAULT_HLT_COMPONENTS, DEFAULT_EXPRESSRECO_COMPONENTS
from rawdata import add_packers

import ROOT
import basf2


class CheckForSoftwareTriggerResult(basf2.Module):
    """test"""

    def __init__(self, has_softwaretriggerresult):
        """
        inut function
        @param has_softwaretriggerresult: if true, data store will
        be checked for a filled SoftwareTriggerResult object
        """
        super().__init__()
        #: Copy of the has_softwaretriggerresult input variable.
        self.has_softwaretriggerresult = has_softwaretriggerresult

    def event(self):
        """reimplementation of Module::event()."""
        sft_trigger = ROOT.Belle2.PyStoreObj("SoftwareTriggerResult")

        if self.has_softwaretriggerresult:
            if not sft_trigger.isValid():
                basf2.B2FATAL("SoftwareTriggerResult object not created")
            else:
                if len(sft_trigger.getResults()) == 0:
                    basf2.B2FATAL("SoftwareTriggerResult exists but has no entries")


def create_test_path(runtype="collision", location="hlt", expNum=0):
    """
    Create the first half of the path to test HLT/ExpressReco scripts
    """
    basf2.set_random_seed(12345)

    tempfolder = tempfile.mkdtemp()
    path = basf2.create_path()

    # specify number of events to be generated
    path.add_module('EventInfoSetter', evtNumList=[1], expList=[expNum])
    path.add_module("HistoManager", histoFileName=os.path.join(tempfolder, "hlt_steering_file_test.root"))

    if runtype == "collision":
        generators.add_continuum_generator(path, finalstate="uubar")
    elif runtype == "cosmics":
        generators.add_cosmics_generator(path)

    additonal_store_arrays_to_keep = []
    add_simulation(path, usePXDDataReduction=False)

    if location == "hlt":
        components = DEFAULT_HLT_COMPONENTS
    elif location == "expressreco":
        additonal_store_arrays_to_keep.append("RawPXDs")
        components = DEFAULT_EXPRESSRECO_COMPONENTS
    else:
        basf2.B2FATAL("Location {} for test is not supported".format(location))

    add_packers(path, components=components)

    # remove everything but HLT input raw objects
    path.add_path(get_store_only_rawdata_path(additonal_store_arrays_to_keep=additonal_store_arrays_to_keep))

    return (path, tempfolder)


def finalize_test_path(path, tempfolder, has_softwaretriggerresult=True):
    """
    Create the second half of the path to test HLT/ExpressReco scripts
    """

    path.add_module("PrintCollections", printForEvent=0)

    path.add_module(CheckForSoftwareTriggerResult(has_softwaretriggerresult))

    basf2.print_path(path)
    basf2.process(path)
    shutil.rmtree(tempfolder)
