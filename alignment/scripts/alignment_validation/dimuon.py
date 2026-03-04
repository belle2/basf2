##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
"""
Dimuon alignment validation: variable definitions, data loading, and validation run.
"""

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
# Variable definitions (CMS frame)
# ---------------------------------------------------------------------------

#: Run number metadata.
run = GlobalVariable("__run__", "run", unit, "run")
#: Event time metadata.
time = GlobalVariable("eventTimeSeconds", r"t$_{0}$", s, "time")
#: Invariant mass metadata.
InvM = GlobalVariable("InvM", r"$M_{inv}$", gev, "InvM")

#: Track d0 metadata (track1/track2).
d = TrackVariable(
    "useCMSFrame__bodaughter__bo0__cm__spd0__bc__bc",
    "useCMSFrame__bodaughter__bo1__cm__spd0__bc__bc",
    r"d$_{0}$", cm, "d",
)
#: Track z0 metadata (track1/track2).
z = TrackVariable(
    "useCMSFrame__bodaughter__bo0__cm__spz0__bc__bc",
    "useCMSFrame__bodaughter__bo1__cm__spz0__bc__bc",
    r"z$_{0}$", cm, "z",
)
#: Track phi0 metadata (track1/track2).
phi = TrackVariable(
    "useCMSFrame__bodaughter__bo0__cm__spphi0__bc__bc",
    "useCMSFrame__bodaughter__bo1__cm__spphi0__bc__bc",
    r"$\Phi_{0}$", rad, "phi",
)
#: Track tan(lambda) metadata (track1/track2).
tanLambda = TrackVariable(
    "useCMSFrame__bodaughter__bo0__cm__sptanLambda__bc__bc",
    "useCMSFrame__bodaughter__bo1__cm__sptanLambda__bc__bc",
    r"tan($\lambda$)", unit, "tanLambda",
)
#: Track omega metadata (track1/track2).
omega = TrackVariable(
    "useCMSFrame__bodaughter__bo0__cm__spomega__bc__bc",
    "useCMSFrame__bodaughter__bo1__cm__spomega__bc__bc",
    r"$\omega$", inverse_cm, "omega",
)
#: Track transverse momentum metadata (track1/track2).
pt = TrackVariable(
    "useCMSFrame__bodaughter__bo0__cm__sppt__bc__bc",
    "useCMSFrame__bodaughter__bo1__cm__sppt__bc__bc",
    r"$P_{t}$", gev, "pt",
)

#: Default ROOT selection for dimuon tracks.
SELECTION = (
    "__run__>=0"
    " && daughter__bo0__cm__spmuonID__bc>=0.8"
    " && daughter__bo1__cm__spmuonID__bc>=0.8"
)

# ---------------------------------------------------------------------------
# IP correction
# ---------------------------------------------------------------------------


def fix_ip_location(data: dict):
    """Refit helix parameters relative to the per-event IP position for all dimuon events.

    Modifies ``data`` in-place. For each event the helix of both tracks is
    passively moved to the per-event IP (stored in branches ``IPX``, ``IPY``,
    ``IPZ``), and the five helix parameters (d0, phi0, omega, z0, tanLambda)
    are updated accordingly.

    Parameters
    ----------
    data : dict
        ``{filename: {branch_name: numpy_array}}`` as returned by
        :func:`load_data`. Must contain ``IPX``, ``IPY``, ``IPZ`` arrays.
    """
    print("Recalculating IP location.")
    for file in data:
        for i, _ in enumerate(data[file][time.name]):
            helix1 = ROOT.Belle2.Helix(
                data[file][d.name1][i], data[file][phi.name1][i],
                data[file][omega.name1][i], data[file][z.name1][i],
                data[file][tanLambda.name1][i],
            )
            helix2 = ROOT.Belle2.Helix(
                data[file][d.name2][i], data[file][phi.name2][i],
                data[file][omega.name2][i], data[file][z.name2][i],
                data[file][tanLambda.name2][i],
            )
            helix1.passiveMoveBy(data[file]["IPX"][i], data[file]["IPY"][i], data[file]["IPZ"][i])
            helix2.passiveMoveBy(data[file]["IPX"][i], data[file]["IPY"][i], data[file]["IPZ"][i])

            data[file][d.name1][i] = helix1.getD0()
            data[file][d.name2][i] = helix2.getD0()
            data[file][z.name1][i] = helix1.getZ0()
            data[file][z.name2][i] = helix2.getZ0()
            data[file][tanLambda.name1][i] = helix1.getTanLambda()
            data[file][tanLambda.name2][i] = helix2.getTanLambda()
            data[file][phi.name1][i] = helix1.getPhi0()
            data[file][phi.name2][i] = helix2.getPhi0()
            data[file][omega.name1][i] = helix1.getOmega()
            data[file][omega.name2][i] = helix2.getOmega()

            if i % 10000 == 0:
                print(f"Processing event: {i} of {len(data[file][d.name1])}.", end="\r", flush=True)
        if len(data[file][d.name1]) > 0:
            print(f"Processing event: {i+1} of {len(data[file][d.name1])}.", flush=True)


