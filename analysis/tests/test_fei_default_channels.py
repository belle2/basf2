#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
import unittest
import unittest.mock
import os
import tempfile
import atexit
import shutil

import fei.default_channels

# @cond


class TestGetDefaultChannels(unittest.TestCase):
    def test_get_default(self):
        particles = fei.default_channels.get_default_channels()
        self.assertEqual([p.identifier for p in particles],
                         ['pi+:generic', 'K+:generic', 'mu+:generic', 'e+:generic', 'gamma:generic', 'pi0:generic', 'K_S0:generic',
                          'J/psi:generic', 'D0:generic', 'D+:generic', 'D_s+:generic', 'D*0:generic', 'D*+:generic',
                          'D_s*+:generic', 'B0:generic', 'B+:generic',
                          'D0:semileptonic', 'D+:semileptonic', 'D*0:semileptonic', 'D*+:semileptonic',
                          'B0:semileptonic', 'B+:semileptonic'])

    def test_get_hadronic(self):
        particles = fei.default_channels.get_default_channels(hadronic=True, semileptonic=False, KLong=False)
        self.assertEqual([p.identifier for p in particles],
                         ['pi+:generic', 'K+:generic', 'mu+:generic', 'e+:generic', 'gamma:generic', 'pi0:generic', 'K_S0:generic',
                          'J/psi:generic', 'D0:generic', 'D+:generic', 'D_s+:generic', 'D*0:generic', 'D*+:generic',
                          'D_s*+:generic', 'B0:generic', 'B+:generic'])

    def test_get_semileptonic(self):
        particles = fei.default_channels.get_default_channels(hadronic=False, semileptonic=True, KLong=False)
        self.assertEqual([p.identifier for p in particles],
                         ['pi+:generic', 'K+:generic', 'mu+:generic', 'e+:generic', 'gamma:generic', 'pi0:generic', 'K_S0:generic',
                          'J/psi:generic', 'D0:generic', 'D+:generic', 'D_s+:generic', 'D*0:generic', 'D*+:generic',
                          'D_s*+:generic', 'D0:semileptonic', 'D+:semileptonic', 'D*0:semileptonic', 'D*+:semileptonic',
                          'B0:semileptonic', 'B+:semileptonic'])

    def test_get_charged(self):
        particles = fei.default_channels.get_default_channels(chargedB=True, neutralB=False)
        self.assertEqual([p.identifier for p in particles],
                         ['pi+:generic', 'K+:generic', 'mu+:generic', 'e+:generic', 'gamma:generic', 'pi0:generic', 'K_S0:generic',
                          'J/psi:generic', 'D0:generic', 'D+:generic', 'D_s+:generic', 'D*0:generic', 'D*+:generic',
                          'D_s*+:generic', 'B+:generic', 'D0:semileptonic', 'D+:semileptonic', 'D*0:semileptonic',
                          'D*+:semileptonic', 'B+:semileptonic'])

    def test_get_neutral(self):
        particles = fei.default_channels.get_default_channels(chargedB=False, neutralB=True)
        self.assertEqual([p.identifier for p in particles],
                         ['pi+:generic', 'K+:generic', 'mu+:generic', 'e+:generic', 'gamma:generic', 'pi0:generic', 'K_S0:generic',
                          'J/psi:generic', 'D0:generic', 'D+:generic', 'D_s+:generic', 'D*0:generic', 'D*+:generic',
                          'D_s*+:generic', 'B0:generic', 'D0:semileptonic', 'D+:semileptonic', 'D*0:semileptonic',
                          'D*+:semileptonic', 'B0:semileptonic'])

    def test_get_klong(self):
        particles = fei.default_channels.get_default_channels(hadronic=False, semileptonic=False, KLong=True)
        self.assertEqual([p.identifier for p in particles],
                         ['pi+:generic', 'K+:generic', 'mu+:generic', 'e+:generic', 'gamma:generic', 'pi0:generic', 'K_S0:generic',
                          'J/psi:generic', 'D0:generic', 'D+:generic', 'D_s+:generic', 'D*0:generic', 'D*+:generic',
                          'D_s*+:generic', 'K_L0:generic', 'D0:KL', 'D+:KL', 'D_s+:KL', 'D*0:KL', 'D*+:KL', 'D_s*+:KL',
                          'B0:KL', 'B+:KL'])

    def test_B_extra_cut(self):
        particles = fei.default_channels.get_default_channels(B_extra_cut='nRemainingTracksInROE == 0', KLong=True)
        self.assertEqual([p.identifier for p in particles],
                         ['pi+:generic', 'K+:generic', 'mu+:generic', 'e+:generic', 'gamma:generic', 'pi0:generic', 'K_S0:generic',
                          'J/psi:generic', 'D0:generic', 'D+:generic', 'D_s+:generic', 'D*0:generic', 'D*+:generic',
                          'D_s*+:generic', 'B0:generic', 'B+:generic',
                          'K_L0:generic', 'D0:KL', 'D+:KL', 'D_s+:KL', 'D*0:KL', 'D*+:KL', 'D_s*+:KL', 'B0:KL', 'B+:KL',
                          'D0:semileptonic', 'D+:semileptonic', 'D*0:semileptonic', 'D*+:semileptonic',
                          'B0:semileptonic', 'B+:semileptonic',
                          ])
        self.assertEqual(particles[14].preCutConfig.userCut, 'Mbc > 5.2 and abs(deltaE) < 0.5 and [nRemainingTracksInROE == 0]')
        self.assertEqual(particles[15].preCutConfig.userCut, 'Mbc > 5.2 and abs(deltaE) < 0.5 and [nRemainingTracksInROE == 0]')
        self.assertEqual(particles[23].preCutConfig.userCut, 'nRemainingTracksInROE == 0')
        self.assertEqual(particles[24].preCutConfig.userCut, 'nRemainingTracksInROE == 0')
        self.assertEqual(particles[29].preCutConfig.userCut, 'nRemainingTracksInROE == 0')
        self.assertEqual(particles[30].preCutConfig.userCut, 'nRemainingTracksInROE == 0')

    def test_get_specific(self):
        particles = fei.default_channels.get_default_channels(specific=True, KLong=True)
        self.assertEqual([p.identifier for p in particles],
                         ['pi+:generic', 'K+:generic', 'mu+:generic', 'e+:generic', 'gamma:generic', 'pi0:generic', 'K_S0:generic',
                          'J/psi:generic', 'D0:generic', 'D+:generic', 'D_s+:generic', 'D*0:generic', 'D*+:generic',
                          'D_s*+:generic', 'B0:generic', 'B+:generic',
                          'K_L0:generic', 'D0:KL', 'D+:KL', 'D_s+:KL', 'D*0:KL', 'D*+:KL', 'D_s*+:KL', 'B0:KL', 'B+:KL',
                          'D0:semileptonic', 'D+:semileptonic', 'D*0:semileptonic', 'D*+:semileptonic',
                          'B0:semileptonic', 'B+:semileptonic'])
        self.assertEqual(particles[0].preCutConfig.userCut, '[dr < 2] and [abs(dz) < 4] and isInRestOfEvent > 0.5')
        self.assertEqual(particles[1].preCutConfig.userCut, '[dr < 2] and [abs(dz) < 4] and isInRestOfEvent > 0.5')
        self.assertEqual(particles[2].preCutConfig.userCut, '[dr < 2] and [abs(dz) < 4] and isInRestOfEvent > 0.5')
        self.assertEqual(particles[3].preCutConfig.userCut, '[dr < 2] and [abs(dz) < 4] and isInRestOfEvent > 0.5')
        self.assertEqual(particles[4].preCutConfig.userCut, '[[clusterReg == 1 and E > 0.10] or [clusterReg == 2 and E > 0.09]'
                         ' or [clusterReg == 3 and E > 0.16]] and isInRestOfEvent > 0.5')
        self.assertEqual(particles[6].preCutConfig.userCut, '0.4 < M < 0.6')
        self.assertEqual(particles[6].channels[2].preCutConfig.userCut, '0.4 < M < 0.6 and isInRestOfEvent > 0.5')
        self.assertEqual(particles[16].preCutConfig.userCut, 'isInRestOfEvent > 0.5')

    def test_get_specific_converted(self):
        particles = fei.default_channels.get_default_channels(specific=True, KLong=True, convertedFromBelle=True)
        self.assertEqual([p.identifier for p in particles],
                         ['pi+:generic', 'K+:generic', 'mu+:generic', 'e+:generic', 'gamma:generic', 'pi0:generic', 'K_S0:generic',
                          'J/psi:generic', 'D0:generic', 'D+:generic', 'D_s+:generic', 'D*0:generic', 'D*+:generic',
                          'D_s*+:generic', 'B0:generic', 'B+:generic',
                          'K_L0:generic', 'D0:KL', 'D+:KL', 'D_s+:KL', 'D*0:KL', 'D*+:KL', 'D_s*+:KL', 'B0:KL', 'B+:KL',
                          'D0:semileptonic', 'D+:semileptonic', 'D*0:semileptonic', 'D*+:semileptonic',
                          'B0:semileptonic', 'B+:semileptonic'])
        self.assertEqual(particles[0].preCutConfig.userCut, '[dr < 2] and [abs(dz) < 4] and isInRestOfEvent > 0.5')
        self.assertEqual(particles[1].preCutConfig.userCut, '[dr < 2] and [abs(dz) < 4] and isInRestOfEvent > 0.5')
        self.assertEqual(particles[2].preCutConfig.userCut, '[dr < 2] and [abs(dz) < 4] and isInRestOfEvent > 0.5')
        self.assertEqual(particles[3].preCutConfig.userCut, '[dr < 2] and [abs(dz) < 4] and isInRestOfEvent > 0.5')
        self.assertEqual(particles[4].preCutConfig.userCut, 'goodBelleGamma == 1 and clusterBelleQuality == 0 '
                                                            'and isInRestOfEvent > 0.5')
        self.assertEqual(particles[5].preCutConfig.userCut, '0.08 < InvM < 0.18 and isInRestOfEvent > 0.5')
        self.assertEqual(particles[6].preCutConfig.userCut, '0.4 < M < 0.6 and isInRestOfEvent > 0.5')
        self.assertEqual(particles[16].preCutConfig.userCut, 'isInRestOfEvent > 0.5')

    def test_get_converted(self):
        particles = fei.default_channels.get_default_channels(convertedFromBelle=True)
        self.assertEqual([p.identifier for p in particles],
                         ['pi+:generic', 'K+:generic', 'mu+:generic', 'e+:generic', 'gamma:generic', 'pi0:generic', 'K_S0:generic',
                          'J/psi:generic', 'D0:generic', 'D+:generic', 'D_s+:generic', 'D*0:generic', 'D*+:generic',
                          'D_s*+:generic', 'B0:generic', 'B+:generic',
                          'D0:semileptonic', 'D+:semileptonic', 'D*0:semileptonic', 'D*+:semileptonic',
                          'B0:semileptonic', 'B+:semileptonic'])

        chargedVariables = ['eIDBelle',
                            'atcPIDBelle(3,2)', 'kIDBelle',
                            'atcPIDBelle(4,2)', 'atcPIDBelle(4,3)',
                            'muIDBelle',
                            'p', 'pt', 'pz', 'dr', 'dz', 'chiProb', 'extraInfo(preCut_rank)']
        self.assertEqual(particles[0].preCutConfig.userCut, '[dr < 2] and [abs(dz) < 4]')
        self.assertEqual(particles[0].mvaConfig.variables, chargedVariables)
        self.assertEqual(particles[0].preCutConfig.bestCandidateVariable, 'atcPIDBelle(2,3)')
        self.assertEqual(particles[1].preCutConfig.userCut, '[dr < 2] and [abs(dz) < 4]')
        self.assertEqual(particles[1].mvaConfig.variables, chargedVariables)
        self.assertEqual(particles[1].preCutConfig.bestCandidateVariable, 'atcPIDBelle(3,2)')
        self.assertEqual(particles[2].preCutConfig.userCut, '[dr < 2] and [abs(dz) < 4]')
        self.assertEqual(particles[2].mvaConfig.variables, chargedVariables)
        self.assertEqual(particles[2].preCutConfig.bestCandidateVariable, 'muIDBelle')
        self.assertEqual(particles[3].preCutConfig.userCut, '[dr < 2] and [abs(dz) < 4]')
        self.assertEqual(particles[3].mvaConfig.variables, chargedVariables)
        self.assertEqual(particles[3].preCutConfig.bestCandidateVariable, 'eIDBelle')
        self.assertEqual(particles[4].preCutConfig.userCut, 'goodBelleGamma == 1 and clusterBelleQuality == 0')
        self.assertEqual(particles[6].mvaConfig.variables, ['dr', 'dz', 'distance', 'significanceOfDistance', 'chiProb',
                                                            'M', 'abs(dM)',
                                                            'useCMSFrame(E)', 'daughterAngle(0,1)',
                                                            'cosAngleBetweenMomentumAndVertexVector',
                                                            'extraInfo(preCut_rank)', 'extraInfo(goodKs)', 'extraInfo(ksnbVLike)',
                                                            'extraInfo(ksnbNoLam)', 'extraInfo(ksnbStandard)'])


