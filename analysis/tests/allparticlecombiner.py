#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import unittest
import os
import tempfile
import basf2
import modularAnalysis as ma
from variables import variables
from vertex import vertexKFit
from ROOT import Belle2
from ROOT import TFile
from ROOT import TNtuple


class TestAllParticleCombiner(unittest.TestCase):
    """The unit test case for the AllParticleCombiner"""

    def testCombination(self):
        """Run the test fit"""

        testFile = tempfile.NamedTemporaryFile()

        main = basf2.create_path()

        ma.inputMdst('default', Belle2.FileSystem.findFile('analysis/tests/mdst.root'), path=main)

        ma.fillParticleList('pi+:fromPV', 'dr < 2 and abs(dz) < 5', path=main)

        ma.variablesToExtraInfo('pi+:fromPV', {'medianValueInList(pi+:fromPV, dz)': 'medianDZ'}, path=main)
        variables.addAlias('medianDzFromPV', 'extraInfo(medianDZ)')
        variables.addAlias('dzFromMedianDz', 'formula(dz - medianDzFromPV)')
        ma.applyCuts('pi+:fromPV', 'abs(dzFromMedianDz) < 0.05', path=main)

        ma.combineAllParticles(['pi+:fromPV'], 'vpho:PVFit', path=main)

        vertexKFit('vpho:PVFit', conf_level=-1, constraint='iptube', path=main, silence_warning=True)

        variables.addAlias('PVX', 'x')
        variables.addAlias('PVY', 'y')
        variables.addAlias('PVZ', 'z')
        variables.addAlias('nGoodTracksFromPV', 'nParticlesInList(pi+:fromPV)')

        main.add_module(
            'VariablesToNtuple',
            particleList='vpho:PVFit',
            fileName=testFile.name,
            variables=[
                'PVX',
                'PVY',
                'PVZ',
                'chiProb',
                'nGoodTracksFromPV',
                'distance'])

        basf2.process(main)

        ntuplefile = TFile(testFile.name)
        ntuple = ntuplefile.Get('ntuple')

        self.assertFalse(ntuple.GetEntries() == 0, "Ntuple is empty.")

        converged = ntuple.GetEntries("chiProb > 0")

        self.assertFalse(converged == 0, "No fit converged.")

        print("Test passed, cleaning up.")


if __name__ == '__main__':
    unittest.main()
