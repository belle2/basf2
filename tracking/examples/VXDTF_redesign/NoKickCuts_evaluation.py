#####################################################################
# NoKickCuts Evaluation
#
# This steering file produce a TFile (NoKickCuts.root) that contains 6 TH3F
# needed for NoKickRTSel module inside Training Sample Preparation.
# A default version of NoKickCuts.root is already present in tracking/data,
# so it is not necessary to reproduce it to run the training with the selection
# procedure enabled.
#
# usage: basf2 NoKickCuts_evaluation.py -i inputfile.root -- (--useValidation) (--useFitMethod)
#
# the inputfile is the same provided by eventSimulation.py script, otherwise a
# default pre-simulated inputfile is provided in this script if -i is omitted.
# the validation attach to the NoKickCuts.root file some useful histograms to
# validate the selection, the FitMethod istead is not recommended because it has
# not been optimized for Belle II Phase III Training Sample.
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
# param("inputFileNames", "/home/belle2/vberta/storage/release1_validation/training_sample/simulated*.root")

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