def fix_ip_location_run_by_run(data: dict):
    """Refit helix parameters using the per-run mean IP position.

    Modifies ``data`` in-place. For each unique (experiment, run) pair the
    mean IP position is computed from the per-event ``IPX``, ``IPY``, ``IPZ``
    values. All events in that run are then refitted using this mean IP,
    rather than the individual per-event IP. This is more robust when the
    per-event IP has large statistical fluctuations.

    Parameters
    ----------
    data : dict
        ``{filename: {branch_name: numpy_array}}`` as returned by
        :func:`load_data`. Must contain ``IPX``, ``IPY``, ``IPZ``, and
        ``__experiment__`` arrays.
    """
    print("Recalculating IP location (run-by-run).")
    for file in data:
        experiments = data[file]["__experiment__"]
        runs = data[file][run.name]
        unique_pairs = np.unique(np.stack([experiments, runs], axis=1), axis=0)
        for pair in unique_pairs:
            exp, run_num = pair[0], pair[1]
            mask = (experiments == exp) & (runs == run_num)
            mean_ipx = float(np.mean(data[file]["IPX"][mask]))
            mean_ipy = float(np.mean(data[file]["IPY"][mask]))
            mean_ipz = float(np.mean(data[file]["IPZ"][mask]))
            n_events = int(np.sum(mask))
            print(
                f"Experiment {exp}, run {run_num}: {n_events} events, "
                f"mean IP = ({mean_ipx:.4f}, {mean_ipy:.4f}, {mean_ipz:.4f})"
            )
            indices = np.where(mask)[0]
            for i in indices:
                helix1 = ROOT.Belle2.Helix(
                    data[file][d.name1][i], data[file][phi.name1][i],
                    data[file][omega.name1][i], data[file][z.name1][i],
                    data[file][tanLambda.name1][i],
                )
                helix2 = ROOT.Belle2.Helix(
                    data[file][d.name2][i], data[file][phi.name2][i],
                    data[file][omega.name2][i], data[file][z.name2][i],
                    data[file][tanLambda.name2][i],
                )
                helix1.passiveMoveBy(mean_ipx, mean_ipy, mean_ipz)
                helix2.passiveMoveBy(mean_ipx, mean_ipy, mean_ipz)

                data[file][d.name1][i] = helix1.getD0()
                data[file][d.name2][i] = helix2.getD0()
                data[file][z.name1][i] = helix1.getZ0()
                data[file][z.name2][i] = helix2.getZ0()
                data[file][tanLambda.name1][i] = helix1.getTanLambda()
                data[file][tanLambda.name2][i] = helix2.getTanLambda()
                data[file][phi.name1][i] = helix1.getPhi0()
                data[file][phi.name2][i] = helix2.getPhi0()
                data[file][omega.name1][i] = helix1.getOmega()
                data[file][omega.name2][i] = helix2.getOmega()

# ---------------------------------------------------------------------------
# Data loading
# ---------------------------------------------------------------------------


def load_data(filenames: list, selection: str = SELECTION) -> dict:
    """Read dimuon ROOT files and return filtered data arrays keyed by filename.

    No IP correction is applied here; call :func:`fix_ip_location` or
    :func:`fix_ip_location_run_by_run` afterwards if needed.

    Parameters
    ----------
    filenames : list of str
        Paths to the input ROOT files. Each file must contain a TTree named
        ``"variables"`` with the branches defined by the module-level variable
        list, plus ``IPX``, ``IPY``, ``IPZ``, and ``__experiment__``.
    selection : str, optional
        ROOT/RDataFrame filter string applied before reading the data.
        Defaults to :data:`SELECTION`.

    Returns
    -------
    dict
        ``{filename: {branch_name: numpy_array}}`` with one entry per file.
        Only events passing ``selection`` are included.
    """
    print("Loading dimuon data.")
    all_vars = [run, time, d, z, phi, tanLambda, omega, pt, InvM]
    branch_names = get_variable_names(all_vars) + ["IPX", "IPY", "IPZ", "__experiment__"]
    data = {}
    for file in filenames:
        print(f"Reading {file}")
        tfile = ROOT.TFile(file, "OPEN")
        df = ROOT.RDataFrame("variables", tfile).Filter(selection)
        data[file] = df.AsNumpy(columns=branch_names)
        print(
            f"Number of events after applying selection in {file}"
            f" is: {len(data[file][branch_names[0]])}"
        )
    return data

