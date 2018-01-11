#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to include the flavor
# tagging user interphase into your analysis IN THE
# SPECIAL CASE you use BELLE MONTE CARLO OR BELLE DATA.
#
# NOTE: Be aware that a specific module order is needed
# in order to convert and analyse BELLE MC OR DATA.
#
# The following decay chain:
#
# B0 -> J/psi Ks
#        |    |
#        |    +-> pi+ pi-
#        |
#        +-> mu- mu+
#
# is the reconstructed B0. The qr value, i.e. the
# flavor*dilution factor of the not reconstructed B0,
# is saved as extraInfo to the reconstructed B0.
#
#
# Note: The weight files for the trained TMVA methods
# were produced using the signal MC sample created in
# MC campaign 5.
#
# Contributors: F. Abudinen & Moritz Gelb (February 2015)
#
######################################################

from basf2 import *
# The FlavorTagger already imports  modularAnalysis
from flavorTagger import *
from b2biiConversion import convertBelleMdstToBelleIIMdst, setupB2BIIDatabase, setupBelleMagneticField
from b2biiMonitors import addBeamParamsConversionMonitors
from b2biiMonitors import addTrackConversionMonitors
from b2biiMonitors import addNeutralsConversionMonitors
from reconstruction import add_mdst_output

if len(sys.argv) != 4:
    sys.exit('Must provide two input parameters: [mc|data] [input_Belle_MDST_file] [output_BelleII_ROOT_file].\n'
             'A small example Belle MDST file can be downloaded from '
             'http://www-f9.ijs.si/~zupanc/evtgen_exp_07_BptoD0pip-D0toKpipi0-0.mdst\n'
             'Check for more belle MC or Data at http://bweb3.cc.kek.jp/\n'
             'Example: http://bweb3/montecarlo.php?ex=33&rs=1&re=9999&ty=evtgen-mixed&dt=on_resonance&bl=caseB&st=Any')

mc_or_data = sys.argv[1].lower()
isMC = {"mc": True, "data": False}.get(mc_or_data, None)
if isMC is None:
    sys.exit('First parameter must be "mc" or "data" to indicate whether we run on MC or real data')

setupB2BIIDatabase(isMC)

inputBelleMDSTFile = sys.argv[2]
outputBelle2ROOTFile = sys.argv[3]

# set the location of the Belle DB server
# options are: ekpbelle.physik.uni-karlsruhe.de
# or ekpbelle.physik.uni-karlsruhe.de (if you're running outside KEKCC computers)
os.environ['BELLE_POSTGRES_SERVER'] = 'can51'
os.environ['USE_GRAND_REPROCESS_DATA'] = '1'

print('BELLE2_EXTERNALS_DIR     = ' + str(os.getenv('BELLE2_EXTERNALS_DIR')))
print('BELLE2_EXTERNALS_SUBDIR  = ' + str(os.getenv('BELLE2_EXTERNALS_SUBDIR')))
print('BELLE2_EXTERNALS_OPTION  = ' + str(os.getenv('BELLE2_EXTERNALS_OPTION')))
print('BELLE2_EXTERNALS_VERSION = ' + str(os.getenv('BELLE2_EXTERNALS_VERSION')))
print('BELLE2_LOCAL_DIR         = ' + str(os.getenv('BELLE2_LOCAL_DIR')))
print('BELLE2_RELEASE           = ' + str(os.getenv('BELLE2_RELEASE')))
print('BELLE2_OPTION            = ' + str(os.getenv('BELLE2_OPTION')))
print('BELLE_POSTGRES_SERVER    = ' + str(os.getenv('BELLE_POSTGRES_SERVER')))
print('USE_GRAND_REPROCESS_DATA = ' + str(os.getenv('USE_GRAND_REPROCESS_DATA')))
print('PANTHER_TABLE_DIR        = ' + str(os.getenv('PANTHER_TABLE_DIR')))
print('PGUSER                   = ' + str(os.getenv('PGUSER')))

# Convert
convertBelleMdstToBelleIIMdst(inputBelleMDSTFile)

# Reconstruct
# first the gearbox needs to be loaded
loadGearbox()

# Create monitoring histograms if wished
# addBeamParamsConversionMonitors()
# addTrackConversionMonitors()
# addNeutralsConversionMonitors()

# Only charged final state particles need to be loaded
# All photon and pi0 candidates are already loaded
# to 'gamma:mdst' and 'pi0:mdst' particle lists
fillParticleList('pi+:all', '')
fillParticleList('K-:all', '')
fillParticleList('mu+:all', '')
fillParticleList('e+:all', '')

