#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

############################################################################
#
# This is a script that shows how to reconstruct B -> J/psi KL events and
# recalculate the KL momentum according to the reconstructed direction using
# kinematical constraints.
#
# Contributors: B. Oberhof, benjamin.oberhof@lnf.infn.it
#
############################################################################

import basf2
import modularAnalysis as ma
from stdCharged import stdMu
from stdKlongs import stdKlongs
import variables.collections as vc
import variables.utils as vu
from variables import variables
from vertex import TagV


main = basf2.create_path()

ma.inputMdstList('default', [basf2.find_file('B02JpsiKL_Jpsi2mumu.root', 'examples', False)], path=main)

# Show progress of processing
main.add_module('ProgressBar')

stdMu('loose', path=main)
stdKlongs('allklm', path=main)

ma.reconstructDecay('J/psi:mumu -> mu-:loose mu+:loose', cut='3.08 < M < 3.12  and 1.45 < useCMSFrame(p) < 1.95', path=main)

pcalc = basf2.register_module('KlongMomentumCalculatorExpert')
pcalc.set_name('KlongMomentumCalculatorExpert_' + 'B0 -> J/psi:mumu K_L0:allklm')
pcalc.param('decayString', 'B0 -> J/psi:mumu K_L0:allklm')
pcalc.param('cut', '')
pcalc.param('decayMode', 0)
pcalc.param('writeOut', False)
pcalc.param('recoList', "_reco")
main.add_module(pcalc)

rmake = basf2.register_module('KlongDecayReconstructorExpert')
rmake.set_name('KlongDecayReconstructorExpert_' + 'B0 -> J/psi:mumu K_L0:allklm')
rmake.param('decayString', 'B0 -> J/psi:mumu K_L0:allklm')
rmake.param('cut', 'M > 0')
rmake.param('decayMode', 0)
rmake.param('writeOut', False)
rmake.param('recoList', "_reco")
main.add_module(rmake)

ma.buildRestOfEvent('B0', path=main)
ma.matchMCTruth('B0', path=main)

TagV('B0', constraintType='tube', confidenceLevel=0.0001, path=main)

commonVariables = vc.inv_mass + vc.mc_truth
commonVariables += vc.kinematics + vc.mc_kinematics
commonVariables += vc.vertex + vc.mc_vertex

variableList = [var for var in commonVariables]
variableList += vc.deltae_mbc + vc.tag_vertex + vc.mc_tag_vertex + vc.reco_stats
variableList += vu.create_aliases(commonVariables, 'daughter(0, {variable})', 'Jpsi')
variableList += vu.create_aliases(commonVariables, 'daughter(1, {variable})', 'KL0')
variableList += vu.create_aliases(vc.kinematics, 'useCMSFrame({variable})', 'CMS')
variableList += vu.create_aliases(vc.kinematics, 'daughter(0, useCMSFrame({variable}))', 'Jpsi_CMS')
variableList += vu.create_aliases(vc.kinematics, 'daughter(1, useCMSFrame({variable}))', 'KL0_CMS')
variables.addAlias('dmID', 'extraInfo(decayModeID)')
variableList += ['dmID']

ma.variablesToNtuple('B0', variables=variableList, filename="Reconstruct_jpsi_kl_example.root", treename='tree', path=main)

# Process all modules added to the main path
basf2.process(main)

# Print out the summary
print(basf2.statistics)
