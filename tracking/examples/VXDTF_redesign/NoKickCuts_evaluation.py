#####################################################################
# NoKickCuts Evaluation
#
# This steering file produce a TFile (NoKickCuts.root) that contains 6 TH3F
# needed for NoKickRTSel module inside Training Sample Preparation.
# A default version of NoKickCuts.root is already present in tracking/data,
# so it is not necessary to reproduce it ro run Training_preparation_NoKick.py
#
# Contributors: Valerio Bertacchi
#####################################################################

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output
from ROOT import Belle2

import argparse
parser = argparse.ArgumentParser(description="NoKickCuts evaluation module")
parser.add_argument(
    '--useValidation',
    dest='use_validation',
    action='store_const',
    const=True,
    default=False,
    help='print validation plots')
parser.add_argument(
    '--useFitMethod',
    dest='use_fitMethod',
    action='store_const',
    const=True,
    default=False,
    help='definition of the cuts with the double-gaussian fit')

args = parser.parse_args()
use_Validation = args.use_validation
use_fit = args.use_fitMethod


main_path = create_path()

mctrackfinder = register_module('TrackFinderMCTruthRecoTracks')
mctrackfinder.param('WhichParticles', ['SVD'])
mctrackfinder.param('EnergyCut', 0)

gearbox = register_module('Gearbox')

geometry = register_module('Geometry')

rootinput = register_module('RootInput')
rootinput.param("inputFileNames", "/group/belle2/users/lueck/data/new/train/simulatedEvents_rndSeed_*.root")

progressbar = register_module('ProgressBar')

mctrackmatcher = register_module('MCRecoTracksMatcher')
mctrackmatcher.param('UseCDCHits', False)
mctrackmatcher.param('UseSVDHits', True)
mctrackmatcher.param('UsePXDHits', True)
mctrackmatcher.param('mcRecoTracksStoreArrayName', 'MCRecoTracks')

NoKickCuts = register_module('NoKickCutsEval')
NoKickCuts.param('useValidation', use_Validation)
NoKickCuts.param('useFitMethod', use_fit)


main_path.add_module(rootinput)
main_path.add_module(gearbox)
main_path.add_module(geometry)
main_path.add_module(mctrackfinder)
main_path.add_module(NoKickCuts)
main_path.add_module(progressbar)

process(main_path)
print(statistics)
