"""
<header>
    <input>../phigamma_neutral_skimmed.udst.root</input>
    <output>phigamma_Validation.root</output>
    <contact>giuseppe.finocchiaro@lnf.infn.it</contact>
</header>
"""

import basf2
from stdV0s import stdKshorts
from ROOT import Belle2
from glob import glob
import modularAnalysis as ma
from variables import variables as vm

inputFiles = '../phigamma_neutral_skimmed.udst.root'
histoRootFile = 'phigamma_Validation.root'
inputFileList = glob(inputFiles)

phigamma_path = basf2.Path()
ma.inputMdstList('default', inputFileList, path=phigamma_path)

stdKshorts(path=phigamma_path)
ma.fillParticleList('K+:all', "", writeOut=True, path=phigamma_path)
ma.fillParticleList('K_L0:all', "", writeOut=True, path=phigamma_path)
ma.fillParticleList('gamma:sig', 'nTracks > 1 and 3. < E < 8.', writeOut=True, path=phigamma_path)

ma.reconstructDecay('phi:KK -> K+:all K-:all', '0.9 < M < 1.2', writeOut=True, path=phigamma_path)

vm.addAlias("gamma_E_CMS", "useCMSFrame(E)")
vm.addAlias("gamma_E", "E")
vm.addAlias("K_S0_mass", "M")
vm.addAlias("phi_mass", "M")


variableshisto = [('gamma_E', 120, 2.5, 8.5),
                  ('gamma_E_CMS', 100, 2.0, 7.0),
                  ('nTracks', 15, 0, 15),
                  ]
variableshistoKS = [('K_S0_mass', 200, 0.4, 0.6),
                    ]
variableshistoPhi = [('phi_mass', 200, 0.8, 1.2),
                     ]

ma.variablesToHistogram('gamma:sig', variableshisto, filename=histoRootFile, path=phigamma_path)
ma.variablesToHistogram('K_S0:merged', variableshistoKS, filename=histoRootFile, path=phigamma_path)
ma.variablesToHistogram('phi:KK', variableshistoPhi, filename=histoRootFile, path=phigamma_path)

basf2.process(phigamma_path)
print(basf2.statistics)
