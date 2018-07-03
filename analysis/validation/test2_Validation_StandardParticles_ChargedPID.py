"""
<header>
<input>../PIDMuons.ntup.root,../PIDElectrons.ntup.root,../PIDProtons.ntup.root,../PIDKpi.ntup.root</input>
<output>standardParticlesValidation_ChargedPID.root</output>
<contact>Jan Strube jan.strube@desy.de, Dmitrii Neverov dmitrii.neverov@desy.de</contact>
</header>
"""

import ROOT
ROOT.gROOT.SetBatch()
from ROOT import TFile, TNamed, TH1D, TEfficiency
import sys
import os.path as path

#: where to find the ntuples
basepath = '../'
#: J/Psi --> ee ntuple
jpsi_ee = TFile.Open(path.join(basepath, 'PIDElectrons.ntup.root'))
#: J/Psi --> mumu ntuple
jpsi_mm = TFile.Open(path.join(basepath, 'PIDMuons.ntup.root'))
#: D* --> D0 pi ntuple
dstars = TFile.Open(path.join(basepath, 'PIDKpi.ntup.root'))
#: Xi --> Lambda pi
xilambda = TFile.Open(path.join(basepath, 'PIDProtons.ntup.root'))
#: D* tree
ds = dstars.Get("dst")
#: electron tree
electrons = jpsi_ee.Get("B0")
#: muon tree
muons = jpsi_mm.Get("B0")
#: xi tree
xis = xilambda.Get("xitree")
#: simple selection cuts to get a clean D0
pikCuts = "(abs(DST_D0_M-1.86484)<0.024)&&(abs(DST_M-DST_D0_M-0.14543)<0.0015)"
#: simple selection cuts to get a clean J/Psi
jpsiCuts = "(abs(B0_Jpsi_M - 3.09692) < 0.04)"
#: simple selection cuts to get a clean Xi
xiCuts = "(abs(Xi_M-1.3216)<0.01) && (Xi_Lambda0_Rho > 0.5)"

#: file that contains the histograms
outputFile = TFile.Open("standardParticlesValidation_ChargedPID.root", "RECREATE")

#: some coloring options
pid_colors = dict([("e", 41), ("mu", 46), ("p", 36), ('pi', 1), ('K', 29)])
#: pid cuts used as benchmark points
pid_vals = dict([("e", 0.5), ("mu", 0.5), ("p", 0.5), ('pi', 0.5), ('K', 0.5)])
#: efficiency reference targets
eff_vals = dict([("e", 0.9), ("mu", 0.9), ("p", 0.6), ('pi', 0.8), ('K', 0.8)])


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
#: dictionary for human readable PID branch names
variables = {
    'e': 'electronID', 'mu': 'muonID', 'pi': 'pionID',
    'K': 'kaonID', 'p': 'protonID',
}


def plot_pidEfficiency(pid, sample, vs='P', isExpertMode=False, detector=""):
    """
    Plots the efficiencies for a given sample for a all pid
    @param pid For which pid we would like to know efficiency.
    @param vs momentum (P) or angle (cosTheta)
    @param isExpertMode Should the plots be made for the default variables or expert ones?
    @param detector The PID subdetector to be used in expert mode (or ALL for combined)
    """
    metaOptions = "nostats"
    if isExpertMode:
        metaOptions += ", expert"
    if isExpertMode:
        pidString = "Expert_PID"
    else:
        pidString = "PID"
    if 'P' in vs:
        axisName, binN, binMin, binMax = vs + " GeV/c", 10, 0.0, 5.0
    if 'cosTheta' in vs:
        axisName, binN, binMin, binMax = vs, 10, -1.0, 1.0

    s = samples[sample]
    track = s.varname
    cuts = s.cuts
    pidcut = 0.5

    hist = dict()
    for histName in ['total', 'passed']:
        hist[histName] = TH1D(
            f"{pid}_{pidString}_{histName}_vs_{vs}",
            f"{pid}_{pidString}_{histName}_vs_{vs}",
            binN, binMin, binMax)

    if isExpertMode:
        selection = "(" + cuts + f") && ({track}_{pid}ExpertPID{detector} > {pidcut})"
    else:
        selection = "(" + cuts + f") && ({track}_{variables[pid]} > {pidcut})"

    s.tree.Project(hist['total'].GetName(), f"{track}_{vs}", cuts)
    s.tree.Project(hist['passed'].GetName(), f"{track}_{vs}", selection)
    h = TEfficiency(hist['passed'], hist['total'])
    h.SetName(f"{pid}_{pidString}_efficiency_vs_{vs}")
    h.SetTitle(f"{pid} {pidString} efficiency vs {vs} ({pidcut:.2f} PID cut);\
              {axisName};\
              efficiency")

    h.GetListOfFunctions().Add(TNamed("MetaOptions", metaOptions))
    h.GetListOfFunctions().Add(TNamed("Description", h.GetTitle()))
    h.GetListOfFunctions().Add(TNamed("Check", "Consistency between the different histograms"))
    h.GetListOfFunctions().Add(TNamed("Contact", "jan.strube@desy.de, dmitrii.neverov@desy.de"))
    outputFile.WriteTObject(h)
    # printout(h)


