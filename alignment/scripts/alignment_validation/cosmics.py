"""
Cosmics alignment validation: variable definitions, data loading, and validation run.
"""

import pathlib
from pathlib import Path

import ROOT
import numpy as np

import alignment_validation.plotting as plotting
from alignment_validation.variables import GlobalVariable, TrackVariable, s, cm, rad, unit, inverse_cm, gev
from alignment_validation.utils import get_variable_names, pseudomomentum, get_filter
from alignment_validation.plotting import (
    plot_histogram, plot_correlations, plot_2D_histogram, draw_map,
    plot_resolutions_hist, plot_resolution_comparison, plot_resolution,
)

# ---------------------------------------------------------------------------
# Variable definitions
# ---------------------------------------------------------------------------

#: Run number metadata.
run = GlobalVariable("run", "run", unit, "run")
#: Event time metadata.
time = GlobalVariable("evtT0", r"t$_{0}$", s, "time")
#: Track d0 metadata (track1/track2).
d = TrackVariable("D01", "D02", r"d$_{0}$", cm, "d")
#: Track z0 metadata (track1/track2).
z = TrackVariable("Z01", "Z02", r"z$_{0}$", cm, "z")
#: Track phi0 metadata (track1/track2).
phi = TrackVariable("Phi01", "Phi02", r"$\Phi_{0}$", rad, "phi")
#: Track tan(lambda) metadata (track1/track2).
tanLambda = TrackVariable("tanLambda1", "tanLambda2", r"$\tan(\lambda$)", unit, "tanLambda")
#: Track omega metadata (track1/track2).
omega = TrackVariable("Omega1", "Omega2", r"$\omega$", inverse_cm, "omega")
#: Track transverse momentum metadata (track1/track2).
pt = TrackVariable("Pt1", "Pt2", r"$P_{t}$", gev, "pt")

#: Default ROOT selection for cosmics tracks.
SELECTION = (
    "run>=0"
    " && abs(D01)<1 && abs(D02)<1"
    " && Z01>-2 && Z02>-2 && Z01<4 && Z02<4"
    " && abs(D01-D02)<0.2 && abs(Z01-Z02)<0.2"
)

# ---------------------------------------------------------------------------
# Data loading
# ---------------------------------------------------------------------------


def load_data(filenames: list, selection: str = SELECTION) -> dict:
    """Read cosmic-track ROOT files and return filtered data arrays keyed by filename.

    Parameters
    ----------
    filenames : list of str
        Paths to the input ROOT files. Each file must contain a TTree named
        ``"tree"`` with the branches defined by the module-level variable list.
    selection : str, optional
        ROOT/RDataFrame filter string applied before reading the data.
        Defaults to :data:`SELECTION`.

    Returns
    -------
    dict
        ``{filename: {branch_name: numpy_array}}`` with one entry per file.
        Only events passing ``selection`` are included.
    """
    print("Loading cosmic data.")
    all_vars = [run, time, d, z, phi, tanLambda, omega, pt]
    branch_names = get_variable_names(all_vars)
    data = {}
    for file in filenames:
        print(f"Reading {file}")
        tfile = ROOT.TFile(file, "OPEN")
        df = ROOT.RDataFrame("tree", tfile).Filter(selection)
        data[file] = df.AsNumpy(columns=branch_names)
        print(
            f"Number of events after applying selection in {file}"
            f" is: {len(data[file][branch_names[0]])}"
        )
    return data

# ---------------------------------------------------------------------------
# Validation run
# ---------------------------------------------------------------------------


