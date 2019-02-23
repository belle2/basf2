#!/usr/bin/env python3
# -*- coding: utf-8 -*

"""
<header>
<input>../1163350000.dst.root</input>
<output>../1163350000.ntup.root</output>
<contact>Racha Cheaib, racha.cheaib@desy.de, Sourav Dey, souravdey@tauex.tau.ac.il</contact>
<interval>nightly</interval>
</header>
"""
#######################################################
#
# This script demonstrates how to reconstruct Btag using
# generically trained FEI and how to validate D(*)+tau- v and D(*) modes. The Btag is forced to a hadronic mode.
#  Release required: release-03-00-02 or later
#
######################################################

import basf2 as b2
import modularAnalysis as ma
from analysisPath import analysis_main
from beamparameters import add_beamparameters
import stdCharged as stdc
import stdPi0s as stdpi0
import stdV0s as stdv0
import stdPhotons as stdphotons
import vertex as vx
import variables.utils as vu
import variables.collections as vc

# create path

my_path = b2.create_path()

gb2_setuprel = "release-03-00-02"

from variables import variables

# their names in the ntuple are human readable
from variables import variables
variables.addAlias('ROE_eextraSel', 'ROE_eextra(ROEclusters)')
variables.addAlias('ROE_neextraSel', 'ROE_neextra(ROEclusters)')
variables.addAlias('ROE_neextra', 'ROE_neextra(simple)')
variables.addAlias('abs_dM', 'abs(dM)')
variables.addAlias('dmID', 'extraInfo(decayModeID)')
# Momenta
variables.addAlias('d0_d0_p', 'daughter(0,daughter(0,p))')  # Dstar
variables.addAlias('d0_d0_pCMS', 'daughter(0,daughter(0,useCMSFrame(p)))')  # Dstar

# Mass
variables.addAlias('d0_M', 'daughter(0,InvM)')  # Bsig
variables.addAlias('d1_M', 'daughter(1,InvM)')  # Btag

variables.addAlias('d2_M', 'daughter(2,InvM)')  # Bsig
variables.addAlias('d3_M', 'daughter(3,InvM)')  # Btag

variables.addAlias('d0_mcpdg', 'daughter(0,mcPDG)')  # Bsig
variables.addAlias('d1_mcpdg', 'daughter(1,mcPDG)')  # Btag

variables.addAlias('d2_mcpdg', 'daughter(2,mcPDG)')  # Bsig
variables.addAlias('d3_mcpdg', 'daughter(3,mcPDG)')  # Btag
variables.addAlias('d0_d0_M', 'daughter(0,daughter(0,InvM))')  # Dstar
variables.addAlias('d0_d1_M', 'daughter(0,daughter(1,InvM))')  # tau
variables.addAlias('d0_d1_d0_M', 'daughter(0,daughter(1,daughter(0,InvM)))')  # muon or electron
variables.addAlias('d0_d0_d0_M', 'daughter(0,daughter(0,daughter(0,InvM)))')  # D+ or D0
variables.addAlias('d_ID', 'daughter(0,daughter(0,daughter(0,extraInfo(decayModeID))))')
variables.addAlias('dstarID', 'daughter(0,daughter(0,extraInfo(decayModeID)))')

# Q2
variables.addAlias('BtagEnergy', 'formula(Ecms/2)')
variables.addAlias('BtagCMSPX', 'daughter(0,useCMSFrame(px))')
variables.addAlias('BtagCMSPY', 'daughter(0,useCMSFrame(py))')
variables.addAlias('BtagCMSPZ', 'daughter(0,useCMSFrame(pz))')

variables.addAlias('Bsig_d0_CMSE', 'daughter(1,daughter(0,useCMSFrame(E)))')
variables.addAlias('Bsig_d0_CMSPX', 'daughter(1,daughter(0,useCMSFrame(px)))')
variables.addAlias('Bsig_d0_CMSPY', 'daughter(1,daughter(0,useCMSFrame(py)))')
variables.addAlias('Bsig_d0_CMSPZ', 'daughter(1,daughter(0,useCMSFrame(pz)))')

variables.addAlias('energySumNoLep', 'formula(BtagEnergy+Bsig_d0_CMSE)')
variables.addAlias('pxSumNoLep', 'formula(BtagCMSPX+Bsig_d0_CMSPX)')
variables.addAlias('pySumNoLep', 'formula(BtagCMSPY+Bsig_d0_CMSPY)')
variables.addAlias('pzSumNoLep', 'formula(BtagCMSPZ+Bsig_d0_CMSPZ)')

variables.addAlias('energyDiffNoLep', 'formula(Ecms-energySumNoLep)')
variables.addAlias('pxDiffNoLep', 'formula(0-pxSumNoLep)')
variables.addAlias('pyDiffNoLep', 'formula(0-pySumNoLep)')
variables.addAlias('pzDiffNoLep', 'formula(0-pzSumNoLep)')

