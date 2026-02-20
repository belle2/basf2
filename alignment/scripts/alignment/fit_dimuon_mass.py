#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
"""
Unbinned maximum-likelihood fit of the di-muon invariant mass.
Model: Crystal Ball (signal peak) + Exponential (ISR / continuum background)

Can be used as a script::

    python3 fit_dimuon_mass.py

or imported and called programmatically::

    from fit_dimuon_mass import fit
    result = fit(data_array)
"""

import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import minimize
from scipy.special import erf
from scipy.stats import chi2 as chi2_dist
from numpy.linalg import inv

# ─── Default fit range & binning ──────────────────────────────────────────────
MASS_MIN = 10.2   # GeV/c^2
MASS_MAX = 10.8
NBINS = 100

# ─── PDF components (pure functions, no state) ────────────────────────────────


def _cb_norm(mu, sigma, alpha, n, a, b):
    """Closed-form integral of the Crystal Ball function over [a, b].

    The power-law tail is on the LOW-MASS side:
    x_break = mu - alpha * sigma.
    """
    xb = mu - alpha * sigma
    A = (n / alpha) ** n * np.exp(-0.5 * alpha**2)
    B = n / alpha - alpha   # > 0 for alpha > 0

    # Gaussian piece: [max(a, xb), b]
    lo = max(a, xb)
    if lo < b:
        s2 = sigma * np.sqrt(2.0)
        n_g = sigma * np.sqrt(np.pi / 2.0) * (
            erf((b - mu) / s2) - erf((lo - mu) / s2)
        )
    else:
        n_g = 0.0

    # Power-law piece: [a, min(b, xb)]
    hi = min(b, xb)
    if a < hi and n > 1.0:
        t_lo = (a - mu) / sigma
        t_hi = (hi - mu) / sigma   # = -alpha at the break
        # ∫ A*(B-t)^{-n} sigma dt = sigma*A/(n-1) * [(B-t)^{1-n}]_{t_lo}^{t_hi}
        n_p = sigma * A / (n - 1.0) * (
            (B - t_hi) ** (1.0 - n) - (B - t_lo) ** (1.0 - n)
        )
    else:
        n_p = 0.0

    return n_g + n_p


def _exp_norm(slope, a, b, x0):
    """Integral of exp(slope*(x - x0)) over [a, b]."""
    if abs(slope) < 1e-12:
        return b - a
    return (np.exp(slope * (b - x0)) - np.exp(slope * (a - x0))) / slope


def _cb(x, mu, sigma, alpha, n):
    """Crystal Ball (unnormalised): Gaussian core + power-law tail on the left."""
    t = (x - mu) / sigma
    A = (n / alpha) ** n * np.exp(-0.5 * alpha**2)
    B = n / alpha - alpha
    # Guard against overflow: np.where pre-evaluates both branches, so
    # substitute a safe dummy in the Gaussian region before taking the power.
    power_arg = np.where(t < -alpha, B - t, n / alpha)
    return np.where(
        t >= -alpha,
        np.exp(-0.5 * t**2),
        A * np.power(power_arg, -n),
    )


def _exp(x, slope, x0):
    return np.exp(slope * (x - x0))


def _hessian(f, x0, eps=1e-5):
    """Symmetric finite-difference Hessian (n + n*(n+1)/2 + 1 evaluations)."""
    nd = len(x0)
    f0 = f(x0)
    fi = [f(x0 + eps * np.eye(nd)[i]) for i in range(nd)]
    H = np.zeros((nd, nd))
    for i in range(nd):
        for j in range(i, nd):
            xij = x0.copy()
            xij[i] += eps
            xij[j] += eps
            H[i, j] = (f(xij) - fi[i] - fi[j] + f0) / eps**2
            H[j, i] = H[i, j]
    return H