# in the case of pi0 candidates in 'pi0:mdst' the mc truth matching
# needs to be executed
matchMCTruth('pi0:mdst')
matchMCTruth('K_S0:mdst')

# fit K_S0 Vertex
vertexRave('K_S0:mdst', -1)

# reconstruct J/psi -> mu+ mu- decay and fit vertex
# keep only candidates with dM<0.11
reconstructDecay('J/psi:mumu ->  mu+:all mu-:all', 'dM<0.11')
applyCuts('J/psi:mumu', '3.07 < M < 3.11')
massVertexRave('J/psi:mumu', 0., '')
matchMCTruth('J/psi:mumu')

# reconstruct B0 -> J/psi Ks decay
# keep only candidates with Mbc > 5.1 and abs(deltaE)<0.15
reconstructDecay('B0:jspiks -> J/psi:mumu  K_S0:mdst', 'Mbc > 5.1 and abs(deltaE)<0.15')

# Fit the B0 Vertex
vertexRave('B0:jspiks', 0.001, 'B0:jspiks -> [J/psi:mumu -> ^mu+ ^mu-] K_S0', '')

# perform MC matching (MC truth asociation). Always before TagV
matchMCTruth('B0:jspiks')

# build the rest of the event associated to the B0
buildRestOfEvent('B0:jspiks')

# Fit Vertex of the B0 on the tag side
TagV('B0:jspiks', 'breco', 0.001, 'standard_PXD')
applyCuts('B0:jspiks', 'abs(DeltaT)<25.')

# Before using the Flavor Tagger you need at least the default weight files. If you do not set
# any parameter the flavorTagger downloads them automatically from the database.
# You just have to use a special global tag of the conditions database. Check in
# https://confluence.desy.de/display/BI/Physics+FlavorTagger
# E.g. for release-00-09-01
use_central_database("GT_gen_prod_003.11_release-00-09-01-FEI-a")
# The default working directory is '.'
# If you have an own analysis package it is recomended to use
# workingDirectory = os.environ['BELLE2_LOCAL_DIR'] + '/analysis/data'.
# Note that if you also train by yourself the weights of the trained Methods are saved therein.
# To save CPU time the weight files should be saved in the same server were you run.
#
# NEVER set uploadToDatabaseAfterTraining to True if you are not a librarian!!!
#
# Flavor Tagging Function. Default Expert mode to use the default weight files for the B2JpsiKs_mu channel.
flavorTagger(
    particleLists=['B0:jspiks'],
    combinerMethods=['TMVA-FBDT', 'FANN-MLP'],
    belleOrBelle2='Belle')
#
# By default the flavorTagger trains and applies two methods, 'TMVA-FBDT' and 'FANN-MLP', for the combiner.
# If you want to train or test the Flavor Tagger only for one of them you have to specify it like:
#
# combinerMethods=['TMVA-FBDT']
#
# With the belleOrBelle2 argument you specify if you are using Belle MC (also Belle Data) or Belle2 MC.
# If you want to use Belle2 MC please follow the dedicated tutorial B2A801-FlavorTagger.py
# since you need to follow a special module order.
#

# You can apply cuts using the flavor Tagger: qrOutput(FBDT) > -2 rejects all events which do not
# provide flavor information using the tag side
applyCuts('B0:jspiks', 'qrOutput(FBDT) > -2')

# If you applied the cut on qrOutput(FBDT) > -2 before then you can rank by highest r- factor
rankByHighest('B0:jspiks', 'abs(qrOutput(FBDT))', 0, 'Dilution_rank')

# create and fill flat Ntuple with MCTruth and kinematic information
toolsK0 = ['EventMetaData', '^K_S0']
toolsK0 += ['Kinematics', '^K_S0 -> ^pi+ ^pi-']
toolsK0 += ['MomentumUncertainty', '^K_S0 -> ^pi+ ^pi-']
toolsK0 += ['InvMass', '^K_S0']
toolsK0 += ['Vertex', '^K_S0']
toolsK0 += ['MCVertex', '^K_S0']
toolsK0 += ['PID', 'K_S0 -> ^pi+ ^pi-']
toolsK0 += ['Track', 'K_S0 -> ^pi+ ^pi-']
toolsK0 += ['TrackHits', 'K_S0 -> ^pi+ ^pi-']
toolsK0 += ['MCTruth', '^K_S0 -> ^pi+ ^pi-']
toolsK0 += [
    'CustomFloats[dr:dz:isSignal:chiProb:extraInfo(goodKs):extraInfo(ksnbVLike):extraInfo(ksnbNoLam):extraInfo(ksnbStandard)]',
    '^K_S0']