variables.addAlias('energyDiffNoLep2', 'formula(energyDiffNoLep*energyDiffNoLep)')
variables.addAlias('pxDiffNoLep2', 'formula(pxDiffNoLep*pxDiffNoLep)')
variables.addAlias('pyDiffNoLep2', 'formula(pyDiffNoLep*pyDiffNoLep)')
variables.addAlias('pzDiffNoLep2', 'formula(pzDiffNoLep*pzDiffNoLep)')

variables.addAlias('Q2', 'formula(energyDiffNoLep2-[pxDiffNoLep2+pyDiffNoLep2+pzDiffNoLep2])')

pzDiffNoLep2outputRootFile = '../1163350000.ntup.root'

fileList = ['../1163350000.dst.root']

ma.inputMdstList('default', fileList, path=my_path)

stdc.stdPi('all', path=my_path)
stdc.stdE('good', path=my_path)
stdc.stdMu('good', path=my_path)
stdc.stdK('good', path=my_path)
# Calling standard particle lists
ma.cutAndCopyList('pi+:95effptcut', 'pi+:all', 'pt>0.05', path=my_path)
ma.cutAndCopyList('K+:85effCleanTracks', 'K+:good', 'dr < 0.5 and -2 < dz < 2 and pt > 0.1', path=my_path)
ma.cutAndCopyList('e+:95effCleanTracks', 'e+:good', 'dr < 0.5 and -2 < dz < 2 and pt > 0.1', path=my_path)
ma.cutAndCopyList('mu+:95effCleanTracks', 'mu+:good', 'dr < 0.5 and -2 < dz < 2 and pt > 0.1', path=my_path)

ma.cutAndCopyList('pi+:sig', 'pi+:95effptcut', 'dr < 0.5 and -2 < dz < 2 and pt > 0.1', path=my_path)
ma.cutAndCopyList('pi+:lowp', 'pi+:95effptcut', '0.06<useCMSFrame(p)<0.25', path=my_path)

# tag side reconstruction
ma.reconstructDecay('D-:tag ->K-:good pi+:sig pi-:sig', '1.8<M<1.9', path=my_path)
ma.reconstructDecay('B0:tag ->D-:tag pi+:sig', 'Mbc>5.27 and abs(deltaE)<0.2', path=my_path)
# KSs
stdv0.stdKshorts(path=my_path)
# vx.vertexKFit('K_S0:all', 0.001)  Please do not remove the comment for the time being
vx.vertexTree('K_S0:all', 0.0, updateAllDaughters=False, path=my_path)
ma.cutAndCopyList('K_S0:sig', 'K_S0:all', 'distance>0.5 and significanceOfDistance>2', path=my_path)
# Photons
stdphotons.stdPhotons('tight', path=my_path)

# pi0
ma.reconstructDecay('pi0:allsig -> gamma:tight gamma:tight', '0.05<InvM<0.35', path=my_path)
ma.cutAndCopyList('pi0:sig', 'pi0:allsig', '0.124<InvM<0.140', path=my_path)
ma.cutAndCopyList('pi0:lowp', 'pi0:sig', '0.06<useCMSFrame(p)<0.4', path=my_path)
ma.cutAndCopyList('pi+:sig', 'pi+:all', 'dr < 0.5 and -2 < dz < 2 and pt >0.1', path=my_path)
ma.cutAndCopyList('pi+:lowp', 'pi+:all', '0.06<useCMSFrame(p)<0.4', path=my_path)

# D+ :

ma.reconstructDecay('D+:sig1 -> K-:good pi+:sig pi+:sig', '1.7<InvM<2.1', 1, path=my_path)
ma.reconstructDecay('D+:sig2 -> K-:good pi+:sig pi+:sig pi0:sig', '1.7<InvM<2.1', 2, path=my_path)
ma.reconstructDecay('D+:sig3 -> K_S0:sig pi+:sig', '1.7<InvM<2.1', 3, path=my_path)
ma.reconstructDecay('D+:sig4 -> K_S0:sig pi+:sig pi-:95effptcut pi+:sig', '1.7<InvM<2.1', 4, path=my_path)
ma.reconstructDecay('D+:sig5 -> K_S0:sig pi+:sig pi0:sig', '1.7<InvM<2.1', 5, path=my_path)
ma.reconstructDecay('D+:sig6 -> K_S0:sig K+:85effCleanTracks', '1.7<InvM<2.1', 6, path=my_path)

ma.copyLists('D+:sigD', ['D+:sig1', 'D+:sig2', 'D+:sig3', 'D+:sig4', 'D+:sig5', 'D+:sig6'], path=my_path)
ma.rankByLowest('D+:sigD', 'abs_dM', path=my_path)

ma.reconstructDecay('D0:sig1 -> K-:good pi+:sig', '1.7<InvM<2.1', 7, path=my_path)
ma.reconstructDecay('D0:sig2 -> K_S0:sig pi0:sig', '1.7<InvM<2.1', 8, path=my_path)
ma.reconstructDecay('D0:sig3 -> K-:good pi+:sig pi0:sig', '1.7<InvM<2.1', 9, path=my_path)
ma.reconstructDecay('D0:sig4 -> K-:good pi+:sig pi-:sig pi+:sig', '1.7<InvM<2.1', 10, path=my_path)
ma.reconstructDecay('D0:sig5 -> K_S0:sig pi+:sig pi-:sig', '1.7<InvM<2.1', 11, path=my_path)
ma.reconstructDecay('D0:sig6 -> K_S0:sig pi+:sig pi-:sig pi0:sig', '1.7<InvM<2.1', 12, path=my_path)
ma.copyLists('D0:sigD', ['D0:sig1', 'D0:sig2', 'D0:sig3', 'D0:sig4', 'D0:sig5', 'D0:sig6'], path=my_path)

