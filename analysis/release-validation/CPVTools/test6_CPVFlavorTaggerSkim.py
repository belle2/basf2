#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <contact>Fernando Abudinen; abudinen@mpp.mpg.de</contact>
  <description>This file skims events that are usefull to test the time dependent CPV analysis tools,
  i.e. those events where the signal B0 meson is correctly MC matched and where no tracks on the tag side
  belongs to the signal side. The variable used for skimming is isRelatedRestOfEventB0Flavor.
  As input one needs a MC mdst file for the signal channel B0sig->J/PsiKs.
  The script reconstructs B0sig->J/PsiKs on the signal side
  and applies the flavor tagger on the ROE.
  The vertex of B0sig is reconstructed (JPsi vertex) and the vertex of B0tag is reconstructed
  with the TagV module. </description>
</header>
"""

import basf2 as b2
import modularAnalysis as ma
import os
import sys

from reconstruction import add_mdst_output

if len(sys.argv) != 3:
    sys.exit('Must provide three input parameters: [Belle_Belle2] [output_root_file_name]'
             )

belleOrBelle2Flag = sys.argv[1]
outRootFileName = sys.argv[2]


# create path
cp_val_path = b2.Path()

environmentType = "default"

if belleOrBelle2Flag == "Belle":
    from b2biiConversion import setupB2BIIDatabase

    isBelleMC = True
    # if belleData == "BelleDataConv":
    #    isBelleMC=False
    setupB2BIIDatabase(isBelleMC)  # False for Belle Data True for Belle MC
    os.environ['BELLE_POSTGRES_SERVER'] = 'can51'
    os.environ['USE_GRAND_REPROCESS_DATA'] = '1'

    environmentType = "Belle"

ma.inputMdstList(environmentType=environmentType, filelist=[], path=cp_val_path)

ma.fillParticleList(decayString='mu+:all', cut='', path=cp_val_path)
ma.reconstructDecay(decayString='J/psi:mumu -> mu+:all mu-:all', cut='dM<0.11', path=cp_val_path)
ma.matchMCTruth(list_name='J/psi:mumu', path=cp_val_path)

if belleOrBelle2Flag == "Belle":

    # use the existent K_S0:mdst list
    ma.matchMCTruth(list_name='K_S0:mdst', path=cp_val_path)

    # reconstruct B0 -> J/psi Ks decay
    ma.reconstructDecay(decayString='B0:sig -> J/psi:mumu  K_S0:mdst', cut='Mbc > 5.2 and abs(deltaE)<0.15', path=cp_val_path)

if belleOrBelle2Flag == "Belle2":

    # reconstruct Ks from standard pi+ particle list
    ma.fillParticleList(decayString='pi+:all', cut='', path=cp_val_path)
    ma.reconstructDecay(decayString='K_S0:pipi -> pi+:all pi-:all', cut='dM<0.25', path=cp_val_path)

    # reconstruct B0 -> J/psi Ks decay
    ma.reconstructDecay(decayString='B0:sig -> J/psi:mumu K_S0:pipi', cut='Mbc > 5.2 and abs(deltaE)<0.15', path=cp_val_path)

ma.matchMCTruth(list_name='B0:sig', path=cp_val_path)

ma.buildRestOfEvent(target_list_name='B0:sig', path=cp_val_path)

ma.applyCuts(list_name='B0:sig', cut='abs(isRelatedRestOfEventB0Flavor) == 1', path=cp_val_path)

# # Get Special GT for the flavor tagger weight files
# b2.use_central_database("analysis_tools_release-03-01-00")
#
# # Flavor Tagger, Vertex of Signal Side and TagV
# ft.flavorTagger(
#     particleLists=['B0:sig'],
#     weightFiles='B2JpsiKs_muBGx1',
#     belleOrBelle2=belleOrBelle2Flag,
#     path=cp_val_path)
#
# cutString = "qpCategory(FastPion) > 0.4 and qpCategory(FastPion) < 0.5 " + \
#             "and hasTrueTargets(FastPion) < 1 and isRelatedRestOfEventB0Flavor == 1"
# cutString = "qpCategory(Kaon) > -1 and qpCategory(Kaon) < -0.8 and hasTrueTargets(Kaon) == 0 "
# cutString = "qpCategory(KinLepton) > 0.8 and qpCategory(KinLepton) < 1 and hasTrueTargets(KinLepton) == 0 "
# cutString = "qpCategory(SlowPion)> -1 and qpCategory(SlowPion) < -0.8 and hasTrueTargets(SlowPion) == 0 "
# cutString = "qrOutput(FBDT) > 0.4 and qrOutput(FBDT) < 0.65"

# printVariablesList = []
# for (particleList, category, combinerVariable) in eventLevelParticleLists:
#     if category != "MaximumPstar":
#         cutString = cutString + " and hasTrueTargets(" + category + ") == 0 "
#    printVariablesList.append("qpCategory(" + category + ")")
#    printVariablesList.append("hasTrueTargets(" + category + ")")

# applyCuts('B0:sig', cutString)

# printVariableValues('B0:sig', printVariablesList)

# Skim 1
# The new algorithm
skimfilter = b2.register_module('SkimFilter')
skimfilter.set_name('SkimFilter_B2JPsiKs_NoTargetOfFT')
# Provide the particle lists that will be used as an event/skim filter
skimfilter.param('particleLists', ['B0:sig'])
cp_val_path.add_module(skimfilter)
# ------------------> summaryOfLists -----------------------------------------------------------------


# Create a new path for the skim output
B0skim_path = b2.Path()
# The filter provides a boolean, which is true if any of the argument particle lists are not empty
# skimfilter.if_value('=1', B0skim_path, AfterConditionPath.CONTINUE)
skimfilter.if_value('=0', B0skim_path)

# dump in MDST format
add_mdst_output(cp_val_path, True, outRootFileName)

ma.summaryOfLists(particleLists=['B0:sig'], path=cp_val_path)

# Process the events
ma.process(cp_val_path)

# print out the summary
print(b2.statistics)
