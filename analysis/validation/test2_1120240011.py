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


def sq_p_component(particle_a, particle_b, component):
    """A variable string for a squared component of a particle's 3-momentum"""
    p = '({A}_P4[{i}] + {B}_P4[{i}])'.format(
        A=particle_a, B=particle_b, i=str(component))
    return '(' + p + '*' + p + ')'


def sumsq_p_string(particle_a, particle_b):
    """Make a variable string for the squared sum of a particle's 3-momentum"""
    xsq = sq_p_component(particle_a, particle_b, 0)
    ysq = sq_p_component(particle_a, particle_b, 1)
    zsq = sq_p_component(particle_a, particle_b, 2)
    return xsq + '+' + ysq + '+' + zsq


def inv_mass_sq(particle_a, particle_b):
    """A variable string or the squared invariant mass of two particles"""
    tsq = sq_p_component(particle_a, particle_b, 3)
    return tsq + '-' + sumsq_p_string(particle_a, particle_b)


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

hq2 = TH1F("hq2", title, 50, 0., 25.0)
hq2.SetXTitle("q^{2} (GeV^{2}/c^{4})")
tree.Project("hq2", inv_mass_sq('B0_mu0', 'B0_mu1'))


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

dq2 = TNamed(
    "Description",
    "Squared invariant mass of the pair of muons (q^{2}) from B^{0}#to K*(892)^{0}#mu^{+}#mu^{-} decays.")
cq2 = TNamed("Check", "Consistent curving shape turns and ends before 25")
pq2 = TNamed("Contact", contactperson)
hq2.GetListOfFunctions().Add(dq2)
hq2.GetListOfFunctions().Add(cq2)
hq2.GetListOfFunctions().Add(pq2)

# save
ofile.cd()
ofile.Write()
ofile.Close()
