#############################################################
#
# Write out dE/dx information for offline calibration tests
#
# Contributors: Jake Bennett
#
# Example steering file - 2017 Belle II Collaboration
#############################################################

from basf2 import *

# main path
main = create_path()

# read in a file in DST format (must contain CDCDedxTracks, Tracks, and TrackFitResults)
input_files = 'B2Electrons.root'
main.add_module('RootInput', inputFileName=input_files)

# write out a flat ROOT file with dE/dx details
main.add_module('HitLevelInfoWriter', outputFileName='electrons.root')

# process the events
process(main)

# show call statistics
print(statistics)
