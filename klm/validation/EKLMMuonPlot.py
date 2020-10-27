#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../muon-KLMValidation.root</input>
    <contact>Martina Laurenza (martina.laurenza@roma3.infn.it)</contact>
    <description>Creation of EKLM muon validation plots.</description>
</header>
"""

from basf2 import *
import ROOT
from ROOT import Belle2
from ROOT import TFile, TTree, TH1F, TNamed

ROOT.gROOT.SetBatch(True)
input_file = ROOT.TFile('../muon-KLMValidation.root')
tree = input_file.Get('tree')
output_file = ROOT.TFile('EKLMMuon.root', 'recreate')
contact = 'Martina Laurenza (martina.laurenza@roma3.infn.it)'


# Shifter Plots

h2dtres = ROOT.TH1F('muon_h2dtres', 'EKLM muon 2d hits time resolution',
                    100, -10, 10)
h2dtres.SetXTitle('ns')
h2dtres.SetYTitle('Events')
tree.Draw('EKLMHit2ds.m_Time-EKLMHit2ds.m_MCTime>>muon_h2dtres')
functions = h2dtres.GetListOfFunctions()
functions.Add(TNamed('Description', 'Time resolution'))
functions.Add(TNamed('Check', 'No bias.'))
functions.Add(TNamed('Contact', contact))
functions.Add(TNamed('MetaOptions', 'shifter'))

eklm_digits_tres = ROOT.TH1F('eklm_digits_tres', 'KLMDigits Time resolution in EKLM',
                             250, -25, 25)
eklm_digits_tres.SetXTitle('ns')
eklm_digits_tres.SetYTitle('Events')
tree.Draw('KLMDigits.m_Time-KLMDigits.m_MCTime>>eklm_digits_tres',
          'KLMDigits.getSubdetector()==2')
functions = eklm_digits_tres.GetListOfFunctions()
functions.Add(TNamed('Description', 'KLMDigits Time resolution in EKLM'))
functions.Add(TNamed('Check', '.'))
functions.Add(TNamed('Contact', contact))
functions.Add(TNamed('MetaOptions', 'shifter'))

section = TH1F('Forward', 'Section for EKLMHit2ds', 2, 0.5, 2.5)
section.GetXaxis().SetTitle('1=backward  2=forward')
section.GetYaxis().SetTitle('Events')
tree.Draw('EKLMHit2ds.getSection()>>Forward', '')
functions = section.GetListOfFunctions()
functions.Add(TNamed('Description', 'Flag indicating if a muon hit is in backward(1) or forward(2) EKLM'))
functions.Add(TNamed('Check', ''))
functions.Add(TNamed('Contact', contact))
functions.Add(TNamed('MetaOptions', 'shifter'))

sector = TH1F('Sector', 'Sector for EKLMHit2ds', 6, -0.5, 5.5)
sector.GetXaxis().SetTitle('Sector #')
sector.GetYaxis().SetTitle('Events')
tree.Draw('EKLMHit2ds.getSector()>>Sector', '')
functions = sector.GetListOfFunctions()
functions.Add(TNamed('Description', 'Sector number of muon hit'))
functions.Add(TNamed('Check', ''))
functions.Add(TNamed('Contact', contact))
functions.Add(TNamed('MetaOptions', 'shifter'))

layer = TH1F('Layer', 'Layer for EKLMHit2ds', 16, -0.5, 15.5)
layer.GetXaxis().SetTitle('Layer #')
layer.GetYaxis().SetTitle('Events')
tree.Draw('EKLMHit2ds.getLayer()>>Layer', '')
functions = layer.GetListOfFunctions()
functions.Add(TNamed('Description', 'Layer number of muon hit'))
functions.Add(TNamed('Check', ''))
functions.Add(TNamed('Contact', contact))
functions.Add(TNamed('MetaOptions', 'shifter'))

time = TH1F('Time', 'Hit time for EKLMHit2ds', 150, 0.0, 30.0)
time.GetXaxis().SetTitle('t (ns)')
time.GetYaxis().SetTitle('Events')
tree.Draw('EKLMHit2ds.getTime()>>Time', '')
functions = time.GetListOfFunctions()
functions.Add(TNamed('Description', 'Time of muon hit in EKLM'))
functions.Add(TNamed('Check', ''))
functions.Add(TNamed('Contact', contact))
functions.Add(TNamed('MetaOptions', 'shifter'))

nPE = TH1F('nGenPE', 'Generated PE in EKLM', 100, 0.0, 200)
nPE.GetXaxis().SetTitle('# generated PE')
nPE.GetYaxis().SetTitle('Events')
tree.Draw('KLMDigits.getNGeneratedPhotoelectrons()>>nGenPE',
          'KLMDigits.getSubdetector()==2')
functions = nPE.GetListOfFunctions()
functions.Add(TNamed('Description', 'Number of generated photoelectrons in EKLM'))
functions.Add(TNamed('Check', ''))
functions.Add(TNamed('Contact', contact))
functions.Add(TNamed('MetaOptions', 'shifter'))

# Expert Plots

edep = TH1F('edep', 'Energy deposition for KLMDigits', 150, 0.0, 15)
edep.GetXaxis().SetTitle('E (MeV)')
edep.GetYaxis().SetTitle('Events')
tree.Draw('EKLMHit2ds.getEnergyDeposit()>>edep', '')
functions = edep.GetListOfFunctions()
functions.Add(TNamed('Description', 'dE/dx energy deposition of muon hit'))
functions.Add(TNamed('Check', ''))
functions.Add(TNamed('Contact', contact))
functions.Add(TNamed('MetaOptions', 'shifter'))

output_file.cd()
h2dtres.Write()
eklm_digits_tres.Write()
section.Write()
sector.Write()
layer.Write()
time.Write()
nPE.Write()
edep.Write()
output_file.Close()