ma.rankByLowest('D0:sigD', 'abs_dM', path=my_path)
ma.reconstructDecay('D*+:sigDstar1 -> D0:sigD pi+:lowp', '0.139<massDifference(0)<0.16', 13, path=my_path)
ma.reconstructDecay('D*+:sigDstar2 ->D+:sigD pi0:lowp', '0.139<massDifference(0)<0.16', 14, path=my_path)

ma.copyLists('D*+:sigDstar', ['D*+:sigDstar1', 'D*+:sigDstar2'], path=my_path)
ma.rankByLowest('D*-:sigDstar', 'massDifference(0)', path=my_path)
ma.rankByLowest('D*-:sigDstar', 'abs_dM', path=my_path)

# tau
ma.reconstructDecay('tau-:ch1 -> e-:95effCleanTracks', '', path=my_path)
ma.reconstructDecay('tau-:ch2 -> mu-:95effCleanTracks', '', path=my_path)
ma.copyLists('tau-:mytau', ['tau-:ch1', 'tau-:ch2'], path=my_path)

# B0:sig

ma.reconstructDecay('B0:sig -> D*-:sigDstar tau+:mytau', 'Mbc>0', path=my_path)

# Upsilon(4S)

ma.reconstructDecay('Upsilon(4S) ->  B0:sig  anti-B0:tag', '', path=my_path)

# Rest of EVent
ma.buildRestOfEvent('Upsilon(4S)', path=my_path)
ma.buildRestOfEvent('B0:sig', path=my_path)

ROETracks = ('ROETracks', '', '')
ROEclusters = ('ROEclusters', '', 'abs(clusterTiming)<clusterErrorTiming and E>0.05')

ma.buildEventShape(path=my_path)
ma.appendROEMasks('Upsilon(4S)', [ROEclusters, ROETracks], path=my_path)

ma.appendROEMask('Upsilon(4S)', 'simple', 'pt>0.05 and -2< dr < 2 and -4.0<dz < 4.0',
                 'abs(clusterTiming)<clusterErrorTiming and E>0.075', path=my_path)
ma.appendROEMask('B0:sig', 'simpleB', 'pt>0.05 and -2< dr < 2 and -4.0<dz < 4.0',
                 'abs(clusterTiming)<clusterErrorTiming and E>0.075', path=my_path)

ma.buildContinuumSuppression('B0:sig', 'simpleB', path=my_path)

Y4S_B0DstarVars_1 = [
    "m2Recoil",
    "pRecoil",
    "ROE_neextra",
    "d0_d0_M",
    "d0_d0_d0_M",
    "d0_d0_pCMS",
    "Q2",
    "d_ID",
    "dstarID",
    "foxWolframR2"]
Y4S_B0DstarVars_2 = vu.create_aliases_for_selected(list_of_variables=vc.kinematics,
                                                   decay_string='Upsilon(4S) -> [anti-B0:sig -> D*+:sigDstar ^tau-:mytau] B0:tag')
Y4S_B0DstarVars_3 = vu.create_aliases_for_selected(
    list_of_variables=[
        'Mbc',
        'deltaE'],
    decay_string='Upsilon(4S) -> [^anti-B0:sig -> D*+:sigDstar tau-:mytau] ^B0:tag')
Y4S_B0DstarVars_4 = vu.create_aliases_for_selected(list_of_variables=['cosTBTO'], decay_string='Upsilon(4S) -> ^B0:sig anti-B0:tag')

V_Y4S_B0Dstar = Y4S_B0DstarVars_1 + Y4S_B0DstarVars_2 + Y4S_B0DstarVars_3 + Y4S_B0DstarVars_4

V_D = ['InvM', 'dmID']
V_tag = ['deltaE', 'Mbc']
# ma.variablesToNtuple('Upsilon(4S)', V_Y4S_B0Dstar,
# filename='../1163350000.ntup.root', treename='Y4S',
# path=my_path)#commented out by Sourav Dey for the time being. Please do not remove the line. SD
ma.variablesToNtuple('D0:sigD', V_D, filename='../1163350000.ntup.root', treename='D0all', path=my_path)
ma.variablesToNtuple('D*+:sigDstar', V_D, filename='../1163350000.ntup.root', treename='DSTsig', path=my_path)
ma.variablesToNtuple('D+:sigD', V_D, filename='../1163350000.ntup.root', treename='Dpall', path=my_path)
ma.variablesToNtuple('B0:tag', V_tag, filename='../1163350000.ntup.root', treename='Btag', path=my_path)
# Process the events
b2.process(my_path)
# print out the summary
print(ma.statistics)
