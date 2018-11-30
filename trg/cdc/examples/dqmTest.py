from basf2 import *
from ROOT import Belle2
# use_central_database("data_reprocessing_prod5")
main = create_path()

main.add_module('RootInput',
                inputFileName='')

main.add_module('HistoManager',
                histoFileName='histogram.root')

main.add_module('CDCTriggerDQM')

process(main)
print(statistics)
