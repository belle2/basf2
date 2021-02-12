#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>muon-KLMValidation.root</input>
    <contact>Martina Laurenza (martina.laurenza@roma3.infn.it)</contact>
    <description>Creation of EKLM muon validation plots.</description>
</header>
"""

import ROOT
from ROOT import TNamed

ROOT.gROOT.SetBatch(True)
input_file = ROOT.TFile('../muon-KLMValidation.root')
tree = input_file.Get('tree')
output_file = ROOT.TFile('EKLMMuon.root', 'recreate')
contact = 'Martina Laurenza (martina.laurenza@roma3.infn.it)'


# Shifter Plots

h2dtres = ROOT.TH1F('muon_h2dtres', 'EKLM muon 2d hits time resolution',
                    200, -20, 10)
h2dtres.SetXTitle('ns')
h2dtres.SetYTitle('Events')
tree.Draw('EKLMHit2ds.m_Time-EKLMHit2ds.m_MCTime>>muon_h2dtres')
functions = h2dtres.GetListOfFunctions()
functions.Add(TNamed('Description', 'Time resolution'))
functions.Add(TNamed('Check', 'No bias.'))
functions.Add(TNamed('Contact', contact))
functions.Add(TNamed('MetaOptions', 'shifter'))

section = ROOT.TH1F('Forward', 'Section for EKLMHit2ds', 2, 0.5, 2.5)
section.GetXaxis().SetTitle('1=backward  2=forward')
section.GetYaxis().SetTitle('Events')
tree.Draw('EKLMHit2ds.getSection()>>Forward', '')
functions = section.GetListOfFunctions()
functions.Add(TNamed('Description',
                     'Flag indicating if a muon hit is in backward(1) or forward(2) EKLM'))
functions.Add(TNamed('Check', 'More hits in backward.'))
functions.Add(TNamed('Contact', contact))
functions.Add(TNamed('MetaOptions', 'shifter'))

sector = ROOT.TH1F('Sector', 'Sector for EKLMHit2ds', 6, -0.5, 5.5)
sector.GetXaxis().SetTitle('Sector #')
sector.GetYaxis().SetTitle('Events')
tree.Draw('EKLMHit2ds.getSector()>>Sector', '')
functions = sector.GetListOfFunctions()
functions.Add(TNamed('Description', 'Sector number of muon hit'))
functions.Add(TNamed('Check', 'More hits in sector 1, then mostly flat.'))
functions.Add(TNamed('Contact', contact))
functions.Add(TNamed('MetaOptions', 'shifter'))

layer = ROOT.TH1F('Layer', 'Layer for EKLMHit2ds', 16, -0.5, 15.5)
layer.GetXaxis().SetTitle('Layer #')
layer.GetYaxis().SetTitle('Events')
tree.Draw('EKLMHit2ds.getLayer()>>Layer', '')
functions = layer.GetListOfFunctions()
functions.Add(TNamed('Description', 'Layer number of muon hit'))
functions.Add(TNamed('Check', 'Number of events decreases with number of layer.'))
functions.Add(TNamed('Contact', contact))
functions.Add(TNamed('MetaOptions', 'shifter'))

time = ROOT.TH1F('Time', 'Hit time for EKLMHit2ds', 150, 0.0, 30.0)
time.GetXaxis().SetTitle('t (ns)')
time.GetYaxis().SetTitle('Events')
tree.Draw('EKLMHit2ds.getTime()>>Time', '')
functions = time.GetListOfFunctions()
functions.Add(TNamed('Description', 'Time of muon hit in EKLM'))
functions.Add(TNamed('Check', 'Mean around 13 ns.'))
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
tree.Draw('EKLMHit2ds.getEnergyDeposit()>>edep', '')
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
