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

main_path = create_path()

mctrackfinder = register_module('TrackFinderMCTruthRecoTracks')
mctrackfinder.param('WhichParticles', ['SVD'])
mctrackfinder.param('EnergyCut', 0)

gearbox = register_module('Gearbox')

geometry = register_module('Geometry')

rootinput = register_module('RootInput')

progressbar = register_module('ProgressBar')

mctrackmatcher = register_module('MCRecoTracksMatcher')
mctrackmatcher.param('UseCDCHits', False)
mctrackmatcher.param('UseSVDHits', True)
mctrackmatcher.param('UsePXDHits', False)
mctrackmatcher.param('mcRecoTracksStoreArrayName', 'MCRecoTracks')

NoKickCuts = register_module('NoKickCutsEval')


main_path.add_module(rootinput)
main_path.add_module(gearbox)
main_path.add_module(geometry)
main_path.add_module(mctrackfinder)
main_path.add_module(NoKickCuts)
main_path.add_module(progressbar)

process(main_path)
print(statistics)
