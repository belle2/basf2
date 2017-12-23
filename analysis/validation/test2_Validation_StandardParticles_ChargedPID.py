"""
<header>
<input>../1111540100.ntup.root,../1111440100.ntup.root,../2610030000.ntup.root,../ana-xi-lambda.root</input>
<output>standardParticlesValidation_ChargedPID.root</output>
<contact>"Jan Strube" <jan.strube@pnnl.gov></contact>
</header>
"""

import ROOT
ROOT.gROOT.SetBatch()
from ROOT import TFile, TNamed, TH1D
import sys
import os.path as path

basepath = '../'
jpsi_ee = TFile.Open(path.join(basepath, '1111540100.ntup.root'))
jpsi_mm = TFile.Open(path.join(basepath, '1111440100.ntup.root'))
dstars = TFile.Open(path.join(basepath, '2610030000.ntup.root'))
xilambda = TFile.Open(path.join(basepath, 'ana-xi-lambda.root'))
ds = dstars.Get("dst")
electrons = jpsi_ee.Get("B0")
muons = jpsi_mm.Get("B0")
xis = xilambda.Get("xitree")
pikCuts = "(abs(DST_D0_M-1.86484)<0.012)&&(abs(DST_M-DST_D0_M-0.14543)<0.00075)"
jpsiCuts = "(abs(B0_Jpsi_M - 3.09692) < 0.05)"
xiCuts = "(abs(Xi_M-1.3216)<0.005) && (Xi_Lambda0_Rho > 0.5)"

outputFile = TFile.Open("standardParticlesValidation_ChargedPID.root", "RECREATE")

pid_colors = dict([("e", 41), ("mu", 46), ("p", 36), ('pi', 1), ('K', 29)])
# pid cuts used as benchmark points
xvals = [0.5, 0.75, 0.8, 0.9]


class Sample:
    """
    Simple container to define the sample via its name, tree, and purity cuts
    """
    def __init__(self, pid, tree, cuts, pname):
        self.pid = pid      # name of the particle for the PID cut
        self.tree = tree    # tree with data
        self.cuts = cuts    # cleaning cuts on the tree
        self.pname = pname  # name of the variable with the particle

samples = {
    'e': Sample('e', electrons, jpsiCuts, "B0_Jpsi_e0"),
    'mu': Sample('mu', muons, jpsiCuts, "B0_Jpsi_mu0"),
    'pi': Sample('pi', ds, pikCuts, "DST_D0_pi"),
    'K': Sample('K', ds, pikCuts, "DST_D0_K"),
    'p': Sample('p', xis, xiCuts, "Xi_Lambda0_p"),
}


def plot_pidEfficiencyInSample(pid, sample, detector=""):
    """
    Plots the fake rate for a given sample for a given pid
    """
    # which sample is faking which particle?
    s = samples[sample]
    track = s.pname
    cuts = s.cuts
    total = s.tree.GetEntries(cuts)
    h = TH1D(
        "%sPIDEff_in_%s" % (sample, pid),
        "{0} PID efficiency in {1} sample;{0} PID cut;efficiency in {1} sample".format(pid, sample),
        4, 0, 4)
    for bin, pidcut in enumerate(xvals):
        h.SetBinContent(
            bin+1,
            s.tree.GetEntries("(" + cuts + ") && (%s_%sExpertPID%s > %.2f)" % (track, pid, detector, pidcut)) / total
        )
        h.GetXaxis().SetBinLabel(bin+1, "{:.2f}".format(pidcut))
        h.GetListOfFunctions().Add(TNamed("Meta", "nostats"))
        h.GetListOfFunctions().Add(TNamed("Description", h.GetTitle()))
        h.GetListOfFunctions().Add(TNamed("Check", "Consistency between the different histograms"))
        h.GetListOfFunctions().Add(TNamed("Contact", "jan.strube@desy.de"))
    outputFile.WriteTObject(h)
    pass


for detector in ("_ALL",):
    for pid, sample in [
        ("e", "e"),     # sample PID efficiencies
        ("mu", "mu"),
        ("pi", "pi"),
        ("K", "K"),
        ("p", "p"),
        ("e", "pi"),    # fake rates; focus on pions faking other particles
        ("e", "p"),
        ('mu', 'pi'),
        ('pi', 'K'),
        ('K', 'pi'),
        ('p', 'e'),
        ('p', 'pi')
    ]:
        plot_pidEfficiencyInSample(pid, sample, detector)
