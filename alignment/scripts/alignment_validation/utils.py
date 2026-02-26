"""
Pure utility functions shared between cosmics and dimuon validation.
"""

import numpy as np

from alignment_validation.variables import TrackVariable

#: log level (0 = silent, 1 = warnings, 2 = verbose)
log_level = 0


def get_variable_names(variables: list) -> list:
    """Flatten a list of Variable objects into a list of ROOT branch-name strings.

    Parameters
    ----------
    variables : list of GlobalVariable or TrackVariable
        Variable metadata objects whose branch names should be collected.

    Returns
    -------
    list of str
        All ROOT branch names in the order they appear in ``variables``.
        TrackVariable contributes two names (name1, name2).
    """
    names = []
    for variable in variables:
        names.extend(variable.getName())
    return names


def auto_range(data_list: list, percent: float, modify: float = 0, symmetric: bool = False) -> tuple:
    """Return an axis range covering the central ``percent`` of all datasets.

    The range is determined by the union of per-dataset percentile intervals,
    optionally symmetrised around the grand median and padded by ``modify``.

    Parameters
    ----------
    data_list : list of array-like
        One or more data arrays to consider simultaneously. The returned range
        is wide enough to cover all of them.
    percent : float
        Central percentage of data to include (e.g. 96 keeps the central 96%,
        clipping 2% from each tail).
    modify : float, optional
        Fractional padding applied to the final range width on each side.
        E.g. 0.1 expands the range by 10% on each end. Default is 0.
    symmetric : bool, optional
        If True, the range is symmetrised around the mean of per-dataset
        medians before padding. Default is False.

    Returns
    -------
    tuple of (float, float)
        ``(minimum, maximum)`` of the computed range.
    """
    low_percent = (100 - percent) / 2
    high_percent = 100 - low_percent
    low, high, median = (), (), ()
    for data in data_list:
        low += (np.percentile(data, low_percent),)
        high += (np.percentile(data, high_percent),)
        median += (np.median(data),)
    minimum = min(low)
    maximum = max(high)
    center = np.mean(median)
    if symmetric:
        if abs(center - minimum) > abs(center - maximum):
            maximum = center + center - minimum
        if abs(center - minimum) < abs(center - maximum):
            minimum = center + center - maximum
    minimum = minimum - (maximum - minimum) * modify
    maximum = maximum + (maximum - minimum) * modify
    return (minimum, maximum)


def normal_distribution(x: float, a: float, mu: float, sigma: float) -> float:
    """Evaluate a normalised Gaussian at ``x``.

    Parameters
    ----------
    x : float or array-like
        Point(s) at which to evaluate the Gaussian.
    a : float
        Amplitude (total integral of the Gaussian).
    mu : float
        Mean of the Gaussian.
    sigma : float
        Standard deviation of the Gaussian.

    Returns
    -------
    float or ndarray
        Value of ``a / (sigma * sqrt(2*pi)) * exp(-0.5 * ((x-mu)/sigma)^2)``.
    """
    return (a / (sigma * (2 * np.pi) ** 0.5)) * np.exp(-((x - mu) / sigma) ** 2 / 2)