toolsB = ['EventMetaData', '^B+']
toolsB += ['InvMass', '^B+ -> ^anti-D0 pi+']
toolsB += ['InvMass[BeforeFit]', '^B+ -> [anti-D0 -> K- pi+ ^pi0] pi+']
toolsB += ['DeltaEMbc', '^B+']
toolsB += ['Cluster', 'B+ -> [anti-D0 -> K- pi+ [pi0 -> ^gamma ^gamma]] pi+']
toolsB += ['MCTruth', '^B+ -> ^anti-D0 pi+']
toolsB += ['CustomFloats[isSignal]', '^B+ -> ^anti-D0 pi+']
toolsB += ['CustomFloats[kIDBelle]', 'B+ -> [anti-D0 -> ^K- ^pi+ pi0] ^pi+']

toolsTrackPI = ['EventMetaData', 'pi+']
toolsTrackPI += ['Kinematics', '^pi+']
toolsTrackPI += ['Track', '^pi+']
toolsTrackPI += ['MCTruth', '^pi+']
toolsTrackPI += ['MCKinematics', '^pi+']
toolsTrackPI += ['ErrMatrix', '^pi+']
toolsTrackPI += ['CustomFloats[eIDBelle:muIDBelleQuality:muIDBelle:atcPIDBelle(3,2):atcPIDBelle(4,2):atcPIDBelle(4,3)]', '^pi+']


tools = [
    'EventMetaData',
    'B0',
    'RecoStats',
    'B0',
    'MCHierarchy',
    'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]',
    'Kinematics',
    '^B0 -> [^J/psi -> ^mu+ ^mu-] [^K_S0 -> ^pi+ ^pi-]',
    'InvMass[BeforeFit]',
    '^B0 -> [^J/psi -> mu+ mu-] [^K_S0 -> pi+ pi-]',
    'MCKinematics',
    '^B0 -> [^J/psi -> ^mu+ ^mu-] [^K_S0 -> ^pi+ ^pi-]',
    'CMSKinematics',
    '^B0 -> [^J/psi -> ^mu+ ^mu-] [^K_S0 -> ^pi+ ^pi-]',
    'MCTruth',
    '^B0 -> [^J/psi -> ^mu+ ^mu-] [^K_S0 -> ^pi+ ^pi-]',
    'DeltaEMbc',
    '^B0 -> [J/psi -> mu+ mu-] [K_S0 -> pi+ pi-]',
    'MCHierarchy',
    'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]',
    'PID',
    'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]',
    'MCVertex',
    '^B0 -> [^J/psi -> mu+ mu-] [^K_S0 -> pi+ pi-]',
    'Vertex',
    '^B0 -> [^J/psi -> ^mu+ ^mu-] [^K_S0 -> ^pi+ ^pi-]',
    'TagVertex',
    '^B0 -> [J/psi -> mu+ mu-] [K_S0 -> pi+ pi-]',
    'MCTagVertex',
    '^B0 -> [J/psi -> mu+ mu-] [K_S0 -> pi+ pi-]',
    'DeltaT',
    '^B0 -> [J/psi -> mu+ mu-] [K_S0 -> pi+ pi-]',
    'MCDeltaT',
    '^B0 -> [J/psi -> mu+ mu-] [K_S0 -> pi+ pi-]',
    'ROEMultiplicities',
    '^B0 -> [J/psi -> mu+ mu-] [K_S0 -> pi+ pi-]',
    'TrackHits',
    'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]',
    'CustomFloats[extraInfo(mdstIndex_rank)]',
    # create and fill flat Ntuple with MCTruth, kinematic information and Flavor Tagger Output
    # Without any arguments only TMVA is saved. If you want to save the FANN Output please specify it.
    # If you set qrCategories, the output of each category is saved.
    '^B0',
    'FlavorTagging[TMVA-FBDT, FANN-MLP, qpCategories]', '^B0',
]
# Note: The Ntuple Output is set to zero during training processes, i.e. when the 'Teacher' mode is used

# write out the flat ntuples
ntupleFile(outputBelle2ROOTFile)
ntupleTree('kshort', 'K_S0:mdst', toolsK0)
ntupleTree('B0tree', 'B0:jspiks', tools)

# Summary of created Lists
summaryOfLists(['J/psi:mumu', 'K_S0:mdst', 'B0:jspiks'])

# Progress
progress = register_module('Progress')
analysis_main.add_module(progress)

process(analysis_main)

# Print call statistics
print(statistics)

# If you want to calculate the efficiency of the FlavorTagger on your own
# File use the script analysis/examples/FlavorTaggerEfficiency.py giving
# your file as argument:

# basf2 FlavorTaggerEfficiency.py YourFile.root

# Note: This efficiency script needs MCParticles. If the name of your tree is not 'B0tree' please change line 65.
