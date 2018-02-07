import basf2
import generators
from softwaretrigger.path_functions import add_hlt_processing, get_store_only_rawdata_path
from rawdata import add_packers
from simulation import add_simulation


path = basf2.create_path()

basf2.set_random_seed(12345)

# specify number of events to be generated
path.add_module('EventInfoSetter', evtNumList=[1])
path.add_module("HistoManager", histoFileName="RemoveMePlease.root")

generators.add_cosmics_generator(path)
add_simulation(path, usePXDDataReduction=False)
add_packers(path)

# remove everything but HLT input raw objects
path.add_path(get_store_only_rawdata_path())

path.add_module("PrintCollections", printForEvent=0)
# no reconstruction or software trigger added at all
add_hlt_processing(path, run_type="cosmics", softwaretrigger_mode="monitor")

basf2.print_path(path)
basf2.process(path)