def to_bins(x, y, bins, x_limits):
    """Bin (x, y) data and compute per-bin median, sigma68, and their uncertainties.

    Bins with fewer than 10 entries are filled with ``nan``.

    Parameters
    ----------
    x : array-like
        Independent variable used to define bins.
    y : array-like
        Dependent variable whose distribution is summarised in each bin.
    bins : int or sequence of float
        Number of equal-width bins or an explicit sequence of bin edges,
        passed directly to ``numpy.histogram_bin_edges``.
    x_limits : tuple of (float, float)
        ``(min, max)`` range over which to bin.

    Returns
    -------
    x_vals : list of float
        Bin-centre x values (``nan`` for empty bins).
    y_medians : list of float
        Per-bin median of ``y`` (``nan`` for empty bins).
    x_halfwidth : list of float
        Half-width of each bin in x (``nan`` for empty bins).
    y_approx_uncert : list of float
        Approximate uncertainty on the median: ``std(y) / sqrt(N)``
        (``nan`` for empty bins).
    halfwidth_of_sigma68 : list of float
        Half the 16th–84th percentile range of ``y``, i.e. sigma68
        (``nan`` for empty bins).
    sigma68_approx_uncert : list of float
        Approximate uncertainty on sigma68: ``sigma68 / sqrt(N)``
        (``nan`` for empty bins).
    """
    x_edges = np.histogram_bin_edges(x, bins, x_limits)
    x_bin_halfwidth = [(x_edges[i] - x_edges[i - 1]) / 2 for i in range(1, len(x_edges))]
    bin_numbers = np.digitize(x, x_edges) - 1
    y_medians, y_approx_uncert, halfwidth_of_sigma68, sigma68_approx_uncert, x_halfwidth, x_vals = [], [], [], [], [], []
    for i in range(len(x_edges) - 1):
        if y[bin_numbers == i].size >= 10:
            if log_level > 1:
                print(f"Events in bin {x_edges[i]} to {x_edges[i+1]}: {y[bin_numbers == i].size}")
            y_medians.append(float(np.median(y[bin_numbers == i])))
            y_approx_uncert.append(float(np.std(y[bin_numbers == i]) / len(y[bin_numbers == i]) ** 0.5))
            halfwidth_of_sigma68.append((np.percentile(y[bin_numbers == i], 84) -
                                         np.percentile(y[bin_numbers == i], 16)) / 2)
            sigma68_approx_uncert.append(halfwidth_of_sigma68[-1] / len(y[bin_numbers == i]) ** 0.5)
            x_halfwidth.append(x_bin_halfwidth[i])
            x_vals.append(x_edges[i] + x_bin_halfwidth[i])
        else:
            y_medians.append(np.nan)
            y_approx_uncert.append(np.nan)
            halfwidth_of_sigma68.append(np.nan)
            sigma68_approx_uncert.append(np.nan)
            x_halfwidth.append(np.nan)
            x_vals.append(np.nan)
            if log_level:
                print(f"Warning: bin {x_edges[i]}-{x_edges[i+1]} is empty.")
    return x_vals, y_medians, x_halfwidth, y_approx_uncert, halfwidth_of_sigma68, sigma68_approx_uncert


def swap_tracks(data: dict, vars: list) -> dict:
    """Swap the name1/name2 arrays for each TrackVariable in ``vars`` within the data dict.

    Modifies ``data`` in-place and also returns it.

    Parameters
    ----------
    data : dict
        Data dictionary mapping ROOT branch names to arrays, as returned by
        ``load_data``.
    vars : list of GlobalVariable or TrackVariable
        Variables to process. Only TrackVariable instances are swapped;
        GlobalVariable instances are ignored.

    Returns
    -------
    dict
        The same ``data`` dict with track-1 and track-2 arrays swapped for
        each TrackVariable in ``vars``.
    """
    for var in vars:
        if isinstance(var, TrackVariable):
            data[var.name1], data[var.name2] = data[var.name2], data[var.name1]
    return data


def get_filter(data: dict, cut: dict) -> np.ndarray:
    """Return a boolean mask selecting events that pass all (min, max) cuts.

    Parameters
    ----------
    data : dict
        Data dictionary mapping branch names to arrays.
    cut : dict
        Selection criteria as ``{branch_name: (min, max)}``. An event passes
        if ``min < data[branch_name] < max`` for every entry in ``cut``.
        An empty dict returns a mask of all True.

    Returns
    -------
    numpy.ndarray of bool
        Boolean mask with the same length as the data arrays.
    """
    mask = np.ones_like(next(iter(data.values())), dtype=bool)
    for var in cut:
        mask = np.logical_and(mask, np.logical_and(cut[var][0] < data[var], data[var] < cut[var][1]))
    return mask


def pseudomomentum(p, tan, power):
    """Compute the muon pseudomomentum p*beta*sin(theta)^(power/2).

    This quantity is used as the x-axis for resolution-vs-momentum plots.
    The muon mass (105.7 MeV/c²) is used for the beta factor.

    Parameters
    ----------
    p : array-like
        Transverse momentum in GeV/c.
    tan : array-like
        tan(lambda), where lambda is the dip angle (= pz/pt).
    power : int or float
        Exponent of sin(theta) in the pseudomomentum definition.
        Typical values are 3 (for z0/tanLambda) and 5 (for d0/phi).

    Returns
    -------
    ndarray
        Pseudomomentum values in GeV/c.
    """
    return p * (1 + 1 / tan ** 2) ** 0.5 * 1 / (1 + 0.105 ** 2 / (p ** 2 *
                                                                  (1 + tan ** 2))) ** 0.5 * 1 / ((1 + tan ** 2) ** 0.5) ** power
