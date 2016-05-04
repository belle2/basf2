import glob
from basf2 import *

# create a path with all modules needed before calibration path is run.
main = create_path()

input_data_file_path = 'input_data_files.data'

import os
import sys

if os.path.exists(input_data_file_path):
    import pickle
    with open(input_data_file_path, 'br') as input_data_file:
        inputfiles = pickle.load(input_data_file)
else:
    B2ERROR("No input data pickle file could be found: {0}".format(input_data_file))
    sys.exit(1)

main.add_module('RootInput', excludeBranchNames=['GF2TracksToMCParticles', 'TrackCandsToGF2Tracks', 'GF2Tracks'],
                inputFileNames=inputfiles)
main.add_module('Gearbox')

magnet = False
if magnet:
    print('Using magnetic field')
    main.add_module('Geometry', components=['BeamPipe', 'MagneticFieldConstant4LimitedRCDC', 'PXD', 'SVD'])
else:
    print('Using NO magnetic field')
    main.add_module('Geometry', components=['BeamPipe', 'PXD', 'SVD'])

main.add_module('SetupGenfitExtrapolation', noiseBetheBloch=False, noiseCoulomb=False, noiseBrems=False)
main.add_module('GBLfit')

pickle_paths = glob.glob('*.pickle')

if pickle_paths:
    for pickle_path in pickle_paths:
        main.add_path(get_path_from_file(pickle_path))
else:
    B2FATAL("Couldn't find any pickle files for basf2 paths!")

main.add_module('RootOutput', branchNames=['EventMetaData'])

process(main)
