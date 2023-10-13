#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
    <input>muon-KLMValidation.root</input>
    <contact>Laura Salutari (Laura.Salutari@uniroma3.it)</contact>
    <description>Creation of EKLM muon validation plots.</description>
</header>
"""

import ROOT
from ROOT import TNamed

ROOT.gROOT.SetBatch(True)
input_file = ROOT.TFile('../muon-KLMValidation.root')
tree = input_file.Get('tree')
output_file = ROOT.TFile('EKLMMuon.root', 'recreate')
contact = 'Laura Salutari (Laura.Salutari@uniroma3.it)'


# Shifter Plots

subdetector_selection = 'KLMHit2ds.m_Subdetector==2'

h2dtres = ROOT.TH1F('muon_h2dtres', 'EKLM muon 2d hits time resolution',
                    80, -20, 20)
h2dtres.SetXTitle('ns')
h2dtres.SetYTitle('Events')
tree.Draw('KLMHit2ds.m_Time+(EventT0.hasEventT0()?EventT0.getEventT0():0)-KLMHit2ds.m_MCTime>>muon_h2dtres', subdetector_selection)
functions = h2dtres.GetListOfFunctions()
functions.Add(TNamed('Description', 'Time resolution'))
functions.Add(TNamed('Check', 'No bias.'))
functions.Add(TNamed('Contact', contact))
functions.Add(TNamed('MetaOptions', 'shifter'))

section = ROOT.TH1F('Forward', 'Section for KLMHit2ds', 2, 0.5, 2.5)
section.GetXaxis().SetTitle('1=backward  2=forward')
section.GetYaxis().SetTitle('Events')
tree.Draw('KLMHit2ds.getSection()>>Forward', subdetector_selection)
functions = section.GetListOfFunctions()
functions.Add(TNamed('Description',
                     'Flag indicating if a muon hit is in backward(1) or forward(2) EKLM'))
functions.Add(TNamed('Check', 'More hits in backward.'))
functions.Add(TNamed('Contact', contact))
functions.Add(TNamed('MetaOptions', 'shifter'))

sector = ROOT.TH1F('Sector', 'Sector for KLMHit2ds', 6, -0.5, 5.5)
sector.GetXaxis().SetTitle('Sector #')
sector.GetYaxis().SetTitle('Events')
tree.Draw('KLMHit2ds.getSector()>>Sector', subdetector_selection)
functions = sector.GetListOfFunctions()
functions.Add(TNamed('Description', 'Sector number of muon hit'))
functions.Add(TNamed('Check', 'More hits in sector 1, then mostly flat.'))
functions.Add(TNamed('Contact', contact))
functions.Add(TNamed('MetaOptions', 'shifter'))

layer = ROOT.TH1F('Layer', 'Layer for KLMHit2ds', 16, -0.5, 15.5)
layer.GetXaxis().SetTitle('Layer #')
layer.GetYaxis().SetTitle('Events')
tree.Draw('KLMHit2ds.getLayer()>>Layer', subdetector_selection)
functions = layer.GetListOfFunctions()
functions.Add(TNamed('Description', 'Layer number of muon hit'))
functions.Add(TNamed('Check', 'Number of events decreases with number of layer.'))
functions.Add(TNamed('Contact', contact))
functions.Add(TNamed('MetaOptions', 'shifter'))

time = ROOT.TH1F('Time', 'Hit time for KLMHit2ds', 80, -5.0, 35.0)
time.GetXaxis().SetTitle('t (ns)')
time.GetYaxis().SetTitle('Events')
tree.Draw('KLMHit2ds.getTime()>>Time', subdetector_selection)
functions = time.GetListOfFunctions()
functions.Add(TNamed('Description', 'Time of muon hit in EKLM'))
functions.Add(TNamed('Check', 'Mean around 16 ns.'))
functions.Add(TNamed('Contact', contact))
functions.Add(TNamed('MetaOptions', 'shifter'))

# Expert Plots

nPE = ROOT.TH1F('nGenPE', 'Generated PE in EKLM', 100, 0.0, 200)
nPE.GetXaxis().SetTitle('# generated PE')
nPE.GetYaxis().SetTitle('Events')
tree.Draw('KLMDigits.getNGeneratedPhotoelectrons()>>nGenPE',
          'KLMDigits.getSubdetector()==2')
functions = nPE.GetListOfFunctions()
functions.Add(TNamed('Description', 'Number of generated photoelectrons in EKLM.'))
functions.Add(TNamed('Check', 'Mean around 37.'))
functions.Add(TNamed('Contact', contact))
functions.Add(TNamed('MetaOptions', 'expert'))

edep = ROOT.TH1F('edep', 'Energy deposition for KLMDigits', 150, 0.0, 15)
edep.GetXaxis().SetTitle('E (MeV)')
edep.GetYaxis().SetTitle('Events')
tree.Draw('KLMHit2ds.getEnergyDeposit()>>edep', subdetector_selection)
functions = edep.GetListOfFunctions()
functions.Add(TNamed('Description', 'dE/dx energy deposition of muon hit'))
functions.Add(TNamed('Check', 'Peak around 3 MeV, with mean 3.7 MeV.'))
functions.Add(TNamed('Contact', contact))
functions.Add(TNamed('MetaOptions', 'expert'))

output_file.cd()
h2dtres.Write()
section.Write()
sector.Write()
layer.Write()
time.Write()
nPE.Write()
edep.Write()
output_file.Close()