def run_validation(filenames: list, output_dir: str, file_format: str = "pdf"):
    """Load cosmics data and produce all validation plots.

    Produces the following sets of plots in ``output_dir``:

    - Per-variable histograms and track1 − track2 difference histograms.
    - Correlation profiles (median and sigma68 vs each track parameter).
    - 2D detector maps (phi vs tan(lambda)) of median and resolution for d0 and z0,
      using Delta mode (track1 - track2) for both variables.
    - Resolution histograms per dataset and a multi-dataset comparison.
    - Resolution vs pseudomomentum for d0, z0, phi0, tan(lambda).
    - Pt resolution vs Pt.

    Parameters
    ----------
    filenames : list of str
        Paths to the input ROOT files.
    output_dir : str
        Directory where all plots are saved. Created if it does not exist.
    file_format : str, optional
        Image format passed to matplotlib (e.g. ``"pdf"``, ``"png"``).
        Default is ``"pdf"``.
    """
    pathlib.Path(output_dir).mkdir(parents=True, exist_ok=True)
    plotting.output_dir = output_dir
    plotting.file_format = file_format

    labels = [Path(f).stem for f in filenames]
    data = load_data(filenames)

    # -----------------------------------------------------------------------
    # Histograms
    # -----------------------------------------------------------------------
    print("Making histograms")

    for var in [d, z, omega, pt]:
        plot_histogram(
            [[*data[f][var.name1], *data[f][var.name2]] for f in filenames],
            labels, var.plaintext, f"{var.latex} {var.unit.name}",
        )

    for var in [phi, tanLambda]:
        plot_histogram(
            [[*data[f][var.name1], *data[f][var.name2]] for f in filenames],
            labels, var.plaintext, f"{var.latex} {var.unit.name}", range=100,
        )

    for var in [run, time]:
        plot_histogram(
            [data[f][var.name] for f in filenames],
            labels, var.plaintext, f"{var.latex} {var.unit.name}",
        )

    print("Making track1 - track2 histograms")
    for var in [d, z, phi, tanLambda, omega, pt]:
        plot_histogram(
            [(data[f][var.name1] - data[f][var.name2]) / 2**0.5 for f in filenames],
            labels, "delta-" + var.plaintext, f"$\\Delta${var.latex} {var.unit.name}",
        )

    # -----------------------------------------------------------------------
    # Correlations
    # -----------------------------------------------------------------------
    print("Making correlations")

    xdata = {f: {var: np.array([*data[f][var.name1], *data[f][var.name2]])
                 for var in [d, z, phi, tanLambda, omega]}
             for f in filenames}
    ydata = {f: {var: np.array([*(data[f][var.name1] - data[f][var.name2]) / 2**0.5 * var.unit.convert,
                                *(data[f][var.name1] - data[f][var.name2]) / 2**0.5 * var.unit.convert])
                 for var in [d, z, phi, tanLambda, omega]}
             for f in filenames}
    xlabels = {var: var.latex + var.unit.name for var in [d, z, phi, tanLambda, omega]}
    ylabels = {var: r"$\Delta$" + var.latex + var.unit.dname for var in [d, z, phi, tanLambda, omega]}

    plot_correlations(
        'median',
        [xdata[f] for f in filenames],
        [ydata[f] for f in filenames],
        [xlabels[var] for var in xlabels],
        [ylabels[var] for var in ylabels],
        labels, nbins=15, figsize=(10.0, 7.5),
    )
    plot_correlations(
        'resolution',
        [xdata[f] for f in filenames],
        [ydata[f] for f in filenames],
        [xlabels[var] for var in xlabels],
        [r"$\sigma_{{68}} ({})$".format(ylabels[var].replace(var.unit.dname, "").replace("$", " "))
         + "\n" + var.unit.dname for var in ylabels],
        labels, nbins=15, figsize=(10.0, 7.5), make_2D_hist=False,
    )

    # -----------------------------------------------------------------------
    # Detector maps
    # -----------------------------------------------------------------------
    print("Making maps")
    map_bins = (80, 80)

    for f in filenames:
        label = Path(f).stem
        plot_2D_histogram(data[f], label, map_bins, phi, tanLambda)
        for var in [d, z]:
            draw_map('median', data[f], label, var, 'delta', map_bins, phi, tanLambda)
            draw_map('resolution', data[f], label, var, 'delta', map_bins, phi, tanLambda)

    # -----------------------------------------------------------------------
    # Resolutions
    # -----------------------------------------------------------------------
    print("Making resolutions")
    resolutions_cut = {}

    resolutions_data = {}
    resolutions_labels = {}
    for f in filenames:
        resolutions_data[f] = {}
        mask = get_filter(data[f], resolutions_cut)
        for var in [d, z, phi, tanLambda, omega]:
            resolutions_data[f][var] = (
                (data[f][var.name1][mask] - data[f][var.name2][mask]) / 2**0.5 * var.unit.convert
            )
            resolutions_labels[var] = r"$\Delta$" + var.latex + var.unit.dname
        resolutions_data[f][pt] = (
            (data[f][pt.name1][mask] - data[f][pt.name2][mask]) /
            (data[f][pt.name1][mask] + data[f][pt.name2][mask]) / 2**0.5
        )
        resolutions_labels[pt] = r"$\frac{\Delta P_{t}}{\overline{P_{t}}}$ [1]"

        plot_resolutions_hist(
            f"Resolutions {Path(f).stem}",
            resolutions_data[f], resolutions_labels,
            nbins=40, vars_to_fit=resolutions_data[f].keys(),
        )

    plot_resolution_comparison(
        "Resolutions",
        [resolutions_data[f] for f in filenames],
        labels, resolutions_labels,
        nbins=40, figsize=(13.0, 8.0),
    )

    # -----------------------------------------------------------------------
    # Resolution vs pseudomomentum
    # -----------------------------------------------------------------------
    print("Making resolution vs pseudomomentum")

    pseudomom3 = [np.array([
        *pseudomomentum(data[f][pt.name1], data[f][tanLambda.name1], 3),
        *pseudomomentum(data[f][pt.name2], data[f][tanLambda.name2], 3),
    ]) for f in filenames]
    pseudomom5 = [np.array([
        *pseudomomentum(data[f][pt.name1], data[f][tanLambda.name1], 5),
        *pseudomomentum(data[f][pt.name2], data[f][tanLambda.name2], 5),
    ]) for f in filenames]

    pm_xlimit = [0, 10]
    pm_ylimits = {d: [0, 50], z: [0, 100], phi: [0, 3], tanLambda: [0, 4]}
    pm_bins = [0.5, 1, 1.5, 2, 2.5, 3, 4, 5, 6, 7, 8, 9, 10]

    pm_res_data = {}
    pm_res_labels = {}

    for var in [d, phi]:
        ydata = [
            np.ndarray.flatten(np.array([
                *(data[f][var.name1] - data[f][var.name2]) / 2**0.5 * var.unit.convert,
                *(data[f][var.name1] - data[f][var.name2]) / 2**0.5 * var.unit.convert,
            ]))
            for f in filenames
        ]
        pm_res_data[var] = [[pseudomom5[i], ydata[i]] for i in range(len(filenames))]
        pm_res_labels[var] = [
            r"p$\beta$" + r"(sin$\theta$)$^{5/2}$ [GeV/c]",
            r"$\sigma_{{68}}(\frac{{\Delta {}}}{{\sqrt{{2}}}})$".format(var.latex.replace("$", "")) + f"{var.unit.dname}",
        ]

    for var in [z, tanLambda]:
        ydata = [
            np.ndarray.flatten(np.array([
                *(data[f][var.name1] - data[f][var.name2]) / 2**0.5 * var.unit.convert,
                *(data[f][var.name1] - data[f][var.name2]) / 2**0.5 * var.unit.convert,
            ]))
            for f in filenames
        ]
        pm_res_data[var] = [[pseudomom3[i], ydata[i]] for i in range(len(filenames))]
        pm_res_labels[var] = [
            r"p$\beta$" + r"(sin$\theta$)$^{3/2}$ [GeV/c]",
            r"$\sigma_{{68}}(\frac{{\Delta {}}}{{\sqrt{{2}}}})$".format(var.latex.replace("$", "")) + f"{var.unit.dname}",
        ]

    def resolutionfit(x, a, b):
        return (a**2 + (b / x)**2)**0.5

    def resolutionfitlabel(fit, err):
        return (r"y=$\sqrt{a^{2} +b^{2}/x^{2}} $" + "\n"
                + f"a = ({fit[0]:.3f}" + r" $\pm$ " + f"{err[0]:.3f})" + r"[$\mu$m]" + "\n"
                + f"b = ({fit[1]:.3f}" + r" $\pm$ " + f"{err[1]:.3f})" + r"[$\mu$m GeV/c]")

    plot_resolution(
        "Resolution vs Pseudomomentum",
        {var: pm_res_data[var] for var in [d, z]},
        labels, pm_res_labels, pm_xlimit, pm_ylimits, pm_bins,
        resolutionfit, resolutionfitlabel, fitrange=[0.01, pm_xlimit[1]],
    )
    plot_resolution(
        "Resolution vs Pseudomomentum",
        {var: pm_res_data[var] for var in [phi, tanLambda]},
        labels, pm_res_labels, pm_xlimit, pm_ylimits, pm_bins,
        resolutionfit, resolutionfitlabel, fitrange=[0.01, pm_xlimit[1]],
    )

    # -----------------------------------------------------------------------
    # Pt resolution vs pt
    # -----------------------------------------------------------------------
    print("Making pt resolution vs pt")

    xdata_pt = [np.array([*data[f][pt.name1], *data[f][pt.name2]]) for f in filenames]
    ydata_pt = [
        np.ndarray.flatten(np.array([
            *(data[f][pt.name1] - data[f][pt.name2]) / data[f][pt.name1] * 100,
            *(data[f][pt.name1] - data[f][pt.name2]) / data[f][pt.name2] * 100,
        ]))
        for f in filenames
    ]
    pt_res_data = {pt: [[xdata_pt[i], ydata_pt[i]] for i in range(len(filenames))]}
    pt_res_labels = {pt: [
        f"{pt.latex}{pt.unit.name}",
        r"$\sigma_{{68}}(\frac{{\Delta {}}}{{{}}})$".format(
            pt.latex.replace("$", ""), pt.latex.replace("$", "")) + "[%]",
    ]}

    def ptfit(x, a, b):
        return ((x * a)**2 + b**2)**0.5

    def ptfitlabel(fit, err):
        return (r"y=$\sqrt{A^{2} x^{2}+B^{2}} $" + "\n"
                + f"A = ({fit[0]:.3f}" + r" $\pm$ " + f"{err[0]:.3f})" + "[% c/GeV]" + "\n"
                + f"B = ({fit[1]:.3f}" + r" $\pm$ " + f"{err[1]:.3f})" + "[%]")

    plot_resolution(
        "Pt resolution vs Pt",
        pt_res_data, labels, pt_res_labels,
        xlimit=[0, 10], ylimits={pt: [0, 2]},
        bins=[1, 1.5, 2, 2.5, 3, 4, 5, 6, 7, 8, 9, 10],
        fitfunction=ptfit, fitlabel=ptfitlabel,
    )