def plot_pidEfficienciesInSample(sample, isExpertMode=False, detector=""):
    """
    Plots the efficiencies for a given sample for a all pid
    @param sample The sample to plot
    @param isExpertMode Should the plots be made for the default variables or expert ones?
    @param detector The PID subdetector to be used in expert mode (or ALL for combined)
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
        f"{pidString}Eff_in_{sample}_sample",
        f"{pidString} efficiency in a {sample} sample ({pid_vals[sample]:.8f} PID cut for {eff_vals[sample]:.2f} efficiency);\
        ;\
        efficiency in {sample} sample",
        5, 0, 5)

    for bin, (pid, pidcut) in enumerate(pid_vals.items()):
        if isExpertMode:
            selection = "(" + cuts + f") && ({track}_{pid}ExpertPID{detector} > {pidcut})"
        else:
            selection = "(" + cuts + f") && ({track}_{variables[pid]} > {pidcut})"
        h.SetBinContent(
            bin + 1,
            # the default PID is unfortunately using lower case for the Kaon
            s.tree.GetEntries(selection) / total
        )
        h.GetXaxis().SetBinLabel(bin + 1, pid)
        h.GetListOfFunctions().Add(TNamed("MetaOptions", metaOptions))
        h.GetListOfFunctions().Add(TNamed("Description", h.GetTitle()))
        h.GetListOfFunctions().Add(TNamed("Check", "Consistency between the different histograms"))
        h.GetListOfFunctions().Add(TNamed("Contact", "jan.strube@desy.de, dmitrii.neverov@desy.de"))
    outputFile.WriteTObject(h)
    # printout(h)


def set_pidCutsForGivenEfficiency(pid, targetEff=0.5):
    """
    Sets the PID cuts for a given target efficiency.
    @param pid the PID variable
    @param targetEff the target efficiency
    """
    pid_vals[pid]
    step = 0.13
    currentEff = 1.1
    counter = 0

    s = samples[sample]
    track = s.varname
    cuts = s.cuts
    total = s.tree.GetEntries(cuts)

    while(abs(currentEff - targetEff) > 0.01 and counter < 100):
        step = step if step * (currentEff - targetEff) > 0 else -step / 2
        if 1.00 > pid_vals[pid] + step > 0.0:
            pid_vals[pid] += step
            currentEff = s.tree.GetEntries("(" + cuts + f") && ({track}_{variables[pid]} > {pid_vals[pid]})") / total
        else:
            step /= 2
        counter += 1


def printout(hist, postfix=""):
    """
    Save a histogram to a canvas
    @param hist the histogram to print out
    @param postfix an identifying string
    """
    canv = ROOT.TCanvas()
    canv.cd()
    hist.Draw()
    canv.SaveAs(f"{hist.GetName()}_{postfix}.png")


for detector in ("_ALL",):
    for sample in samples:
        # plot_pidEfficiency(sample, 'P', True, detector)
        # plot_pidEfficiency(sample, 'cosTheta', True, detector)
        plot_pidEfficiency(sample, sample, 'P')
        plot_pidEfficiency(sample, sample, 'cosTheta')
        set_pidCutsForGivenEfficiency(sample, eff_vals[sample])
        plot_pidEfficienciesInSample(sample, True, detector)
        plot_pidEfficienciesInSample(sample)
