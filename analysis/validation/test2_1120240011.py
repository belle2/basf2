#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../1120240011.ntup.root</input>
  <output>1120240011_Validation.root</output>
  <contact>Sam Cunliffe; sam.cunliffe@desy.de</contact>
</header>
"""

# Validation plotting script for event type: 112024001
# i.e. B0 -> K*0 mu+ mu-
#            |
#            +-> K+ pi-
#
# Contributors: Sam Cunliffe (October 2017)

from ROOT import TFile, TTree, TH1F, TNamed

# TTree of candidates, declare output file
ifile = TFile("../1120240011.ntup.root", "READ")
tree = ifile.Get("h1")
ofile = TFile("1120240011_Validation.root", "RECREATE")

title = "B^{0}#rightarrow K*^{0}#mu#mu"
contactperson = "Saurabh Sandilya (saurabhsandilya@gmail.com)"

hmbc = TH1F("hmbc", title, 180, 5.2, 5.29)
hmbc.SetXTitle("M_{bc} (GeV/c^{2})")
tree.Project("hmbc", "B0_mbc")

hdeltae = TH1F("hdeltae", title, 100, -0.5, 0.5)
hdeltae.SetXTitle("#Delta E (GeV)")
tree.Project("hdeltae", "B0_deltae")

hmkpi = TH1F("hmkpi", title, 70, 0.795, 0.995)
hmkpi.SetXTitle("m_{K#pi} (GeV/c^{2})")
tree.Project("hmkpi", "B0_KST0_M")

# add meaningful information to the histograms
dmbc = TNamed("Description", "Reconstructed M_{bc} distribution for B^{0}#to K*(892)^{0}#mu^{+}#mu^{-} decays.")
cmbc = TNamed("Check", "Consistent shape, centred at 0, longer tail on right")
pmbc = TNamed("Contact", contactperson)
hmbc.GetListOfFunctions().Add(dmbc)
hmbc.GetListOfFunctions().Add(cmbc)
hmbc.GetListOfFunctions().Add(pmbc)

ddeltae = TNamed("Description", "Reconstructed #Delta E distribution for B^{0}#to K*(892)^{0}#mu^{+}#mu^{-} decays.")
cdeltae = TNamed("Check", "Consistent shape, centred at 0, longer tail on right")
pdeltae = TNamed("Contact", contactperson)
hdeltae.GetListOfFunctions().Add(ddeltae)
hdeltae.GetListOfFunctions().Add(cdeltae)
hdeltae.GetListOfFunctions().Add(pdeltae)

dmkpi = TNamed(
    "Description",
    "Reconstructed K#pi invariant mass distribution for K*(892)^{0}#to K#pi, from B^{0}#to K*(892)^{0}#mu^{+}#mu^{-} decays.")
cmkpi = TNamed("Check", "Consistent Breit-Wigner shape, mean at 0.896 GeV/c^{2}")
pmkpi = TNamed("Contact", contactperson)
hmkpi.GetListOfFunctions().Add(dmkpi)
hmkpi.GetListOfFunctions().Add(cmkpi)
hmkpi.GetListOfFunctions().Add(pmkpi)

# save
ofile.cd()
ofile.Write()
ofile.Close()
