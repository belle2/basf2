import basf2
import generators
from softwaretrigger.path_functions import add_hlt_processing


path = basf2.create_path()

basf2.set_random_seed(12345)

# specify number of events to be generated
path.add_module('EventInfoSetter', evtNumList=[1], runList=[1], expList=[1])
path.add_module("HistoManager", histoFileName="RemoveMePlease.root")

# generate BBbar events
# path.add_module('EvtGenInput')
generators.add_continuum_generator(path, finalstate="ccbar")

# no reconstruction or software trigger added at all
add_hlt_processing(path, run_type="cosmics", softwaretrigger_mode="monitor")

finalize_hlt_path(path)
basf2.print_path(path)
basf2.process(path)