# ─── Main fitting function ────────────────────────────────────────────────────
def fit(data, mass_min=MASS_MIN, mass_max=MASS_MAX, nbins=NBINS,
        savefig="dimuon_fit.pdf", title=r"Di-muon invariant mass — $e^+e^- \to \mu^+\mu^-$ (Belle II data)"):
    """Fit the di-muon invariant mass with Crystal Ball + exponential background.

    Parameters
    ----------
    data : array-like
        Raw invariant mass values in GeV/c^2 (range selection is applied internally).
    mass_min, mass_max : float
        Fit range in GeV/c^2.
    nbins : int
        Number of histogram bins used for the chi^2/NDF and the plot.
    savefig : str or None
        File name for the output plot.  Pass ``None`` to skip saving.

    Returns
    -------
    dict with keys:
        pfit   – array [mu, sigma, alpha, n, slope, f_sig]
        errors – array of 1-sigma uncertainties (from Hessian)
        chi2   – chi^2 value (binned, bins with expected > 5)
        ndf    – degrees of freedom
        pval   – p-value
        fig    – matplotlib Figure
    """
    data = np.asarray(data)
    data = data[(data >= mass_min) & (data <= mass_max)]
    N = len(data)
    bw = (mass_max - mass_min) / nbins
    x0 = (mass_min + mass_max) / 2.0   # exponential pivot

    print(f"Events in fit range [{mass_min}, {mass_max}] GeV/c^2: {N:,}")

    # ── NLL (closes over data, x0, mass_min, mass_max) ────────────────────────
    def nll(p):
        mu, sigma, alpha, n, slope, fsig = p
        if sigma <= 0 or alpha <= 0 or n <= 1.01 or not (0.0 < fsig < 1.0):
            return 1e15
        cb_n = _cb_norm(mu, sigma, alpha, n, mass_min, mass_max)
        exp_n = _exp_norm(slope, mass_min, mass_max, x0)
        if cb_n <= 0 or exp_n <= 0:
            return 1e15
        pdf = (fsig * _cb(data, mu, sigma, alpha, n) / cb_n +
               (1.0 - fsig) * _exp(data, slope, x0) / exp_n)
        if np.any(pdf <= 0):
            return 1e15
        return -np.sum(np.log(pdf))

    # ── Minimisation ──────────────────────────────────────────────────────────
    #          mu       sigma   alpha    n    slope  fsig
    p0 = [10.575,  0.040,  1.5,    5.0,  3.0,  0.65]
    bounds = [(10.50, 10.65), (0.005, 0.10), (0.1, 5.0),
              (1.1, 20.0), (-10.0, 10.0), (0.01, 0.999)]

    print("Minimising NLL ...")
    res = minimize(nll, p0, method="L-BFGS-B", bounds=bounds,
                   options={"maxiter": 10_000, "ftol": 1e-14, "gtol": 1e-8})
    if not res.success:
        print("  L-BFGS-B did not converge; switching to Nelder-Mead ...")
        res2 = minimize(nll, p0, method="Nelder-Mead",
                        options={"maxiter": 500_000, "xatol": 1e-8,
                                 "fatol": 1e-8, "adaptive": True})
        if res2.fun < res.fun:
            res = res2

    pfit = res.x
    mu_f, sig_f, alp_f, n_f, sl_f, fs_f = pfit
    print(f"  NLL = {res.fun:.4f}  (converged: {res.success})")

    # ── Errors from numerical Hessian ─────────────────────────────────────────
    try:
        H = _hessian(nll, pfit)
        cov = inv(H)
        errors = np.sqrt(np.abs(np.diag(cov)))
    except (np.linalg.LinAlgError, ValueError):
        errors = np.full(len(pfit), np.nan)
        print("  Warning: Hessian inversion failed — errors unavailable")

    pnames = ["mu [GeV/c^2]", "sigma [GeV/c^2]", "alpha", "n", "slope", "f_sig"]
    print("\nFit result:")
    for name, val, err in zip(pnames, pfit, errors):
        print(f"  {name:16s}  {val:+10.6f}  +/-  {err:.6f}")

    # ── Chi^2 / NDF ────────────────────────────────────────────────────────────
    counts, edges = np.histogram(data, bins=nbins, range=(mass_min, mass_max))
    centers = 0.5 * (edges[:-1] + edges[1:])

    cb_n = _cb_norm(mu_f, sig_f, alp_f, n_f, mass_min, mass_max)
    exp_n = _exp_norm(sl_f, mass_min, mass_max, x0)
    exp_c = (fs_f * _cb(centers, mu_f, sig_f, alp_f, n_f) / cb_n +
             (1.0 - fs_f) * _exp(centers, sl_f, x0) / exp_n) * N * bw

    valid = exp_c > 5.0
    chi2v = np.sum((counts[valid] - exp_c[valid])**2 / exp_c[valid])
    ndf = int(valid.sum()) - len(pfit)
    pval = chi2_dist.sf(chi2v, ndf)

    print("\nGoodness of fit (binned chi^2):")
    print(f"  chi^2/NDF  = {chi2v:.1f} / {ndf} = {chi2v / ndf:.3f}")
    print(f"  p-value = {pval:.4f}")

    # ── Plot ──────────────────────────────────────────────────────────────────
    fig, (ax, axp) = plt.subplots(
        2, 1, figsize=(9, 7),
        gridspec_kw={"height_ratios": [3, 1]}, sharex=True
    )

    x_fine = np.linspace(mass_min, mass_max, 2000)
    cb_plt = _cb(x_fine, mu_f, sig_f, alp_f, n_f) / cb_n
    exp_plt = _exp(x_fine, sl_f, x0) / exp_n
    tot_plt = fs_f * cb_plt + (1.0 - fs_f) * exp_plt
    sc = N * bw   # PDF → counts / bin

    ax.errorbar(centers, counts, yerr=np.sqrt(counts),
                fmt="ko", ms=3, lw=1.0, label="Data", zorder=3)
    ax.plot(x_fine, tot_plt * sc,               "b-",  lw=2.0, label="Total fit")
    ax.plot(x_fine, fs_f * cb_plt * sc,         "r--", lw=1.5, label="Crystal Ball (signal)")
    ax.plot(x_fine, (1 - fs_f) * exp_plt * sc,  "g-.", lw=1.5, label="Exp. background")

    ax.set_xlim(mass_min, mass_max)
    ax.set_ylabel(f"Entries / {bw * 1000:.0f} MeV/$c^2$", fontsize=13)
    ax.legend(fontsize=11, loc="upper left")
    ax.set_title(title,
                 fontsize=13)

    info = "\n".join([
        rf"$\mu$ = {mu_f  * 1000:.2f} $\pm$ {errors[0] * 1000:.2f} MeV/$c^2$",
        rf"$\sigma$ = {sig_f * 1000:.2f} $\pm$ {errors[1] * 1000:.2f} MeV/$c^2$",
        rf"$\alpha$ = {alp_f:.3f} $\pm$ {errors[2]:.3f}",
        rf"$n$ = {n_f:.2f} $\pm$ {errors[3]:.2f}",
        rf"$f_{{\rm sig}}$ = {fs_f:.3f} $\pm$ {errors[5]:.3f}",
        rf"$\chi^2$/NDF = {chi2v:.0f}/{ndf} = {chi2v / ndf:.2f}",
        rf"$p$-value = {pval:.4f}",
    ])
    ax.text(0.04, 0.3, info, transform=ax.transAxes, va="bottom", ha="left",
            fontsize=10, bbox=dict(boxstyle="round", facecolor="lightyellow", alpha=0.85))

    pulls = np.where(exp_c > 0, (counts - exp_c) / np.sqrt(exp_c), 0.0)
    axp.bar(centers, pulls, width=bw, color="steelblue", alpha=0.7)
    axp.axhline(0, color="black", lw=0.8)
    axp.axhline(+2, color="red",   lw=0.8, ls="--")
    axp.axhline(-2, color="red",   lw=0.8, ls="--")
    axp.set_ylabel("pull", fontsize=12)
    axp.set_xlabel(r"$M(\mu^+\mu^-)$ [GeV/$c^2$]", fontsize=13)
    axp.set_ylim(-5, 5)

    fig.tight_layout()
    if savefig:
        fig.savefig(savefig, dpi=150)
        print(f"\nSaved {savefig}")

    return dict(pfit=pfit, errors=errors, chi2=chi2v, ndf=ndf, pval=pval, fig=fig)


# ─── Script entry point ───────────────────────────────────────────────────────
if __name__ == "__main__":
    data_all = np.loadtxt("InvM_data.txt")
    result = fit(data_all)
    plt.show()