# ---------------------------------------------------------------------------
# Validation run
# ---------------------------------------------------------------------------


def run_validation(filenames: list, output_dir: str, file_format: str = "pdf",
                   ip_correction: str = "run_by_run"):
    """Load dimuon data and produce all validation plots.

    Produces the following sets of plots in ``output_dir``:

    - Per-variable histograms, track1 − track2 difference and track1 + track2
      sum histograms.
    - Di-muon invariant mass fit (Crystal Ball + exponential background).
    - Correlation profiles (median and sigma68 of d0 sum / z0 difference vs
      phi and tan(lambda)).
    - 2D detector maps (phi vs tan(lambda)) of median and resolution for d0
      and z0, using Sigma mode (track1 + track2) for d0 and Delta mode
      (track1 - track2) for z0.
    - Resolution histograms per dataset and a multi-dataset comparison.
    - Resolution vs pseudomomentum for d0 and z0.
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
    ip_correction : str, optional
        IP correction strategy to apply after loading the data.
        ``"run_by_run"`` (default) uses the mean IP per (experiment, run) pair
        via :func:`fix_ip_location_run_by_run`; ``"per_event"`` uses the
        individual per-event IP via :func:`fix_ip_location`.
    """
    import alignment_validation.fit_dimuon_mass as fit_dimuon_mass  # requires Belle II environment

    Path(output_dir).mkdir(parents=True, exist_ok=True)
    plotting.output_dir = output_dir
    plotting.file_format = file_format

    labels = [Path(f).stem for f in filenames]
    data = load_data(filenames)

    if ip_correction == "run_by_run":
        fix_ip_location_run_by_run(data)
    elif ip_correction == "per_event":
        fix_ip_location(data)
    else:
        raise ValueError(
            f"Unknown ip_correction: {ip_correction!r}. Use 'run_by_run' or 'per_event'."
        )

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

    plot_histogram(
        [data[f][InvM.name] for f in filenames],
        labels, InvM.plaintext, f"{InvM.latex} {InvM.unit.name}",
    )

    print("Fitting dimuon mass")
    for f in filenames:
        result = fit_dimuon_mass.fit(
            data[f][InvM.name],
            savefig=f"{output_dir}/dimuon_fit_{Path(f).stem}.{file_format}",
        )
        print(result)

    print("Making track1 - track2 histograms")
    for var in [z]:
        plot_histogram(
            [(data[f][var.name1] - data[f][var.name2]) / 2**0.5 for f in filenames],
            labels, "delta-" + var.plaintext, f"$\\Delta${var.latex} {var.unit.name}",
        )

    print("Making track1 + track2 histograms")
    for var in [d]:
        plot_histogram(
            [(data[f][var.name1] + data[f][var.name2]) / 2**0.5 for f in filenames],
            labels, "sigma-" + var.plaintext, f"$\\Sigma${var.latex} {var.unit.name}",
        )

    # -----------------------------------------------------------------------
    # Correlations
    # -----------------------------------------------------------------------
    print("Making correlations")

    xdata = {f: {
        phi: np.array([*data[f][phi.name1], *data[f][phi.name2]]),
        tanLambda: np.array([*data[f][tanLambda.name1], *data[f][tanLambda.name2]]),
    } for f in filenames}

    ydata = {f: {
        d: np.array([*(data[f][d.name1] + data[f][d.name2]) / 2**0.5 * d.unit.convert,
                     *(data[f][d.name1] + data[f][d.name2]) / 2**0.5 * d.unit.convert]),
        z: np.array([*(data[f][z.name1] - data[f][z.name2]) / 2**0.5 * z.unit.convert,
                     *(data[f][z.name1] - data[f][z.name2]) / 2**0.5 * z.unit.convert]),
    } for f in filenames}

    xlabels = {phi: phi.latex + phi.unit.name, tanLambda: tanLambda.latex + tanLambda.unit.name}
    ylabels = {d: r"$\Sigma$" + d.latex + d.unit.dname, z: r"$\Delta$" + z.latex + z.unit.dname}

    plot_correlations(
        'median',
        [xdata[f] for f in filenames],
        [ydata[f] for f in filenames],
        [xlabels[var] for var in xlabels],
        [ylabels[var] for var in ylabels],
        labels, nbins=15, figsize=(6, 5),
    )
    plot_correlations(
        'resolution',
        [xdata[f] for f in filenames],
        [ydata[f] for f in filenames],
        [xlabels[var] for var in xlabels],
        [r"$\sigma_{{68}} ({})$".format(ylabels[var].replace(var.unit.dname, "").replace("$", " "))
         + "\n" + var.unit.dname for var in ylabels],
        labels, nbins=15, figsize=(6, 5), make_2D_hist=False,
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
            mode = 'sigma' if var is d else 'delta'
            draw_map('median', data[f], label, var, mode, map_bins, phi, tanLambda)
            draw_map('resolution', data[f], label, var, mode, map_bins, phi, tanLambda)

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
        resolutions_data[f][d] = (data[f][d.name1][mask] + data[f][d.name2][mask]) / 2**0.5 * d.unit.convert
        resolutions_labels[d] = r"$\Sigma$" + d.latex + d.unit.dname
        resolutions_data[f][z] = (data[f][z.name1][mask] - data[f][z.name2][mask]) / 2**0.5 * z.unit.convert
        resolutions_labels[z] = r"$\Delta$" + z.latex + z.unit.dname

        plot_resolutions_hist(
            f"Resolutions {Path(f).stem}",
            resolutions_data[f], resolutions_labels,
            nbins=40, vars_to_fit=resolutions_data[f].keys(),
            shape=(1, 2), figsize=(11.0, 5.0),
        )

    plot_resolution_comparison(
        "Resolutions",
        [resolutions_data[f] for f in filenames],
        labels, resolutions_labels,
        nbins=40, shape=(1, 2), figsize=(11.0, 5.0),
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

    pm_xlimit = [0, 5]
    pm_ylimits = {d: [0, 50], z: [0, 100]}
    pm_bins = [2, 2.5, 3, 3.5, 4, 4.2, 4.4, 4.6, 4.8, 5, 6]

    pm_res_data = {}
    pm_res_labels = {}

    # d uses SUM for dimuon
    ydata_d = [
        np.ndarray.flatten(np.array([
            *(data[f][d.name1] + data[f][d.name2]) / 2**0.5 * d.unit.convert,
            *(data[f][d.name1] + data[f][d.name2]) / 2**0.5 * d.unit.convert,
        ]))
        for f in filenames
    ]
    pm_res_data[d] = [[pseudomom5[i], ydata_d[i]] for i in range(len(filenames))]
    pm_res_labels[d] = [
        r"p$\beta$" + r"(sin$\theta$)$^{5/2}$ [GeV/c]",
        r"$\sigma_{{68}}(\frac{{\Sigma {}}}{{\sqrt{{2}}}})$".format(d.latex.replace("$", "")) + f"{d.unit.dname}",
    ]

    ydata_z = [
        np.ndarray.flatten(np.array([
            *(data[f][z.name1] - data[f][z.name2]) / 2**0.5 * z.unit.convert,
            *(data[f][z.name1] - data[f][z.name2]) / 2**0.5 * z.unit.convert,
        ]))
        for f in filenames
    ]
    pm_res_data[z] = [[pseudomom3[i], ydata_z[i]] for i in range(len(filenames))]
    pm_res_labels[z] = [
        r"p$\beta$" + r"(sin$\theta$)$^{3/2}$ [GeV/c]",
        r"$\sigma_{{68}}(\frac{{\Delta {}}}{{\sqrt{{2}}}})$".format(z.latex.replace("$", "")) + f"{z.unit.dname}",
    ]

    def resolutionfit(x, a, b):
        return (a**2 + (b / x)**2)**0.5

    def resolutionfitlabel(fit, err):
        return (r"y=$\sqrt{a^{2} +b^{2}/x^{2}} $" + "\n"
                + f"a = ({fit[0]:.3f}" + r" $\pm$ " + f"{err[0]:.3f})" + r"[$\mu$m]" + "\n"
                + f"b = ({fit[1]:.3f}" + r" $\pm$ " + f"{err[1]:.3f})" + r"[$\mu$m GeV/c]")

    plot_resolution(
        "Resolution vs Pseudomomentum",
        pm_res_data, labels, pm_res_labels,
        pm_xlimit, pm_ylimits, pm_bins,
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
        xlimit=[2, 5.5], ylimits={pt: [0, 9]},
        bins=[2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5],
        fitfunction=ptfit, fitlabel=ptfitlabel,
    )
