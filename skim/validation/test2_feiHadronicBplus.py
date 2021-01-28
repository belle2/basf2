#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
    <input>../feiHadronicBplus.udst.root</input>
    <output>feiHadronicBplus_Validation.root</output>
    <contact>philip.grace@adelaide.edu.au</contact>
</header>
"""

import basf2 as b2
import modularAnalysis as ma
from variables import variables as vm
from validation_tools.metadata import create_validation_histograms

path = b2.Path()

ma.inputMdst('default', '../feiHadronicBplus.udst.root', path=path)

vm.addAlias('sigProb', 'extraInfo(SignalProbability)')
vm.addAlias('log10_sigProb', 'log10(extraInfo(SignalProbability))')
vm.addAlias('d0_massDiff', 'daughter(0,massDifference(0))')
vm.addAlias('d0_M', 'daughter(0,M)')
vm.addAlias('decayModeID', 'extraInfo(decayModeID)')
vm.addAlias('nDaug', 'countDaughters(1>0)')  # Dummy cut so all daughters are selected.

histogramFilename = 'feiHadronicBplus_Validation.root'
myEmail = 'Phil Grace <philip.grace@adelaide.edu.au>'

create_validation_histograms(
    rootfile=histogramFilename,
    particlelist='B+:generic',
    variables_1d=[
        ('sigProb', 100, 0.0, 1.0, 'Signal probability', myEmail,
         'Signal probability of the reconstructed tag B candidates', 'Most around zero, with a tail at non-zero values.',
         'Signal probability', 'Candidates', 'logy'),
        ('nDaug', 6, 0.0, 6, 'Number of daughters of tag B', myEmail,
         'Number of daughters of tag B', 'Some distribution of number of daughters', 'n_{daughters}', 'Candidates'),
        ('d0_massDiff', 100, 0.0, 0.5, 'Mass difference of D* and D', myEmail,
         'Mass difference of D^{*} and D', 'Peak at 0.14 GeV', 'm(D^{*})-m(D) [GeV]', 'Candidates', 'shifter'),
        ('d0_M', 100, 0.0, 3.0, 'Mass of zeroth daughter (D* or D)', myEmail,
         'Mass of zeroth daughter of tag B (either a $D^{*}$ or a D)', 'Peaks at 1.86 GeV and 2.00 GeV',
         'm(D^{(*)}) [GeV]', 'Candidates', 'shifter'),
        ('deltaE', 40, -0.2, 0.2, '#Delta E', myEmail,
         '$\\Delta E$ of event', 'Peak around zero', '#Delta E [GeV]', 'Candidates', 'shifter'),
        ('Mbc', 40, 5.2, 5.3, 'Mbc', myEmail,
         'Beam-constrained mass of event', 'Peak around B mass (5.28 GeV)', 'M_{bc} [GeV]', 'Candidates', 'shifter')],
    variables_2d=[('deltaE', 100, -0.2, 0.2, 'Mbc', 100, 5.2, 5.3, 'Mbc vs deltaE', myEmail,
                   'Plot of the $\\Delta E$ of the event against the beam constrained mass',
                   'Peak of $\\Delta E$ around zero, and $M_{bc}$ around B mass (5.28 GeV)',
                   '#Delta E [GeV]', 'M_{bc} [GeV]', 'colz'),
                  ('decayModeID', 29, 0, 29, 'log10_sigProb', 100, -3.0, 0.0,
                   'Signal probability for each decay mode ID', myEmail,
                   'Signal probability for each decay mode ID', 'Some distribtuion of candidates in the first few decay mode IDs',
                   'Decay mode ID', '#log_10(signal probability)', 'colz')],
    path=path)

b2.process(path)
print(b2.statistics)
