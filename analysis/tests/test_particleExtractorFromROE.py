#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import unittest
import tempfile
import basf2
import b2test_utils
import modularAnalysis as ma
from ROOT import TFile, Belle2


class TestParticleExtractorFromROE(unittest.TestCase):
    """The unit test for ParticleExtractorFromROE module """

    def test_roepath(self):
        """ Test the extractParticlesFromROE function in roe_path """

        testFile = tempfile.NamedTemporaryFile()

        main = basf2.create_path()

        inputfile = b2test_utils.require_file('analysis/tests/100_noBKG_B0ToPiPiPi0.root', py_case=self)
        ma.inputMdst(inputfile, path=main)

        ma.fillParticleList('pi+:sig', 'pionID > 0.5', path=main)

        ma.fillParticleList('gamma:all', '', path=main)
        ma.reconstructDecay('pi0:sig -> gamma:all gamma:all', '0.125 < InvM < 0.145', path=main)

        ma.reconstructDecay('B0:sig -> pi-:sig pi+:sig pi0:sig', '', path=main)

        ma.matchMCTruth('B0:sig', path=main)
        ma.applyCuts('B0:sig', 'isSignal==1', path=main)

        ma.buildRestOfEvent('B0:sig', fillWithMostLikely=True, path=main)

        roe_path = basf2.create_path()
        deadEndPath = basf2.create_path()
        ma.signalSideParticleFilter('B0:sig', '', roe_path, deadEndPath)

        plists = [f'{ptype}:in_roe' for ptype in ['pi+', 'gamma', 'K_L0', 'K+', 'p+', 'e+', 'mu+']]
        ma.extractParticlesFromROE(plists, maskName='all', path=roe_path)

        charged_inROE = [f'nParticlesInList({ptype}:in_roe)' for ptype in ['pi+', 'K+', 'p+', 'e+', 'mu+']]
        neutral_inROE = [f'nParticlesInList({ptype}:in_roe)' for ptype in ['gamma', 'K_L0']]

        default = ['nParticlesInList(pi+:all)', 'nParticlesInList(gamma:all)', 'nParticlesInList(K_L0:roe_default)']
        mostLikely = [f'nParticlesInList({ptype}:mostlikely_roe)' for ptype in ['K+', 'p+', 'e+', 'mu+']]

        ma.variablesToNtuple('', charged_inROE + neutral_inROE + default + mostLikely,
                             filename=testFile.name,
                             path=roe_path)

        main.for_each('RestOfEvent', 'RestOfEvents', roe_path)

        basf2.process(main)

        ntuplefile = TFile(testFile.name)
        ntuple = ntuplefile.Get('variables')

        self.assertFalse(ntuple.GetEntries() == 0, "Ntuple is empty.")

        nEntries = ntuple.GetEntries()

        # Number of pi+:all = Number of charged-FSPs in ROE + 2-pions in signal side
        cut = Belle2.MakeROOTCompatible.makeROOTCompatible('nParticlesInList(pi+:all)') + ' == ' \
            '(' + '+'.join([Belle2.MakeROOTCompatible.makeROOTCompatible(charged) for charged in charged_inROE]) + ' + 2)'
        nPass_chargedCheck = ntuple.GetEntries(cut)
        self.assertFalse(nPass_chargedCheck < nEntries, "Charged particles are not correctly extracted")

        # Number of mostLikely = Number of charged in ROE
        cut = Belle2.MakeROOTCompatible.makeROOTCompatible('nParticlesInList(e+:mostlikely_roe)') + ' == ' \
            + Belle2.MakeROOTCompatible.makeROOTCompatible('nParticlesInList(e+:in_roe)')
        nPass_electronCheck = ntuple.GetEntries(cut)
        self.assertFalse(nPass_electronCheck < nEntries, "MostLikely option seems broken")

    def test_mainpath(self):
        """ Test the extractParticlesFromROE function in main path """

        testFile = tempfile.NamedTemporaryFile()

        main = basf2.create_path()

        inputfile = b2test_utils.require_file('analysis/tests/100_noBKG_B0ToPiPiPi0.root', py_case=self)
        ma.inputMdst(inputfile, path=main)

        ma.fillParticleList('pi+:sig', 'pionID > 0.5', path=main)

        ma.fillParticleList('gamma:all', '', path=main)
        ma.reconstructDecay('pi0:sig -> gamma:all gamma:all', '0.125 < InvM < 0.145', path=main)

        ma.reconstructDecay('B0:sig -> pi-:sig pi+:sig pi0:sig', '', path=main)

        ma.matchMCTruth('B0:sig', path=main)
        ma.applyCuts('B0:sig', 'isSignal==1', path=main)  # there must be only 1 candidate

        ma.buildRestOfEvent('B0:sig', fillWithMostLikely=True, path=main)

        plists = [f'{ptype}:in_roe' for ptype in ['pi+', 'gamma', 'K_L0', 'K+', 'p+', 'e+', 'mu+']]
        ma.extractParticlesFromROE(plists, maskName='all', path=main, signalSideParticleList='B0:sig')

        charged_inROE = [f'nParticlesInList({ptype}:in_roe)' for ptype in ['pi+', 'K+', 'p+', 'e+', 'mu+']]
        neutral_inROE = [f'nParticlesInList({ptype}:in_roe)' for ptype in ['gamma', 'K_L0']]

        default = ['nParticlesInList(pi+:all)', 'nParticlesInList(gamma:all)', 'nParticlesInList(K_L0:roe_default)']
        mostLikely = [f'nParticlesInList({ptype}:mostlikely_roe)' for ptype in ['K+', 'p+', 'e+', 'mu+']]

        ma.variablesToNtuple('B0:sig', charged_inROE + neutral_inROE + default + mostLikely,
                             filename=testFile.name,
                             path=main)

        basf2.process(main)

        ntuplefile = TFile(testFile.name)
        ntuple = ntuplefile.Get('variables')

        self.assertFalse(ntuple.GetEntries() == 0, "Ntuple is empty.")

        nEntries = ntuple.GetEntries()

        # Number of pi+:all = Number of charged-FSPs in ROE + 2-pions in signal side
        cut = Belle2.MakeROOTCompatible.makeROOTCompatible('nParticlesInList(pi+:all)') + ' == ' \
            '(' + '+'.join([Belle2.MakeROOTCompatible.makeROOTCompatible(charged) for charged in charged_inROE]) + ' + 2)'
        nPass_chargedCheck = ntuple.GetEntries(cut)
        self.assertFalse(nPass_chargedCheck < nEntries, "Charged particles are not correctly extracted")

        # Number of mostLikely = Number of charged in ROE
        cut = Belle2.MakeROOTCompatible.makeROOTCompatible('nParticlesInList(e+:mostlikely_roe)') + ' == ' \
            + Belle2.MakeROOTCompatible.makeROOTCompatible('nParticlesInList(e+:in_roe)')
        nPass_electronCheck = ntuple.GetEntries(cut)
        self.assertFalse(nPass_electronCheck < nEntries, "MostLikely option seems broken")


if __name__ == '__main__':
    with b2test_utils.clean_working_directory():
        unittest.main()
