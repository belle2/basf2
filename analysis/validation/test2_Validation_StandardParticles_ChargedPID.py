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

#: where to find the ntuples
basepath = '../'
#: J/Psi --> ee ntuple
jpsi_ee = TFile.Open(path.join(basepath, '1111540100.ntup.root'))
#: J/Psi --> mumu ntuple
jpsi_mm = TFile.Open(path.join(basepath, '1111440100.ntup.root'))
#: D* --> D0 pi ntuple
dstars = TFile.Open(path.join(basepath, '2610030000.ntup.root'))
#: Xi --> Lambda pi
xilambda = TFile.Open(path.join(basepath, 'ana-xi-lambda.root'))
#: D* tree
ds = dstars.Get("dst")
#: electron tree
electrons = jpsi_ee.Get("B0")
#: muon tree
muons = jpsi_mm.Get("B0")
#: xi tree
xis = xilambda.Get("xitree")
#: simple selection cuts to get a clean D0
pikCuts = "(abs(DST_D0_M-1.86484)<0.012)&&(abs(DST_M-DST_D0_M-0.14543)<0.00075)"
#: simple selection cuts to get a clean J/Psi
jpsiCuts = "(abs(B0_Jpsi_M - 3.09692) < 0.05)"
#: simple selection cuts to get a clean Xi
xiCuts = "(abs(Xi_M-1.3216)<0.005) && (Xi_Lambda0_Rho > 0.5)"

#: file that contains the histograms
outputFile = TFile.Open("standardParticlesValidation_ChargedPID.root", "RECREATE")

#: some coloring options
pid_colors = dict([("e", 41), ("mu", 46), ("p", 36), ('pi', 1), ('K', 29)])
#: pid cuts used as benchmark points
xvals = [0.5, 0.75, 0.8, 0.9]


class Sample:
    """
    Simple container to define the sample via its name, tree, and purity cuts
    """
    def __init__(self, name, tree, cuts, varname):
        """
        @param string name  The particle type of the sample
        @param tree         The tree instance containing the particles
        @param cuts         The cuts to apply to the tree to get a clean sample of the particle
        @param varname      The name of the particles in the tree
        """
        #: name of the particle for the PID cut
        self.name = name
        #: tree with data
        self.tree = tree
        #: cleaning cuts on the tree
        self.cuts = cuts
        #: name of the variable with the particle
        self.varname = varname

#: dictionary of samples. One for each particle
samples = {
    'e': Sample('e', electrons, jpsiCuts, "B0_Jpsi_e0"),
    'mu': Sample('mu', muons, jpsiCuts, "B0_Jpsi_mu0"),
    'pi': Sample('pi', ds, pikCuts, "DST_D0_pi"),
    'K': Sample('K', ds, pikCuts, "DST_D0_K"),
    'p': Sample('p', xis, xiCuts, "Xi_Lambda0_p"),
}


def plot_pidEfficiencyInSample(pid, sample, isExpertMode=False, detector=""):
    """
    Plots the efficiencies for a given sample for a given pid
    """
    metaOptions = "nostats"
    if isExpertMode:
        metaOptions += ", expert"
    if isExpertMode:
        pidString = "Expert_PID"
    else:
        pidString = "PID"
    s = samples[sample]
    track = s.varname
    cuts = s.cuts
    total = s.tree.GetEntries(cuts)
    h = TH1D(
        "%sPIDEff_in_%s" % (sample+pidString, pid),
        "{0} {1} efficiency in a {2} sample;{0} PID cut;efficiency in {2} sample".format(pid, pidString, sample),
        4, 0, 4)
    for bin, pidcut in enumerate(xvals):
        if isExpertMode:
            selection = "(" + cuts + ") && (%s_%sExpertPID%s > %.2f)" % (track, pid, detector, pidcut)
        else:
            selection = "(" + cuts + ") && (%s_PID%s > %.2f)" % (track, pid.lower(), pidcut)
        h.SetBinContent(
            bin+1,
            # the default PID is unfortunately using lower case for the Kaon
            s.tree.GetEntries(selection) / total
        )
        h.GetXaxis().SetBinLabel(bin+1, "{:.2f}".format(pidcut))
        h.GetListOfFunctions().Add(TNamed("MetaOptions", metaOptions))
        h.GetListOfFunctions().Add(TNamed("Description", h.GetTitle()))
        h.GetListOfFunctions().Add(TNamed("Check", "Consistency between the different histograms"))
        h.GetListOfFunctions().Add(TNamed("Contact", "jan.strube@desy.de"))
    outputFile.WriteTObject(h)


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
        plot_pidEfficiencyInSample(pid, sample, isExpertMode=True, detector=detector)
        plot_pidEfficiencyInSample(pid, sample)
