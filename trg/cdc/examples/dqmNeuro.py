import basf2
from ROOT import Belle2
import neurotrigger
import reconstruction
################################################################################
# Setting global tags in case Raw data is unpacked: ###

# basf2.conditions.override_globaltags()
# basf2.conditions.append_globaltag('online')
# basf2.conditions.append_globaltag('neurotrigger') # should not be needed

################################################################################
# Start path: ###
################################################################################
main = basf2.create_path()

# Loading filelist and checking for ending: ###
dstfiles = [sys.argv[1]]
ending = ''
if ".sroot" in dstfiles[0]:
    ending = ".sroot"
else:
    ending = ".root"

print('Files to be processed:')
for x in dstfiles:
    print(x)

# Creating output directories with the script's name: ###
os.makedirs(sys.argv[0].split('.py')[0] + '/data', exist_ok=True)
os.makedirs(sys.argv[0].split('.py')[0] + '/hist', exist_ok=True)

# Creating filenames for dqm histogram output and root dst output: ###
outputfile = sys.argv[0].split('.py')[0] + '/hist/histo.' + sys.argv[1].split('/')[-1].split(ending)[0] + '.root'
dstputfile = sys.argv[0].split('.py')[0] + '/data/dst.' + sys.argv[1].split('/')[-1].split(ending)[0] + '.root'

# adding root input module depending on the input file: ###
if ending == ".sroot":
    main.add_module("SeqRootInput", inputFileNames=dstfiles)
else:
    main.add_module("RootInput", inputFileNames=dstfiles)

# loading gearbox and geometry, which is needed for simulation: ###
main.add_module('Gearbox')
main.add_module('Geometry')

# show progress at least every 10^maxN events: ###
main.add_module('Progress', maxN=3)

# add unpacker function from the neurotrigger script: ###
neurotrigger.add_neuro_2d_unpackers(main, sim13dt=False)

# add filter to just use events with trg information present: ###
main.add_module(neurotrigger.filterTRG(branchname="CDCTriggerNNInput2DFinderTracks"))

# adding neurotrigger simulations for various cases: ###
neurotrigger.add_neurotrigger_sim(main)
neurotrigger.add_neurotrigger_hw(main)
neurotrigger.add_neuro_simulation_swts(main)

# add reconstruction in case .sroot files were used: ###
if ending == ".sroot":
    main.add_module('CDCUnpacker')
    reconstruction.add_reconstruction(main)

# add matcher modules to match trigger tracks to reco tracks: ###
main.add_module('CDCTriggerRecoMatcher', TrgTrackCollectionName=neurotrigger.hwneurotracks,
                hitCollectionName=neurotrigger.hwneuroinputsegmenthits, axialOnly=True)
main.add_module('CDCTriggerRecoMatcher', TrgTrackCollectionName=neurotrigger.hwneurotracks_sim,
                hitCollectionName=neurotrigger.hwneuroinputsetmenthits, axialOnly=True)
main.add_module('CDCTriggerRecoMatcher', TrgTrackCollectionName=neurotrigger.hwneuroinput2dfindertracks,
                hitCollectionName=neurotrigger.hwneuroinputsegmenthits, axialOnly=True)
main.add_module('CDCTriggerRecoHitMatcher', hitCollectionName=neurotrigger.simsegmenthits)
main.add_module('CDCTriggerRecoMatcher', TrgTrackCollectionName=simneurotracks_swtssw2d,
                hitCollectionName=neurotrigger.simsegmenthits, axialOnly=True)
main.add_module('SetupGenfitExtrapolation')

# adding histomanager and the dqm module: ###
main.add_module('HistoManager',
                histoFileName=outputfile)
main.add_module('CDCTriggerNeuroDQM',
                simNeuroTracksName=neurotrigger.hwneurotracks_sim,
                unpackedNeuroInput2dTracksName=hwneurotracks,
                showRecoTracks=True,
                skipWithoutHWTS=True,
                maxRecoZDist=-1,
                maxRecoD0Dist=-1,
                limitedoutput=False,
                )

# add root output: ###
main.add_module('RootOutput', outputFileName=dstputfile)

# run basf2: ###
basf2.process(main)
print(basf2.statistics)
