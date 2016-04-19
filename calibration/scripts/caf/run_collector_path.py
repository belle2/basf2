import glob
from basf2 import *

# create a path with all modules needed before calibration path is run.
main = create_path()
main.add_module('EventInfoSetter',
                evtNumList=[50, 50, 200],
                expList=[1, 1, 1],
                runList=[1, 2, 3])

pickle_paths = glob.glob('*.pickle')

if pickle_paths:
    for pickle_path in pickle_paths:
        main.add_path(get_path_from_file(pickle_path))
else:
    B2FATAL("Couldn't find pickle file!")

process(main)