class TestGetFRChannels(unittest.TestCase):
    def test_get_default(self):
        particles = fei.default_channels.get_fr_channels()
        self.assertEqual([p.identifier for p in particles],
                         ['pi+:generic', 'K+:generic', 'mu+:generic', 'e+:generic', 'gamma:generic', 'pi0:generic', 'K_S0:generic',
                          'J/psi:generic', 'D0:generic', 'D+:generic', 'D_s+:generic', 'D*0:generic', 'D*+:generic',
                          'D_s*+:generic', 'B0:generic', 'B+:generic'])
        self.assertEqual(len(particles[0].channels), 1)
        self.assertEqual(len(particles[1].channels), 1)
        self.assertEqual(len(particles[2].channels), 1)
        self.assertEqual(len(particles[3].channels), 1)
        self.assertEqual(len(particles[4].channels), 2)
        self.assertEqual(len(particles[5].channels), 1)
        self.assertEqual(len(particles[6].channels), 3)
        self.assertEqual(len(particles[7].channels), 2)
        self.assertEqual(len(particles[8].channels), 10)
        self.assertEqual(len(particles[9].channels), 7)
        self.assertEqual(len(particles[10].channels), 8)
        self.assertEqual(len(particles[11].channels), 2)
        self.assertEqual(len(particles[12].channels), 2)
        self.assertEqual(len(particles[13].channels), 1)
        self.assertEqual(len(particles[14].channels), 15)
        self.assertEqual(len(particles[15].channels), 17)


if __name__ == '__main__':
    tempdir = tempfile.mkdtemp()
    os.chdir(tempdir)
    # main() never returns, so install exit handler to do our cleanup
    atexit.register(shutil.rmtree, tempdir)
    unittest.main()

# @endcond
