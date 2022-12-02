#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import numpy as np
import pandas as pd
import h5py
import uproot


def _make_const_lists():
    """Moving this code into a function to avoid a top-level ROOT import."""
    import ROOT.Belle2

    PARTICLES, PDG_CODES = [], []
    for i in range(len(ROOT.Belle2.Const.chargedStableSet)):
        particle = ROOT.Belle2.Const.chargedStableSet.at(i)
        name = (particle.__repr__()[7:-1]
                .replace("-", "")
                .replace("+", "")
                .replace("euteron", ""))
        PARTICLES.append(name)
        PDG_CODES.append(particle.getPDGCode())
    # PARTICLES = ["e", "mu", "pi", "K", "p", "d"]
    # PDG_CODES = [11, 13, 211, 321, 2212, 1000010020]

    DETECTORS = []
    for det in ROOT.Belle2.Const.PIDDetectors.set():
        DETECTORS.append(ROOT.Belle2.Const.parseDetectors(det))
    # DETECTORS = ["SVD", "CDC", "TOP", "ARICH", "ECL", "KLM"]

    return PARTICLES, PDG_CODES, DETECTORS


# PARTICLES, PDG_CODES, DETECTORS = _make_const_lists()
PARTICLES = ["e", "mu", "pi", "K", "p", "d"]
PDG_CODES = [11, 13, 211, 321, 2212, 1000010020]
DETECTORS = ["SVD", "CDC", "TOP", "ARICH", "ECL", "KLM"]

P_BINS = np.array([0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.5])
THETA_BINS = np.radians(np.array([17, 28, 40, 60, 77, 96, 115, 133, 150]))


def _column(particle, detector):
    """Default column names for detector log-likelihoods.

    Args:
        particle (str): particle name
        detector (str): detector name

    Returns:
        str: Corresponding column name.
    """
    return f"{detector}_{particle}"


def root_column(particle, detector):
    """Column names for detector log-likelihoods found in our ROOT datafiles.

    Args:
        particle (str): particle name
        detector (str): detector name

    Returns:
        str: Corresponding column name.
    """
    pdg = PDG_CODES[PARTICLES.index(particle)]
    return f"pidLogLikelyhoodOf{pdg}From{detector}"


def read_root(root_filenames):
    """Reads one or several ROOT datafiles into a DataFrame.

    Args:
        root_filenames (list(str) or str): If only one filename, can be given as
            a string. If more than one, should be given as a list or tuple.

    Returns:
        pandas.DataFrame: DataFrame containing the data of the ROOT datafile(s).
    """

    return uproot.concatenate(root_filenames, library='pd')


def make_h5(df, tags, out_filename, pdg=None, column=root_column):
    """Make an HDF5 file in our 'slim' format from the given DataFrame.

    Args:
        df (pandas.DataFrame): The DataFrame containing the data.
        tags (list(str) or str): The particle tags used as a prefix for desired
            columns. e.g. for kaons in a D* decay, this is 'DST_D0_K'. One or
            more can be given.
        out_filename (str): Output filename for the h5 file that will be
            written.
        pdg (int or None): The PDG code for the particles being
            extracted. If None, uses the values found in the 'mcPDG' column of
            the DataFrame. Defaults to None.
        column: A function which, given the particle and
            detector names, returns the column name for the corresponding
            detector log-likelihood. Defaults to root_column, which assumes
            column names are of the format
            f'pidLogLikelyhoodOf{pdg}From{detector}'.
    """

    if isinstance(tags, str):
        tags = [tags]

    def _concat(arrs):
        return np.concatenate(arrs) if len(arrs) > 1 else arrs[0]

    def _get_all(col):
        return _concat([df[f"{tag}_{col}"].values for tag in tags])

    with h5py.File(out_filename, "w") as f:
        if pdg is not None:
            pdg_values = np.ones(len(df) * len(tags)) * pdg
        else:
            pdg_values = np.abs(_get_all("mcPDG"))

        f.create_dataset("pdg", data=pdg_values)
        f.create_dataset("p", data=_get_all("p"))
        f.create_dataset("theta", data=np.arccos(_get_all("cosTheta")))
        f.create_dataset("phi", data=_get_all("phi"))

        for det in DETECTORS:
            for p in PARTICLES:
                data = _get_all(column(p, det))
                data[np.isnan(data)] = 0
                f.create_dataset(f"{det}/{p}", data=data)


def merge_h5s(filenames, out_filename, pdgs=None):
    """Merge several HDF5 files in our 'slim' format together.

    Args:
        filenames (list(str)): Filenames of HDF5 files to be merged.
        out_filename (str): Output filename.
        pdgs (list(int)): The PDG tags for the particle types, one per
            filename, to overwrite the 'pdg' columns in those files when
            merging. If None, simply uses the 'pdg' columns from the files.
            Defaults to None.
    """
    fs = [h5py.File(fname, "r") for fname in filenames]
    m = h5py.File(out_filename, "w")

    keys = ["p", "theta", "phi"]
    keys += [f"{d}/{p}" for d in DETECTORS for p in PARTICLES]

    for key in keys:
        m.create_dataset(key, data=np.concatenate([f[key][()] for f in fs]))

    if pdgs is not None:
        # replace 'pdg' data with kinematic tags
        m.create_dataset(
            "pdg",
            data=np.concatenate(
                [np.ones_like(f["pdg"][()]) * pdg for f, pdg in zip(fs, pdgs)]
            ),
        )
    else:
        m.create_dataset("pdg", data=np.concatenate([f["pdg"][()] for f in fs]))

    for f in fs:
        f.close()
    m.close()


def split_h5(
    filename,
    output_dir,
    train_size=0.8,
    val_size=0.1,
    test_size=0.1,
    shuffle=True,
    random_state=None,
):
    """Split the data in an HDF5 'slim' format file in train, validation, and
    test sets, stored in .npz files for ease of weight training.

    Args:
        filename (str): Filename of HDF5 input file.
        output_dir (str): Name of output directory, in which the train,
            validation, and test sets will be written. Will be created if it
            does not already exist.
        train_size (float): Fraction of the dataset to use for
            training. Defaults to 0.8.
        val_size (float): Fraction of the dataset to use for
            validation. Defaults to 0.1.
        test_size (float): Fraction of the dataset to use for testing.
            Defaults to 0.1.
        shuffle (bool): Whether to shuffle the dataset before
            splitting. Defaults to True.
        random_state (int or None): Random state for the shuffling.
            Defaults to None.
    """

    from sklearn.model_selection import train_test_split
    from os.path import join
    from os import makedirs

    assert train_size > 0, f"train_size ({train_size}) must be positive"
    assert val_size >= 0, f"val_size ({val_size}) may not be negative"
    assert test_size >= 0, f"test_size ({test_size}) may not be negative"
    assert val_size + test_size != 0, "val_size and test_size cannot both be zero"

    if val_size == 0:
        val_size = test_size
        test_size = 0

    if train_size + val_size + test_size != 1:
        total = train_size + val_size + test_size
        train_size = train_size / total
        val_size = val_size / total
        test_size = test_size / total

    # read data
    with h5py.File(filename, "r") as f:
        data = np.stack(
            [f[det][p][()] for p in PARTICLES for det in DETECTORS], axis=-1
        )
        p_data = f["p"][()]
        theta_data = f["theta"][()]
        labels = np.abs(f["pdg"][()])
        for i, p in enumerate(PDG_CODES):
            labels[labels == p] = i
        mask = labels < 6

    X = data[mask]
    y = labels[mask]
    p = p_data[mask]
    t = theta_data[mask]

    makedirs(output_dir, exist_ok=True)
    kw = dict(shuffle=shuffle, random_state=random_state)

    # split once
    (X_0, X, y_0, y, p_0, p, t_0, t) = train_test_split(
        X, y, p, t, train_size=train_size, **kw
    )
    np.savez(join(output_dir, "train.npz"), X=X_0, y=y_0, p=p_0, theta=t_0)

    # split again if desired
    if test_size != 0:
        size = val_size / (1 - train_size)
        (X_1, X_2, y_1, y_2, p_1, p_2, t_1, t_2) = train_test_split(
            X, y, p, t, train_size=size, **kw
        )

        np.savez(join(output_dir, "val.npz"), X=X_1, y=y_1, p=p_1, theta=t_1)
        np.savez(join(output_dir, "test.npz"), X=X_2, y=y_2, p=p_2, theta=t_2)

    else:
        np.savez(join(output_dir, "val.npz"), X=X, y=y, p=p, theta=t)


def softmax(x):
    """Performs softmax calculation with corrections to help prevent overflow.

    Note:
        This is the calculation used to convert log-likelihoods to likelihood
        ratios. Implementation following
        https://stackoverflow.com/a/67112412/18837571

    Args:
        x (:func:`numpy.array`): Data to be softmaxed. Softmax is calculated over the last
            dimension.

    Returns:
        :func:`numpy.array`: Softmaxed data.
    """
    maxes = np.amax(x, axis=-1, keepdims=True)
    x_exp = np.exp(x - maxes)
    return x_exp / np.sum(x_exp, axis=-1, keepdims=True)


def make_labels(df):
    """Make a 'labels' column for the DataFrame. The 'labels' column contains
    the particle type labels for each event: 0 for electron, 1 for muon, and so
    on.

    Args:
        df (pandas.DataFrame): DataFrame that 'labels' column will be added to. Must
            not have NaN values in the 'pdg' column.
    """
    labels = np.abs(df["pdg"].values)
    if np.count_nonzero(~np.isfinite(labels)):
        print(
            'Warning: dataset contains NaN values in the "pdg" column. '
            'This means the "labels" column cannot be made, so most of the '
            "pidplots methods will fail."
        )
        labels = np.ones_like(labels) * np.nan
    else:
        for i, p in enumerate(PDG_CODES):
            labels[labels == p] = i
        labels[labels >= 6] = -1
    df["labels"] = labels


def make_bins(df, p_bins=P_BINS, theta_bins=THETA_BINS):
    """Make 'p_bin' and 'theta_bin' column in the given DataFrame.

    Args:
        df (pandas.DataFrame): The DataFrame to add bins columns to.
        p_bins (:func:`numpy.array`): The edges of the momentum bins in GeV.
            Defaults to P_BINS, [0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.5] GeV.
        theta_bins (:func:`numpy.array`): The edges of the theta bins in radians.
            Defaults to THETA_BINS, [17, 28, 40, 60, 77, 96, 115, 133, 150]
            degrees.
    """
    df["p_bin"] = np.digitize(df["p"].values, p_bins) - 1
    df["theta_bin"] = np.digitize(df["theta"].values, theta_bins) - 1


def make_lrs(df, column=_column):
    """Makes likelihood ratio columns for each of the six particle types in the
    given DataFrame.

    Args:
        df (pandas.DataFrame): DataFrame to which the columns will be added.
        column: A function which given the particle and
            detector names returns the corresponding detector log-likelihood
            column name. Defaults to _column, which gives column names of the
            format f"{detector}_{particle}".
    """
    # hypothesis log-likelihoods
    h_logls = np.stack(
        [
            np.sum(df[[column(p, det) for det in DETECTORS]].values, -1)
            for p in PARTICLES
        ],
        -1,
    )

    # compute likelihood ratios
    lrs = softmax(h_logls)
    for i, p in enumerate(PARTICLES):
        df[f"lr_{p}"] = lrs[:, i]


def make_binary_lrs(df, column=_column):
    """Makes binary likelihood ratio columns for each of the five non-pion
    particle type hypotheses in the given DataFrame.

    Args:
        df (pandas.DataFrame): DataFrame to which the columns will be added.
        column: A function which given the particle and
            detector names returns the corresponding detector log-likelihood
            column name. Defaults to _column, which gives column names of the
            format f"{detector}_{particle}".
    """
    for h in PARTICLES:
        if h == "pi":
            continue

        h_logls = np.stack(
            [
                np.sum(df[[column(p, det) for det in DETECTORS]].values, -1)
                for p in [h, "pi"]
            ],
            -1,
        )
        lrs = softmax(h_logls)
        df[f"binary_lr_{h}"] = lrs[:, 0]


def make_pid(df):
    """Makes a 'pid' column in the given DataFrame. The 'pid' column is the
    predicted particle type. Requires likelihood ratio columns to exist.

    Args:
        df (pandas.DataFrame): DataFrame to which the 'pid' column will be added.
    """
    lrs = np.stack([df[f"lr_{p}"].values for p in PARTICLES], axis=-1)
    pids = np.argmax(lrs, axis=-1)
    df["pid"] = pids


def compute_det_lrs(d, det, column=_column):
    """Computes single-detector likelihood ratios from the given DataFrame.

    Args:
        d (pandas.DataFrame): DataFrame containing the detector log-likelihoods.
        det (str): The name of the detector for which the single-detector
            likelihood ratios will be calculated.
        column: A function which given the particle and
            detector names returns the corresponding detector log-likelihood
            column name. Defaults to _column, which gives column names of the
            format f"{detector}_{particle}".

    Returns:
        :func:`numpy.array`: The detector likelihood ratios.
    """
    h_logls = d[[column(p, det) for p in PARTICLES]].values
    lrs = softmax(h_logls)
    return lrs


def make_pid_det(df, column=_column):
    """Makes single-detector PID columns for each of the detectors in the given DataFrame.

    Args:
        df (pandas.DataFrame): DataFrame to which the columns will be added.
        column: A function which given the particle and
            detector names returns the corresponding detector log-likelihood
            column name. Defaults to _column, which gives column names of the
            format f"{detector}_{particle}".
    """
    for det in DETECTORS:
        mask = df[column("e", det)] == 0  # TODO: make more robust
        lrs = compute_det_lrs(df, det, column=column)
        pids = np.argmax(lrs, axis=-1)
        pids[mask] = -1
        df[f"pid_{det}"] = pids


def compute_abl_lrs(d, det, column=_column):
    """Computes ablation likelihood ratios from the given DataFrame.

    Args:
        d (pandas.DataFrame): DataFrame containing the detector log-likelihoods.
        det (str): The name of the detector to be omitted for the ablation.
        column: A function which given the particle and
            detector names returns the corresponding detector log-likelihood
            column name. Defaults to _column, which gives column names of the
            format f"{detector}_{particle}".

    Returns:
        :func:`numpy.array`: The ablation likelihood ratios.
    """

    def _cols(p):
        others = [det2 for det2 in DETECTORS if det2 != det]
        return [column(p, det2) for det2 in others]

    h_logls = np.stack([np.sum(d[_cols(p)].values, -1) for p in PARTICLES], -1)
    lrs = softmax(h_logls)
    return lrs


def make_pid_abl(df, column=_column):
    """Makes ablation PID columns for each of the detectors in the given
    DataFrame.

    Args:
        df (pandas.DataFrame): DataFrame to which the columns will be added.
        column: A function which given the particle and
            detector names returns the corresponding detector log-likelihood
            column name. Defaults to _column, which gives column names of the
            format f"{detector}_{particle}".
    """
    for det in DETECTORS:
        lrs = compute_abl_lrs(df, det, column=column)
        pids = np.argmax(lrs, axis=-1)
        df[f"pid_no_{det}"] = pids


def compute_contrib(d, corr=True):
    """Computes the detector contributions.

    Args:
        d (pandas.DataFrame): DataFrame containing the likelihood ratio data.
        corr (bool): Whether to compute contribution to the likelihood
            ratio of the _correct_ hypothesis (True) or the _chosen_ hypothesis
            (False). Defaults to True.

    Returns:
        dict[str, :func:`numpy.array`]: The contributions of each detector.
    """
    out = dict()
    for det in DETECTORS:
        reg_lrs = d[[f"lr_{p}" for p in PARTICLES]].values
        abl_lrs = compute_abl_lrs(d, det)
        idx = d["labels" if corr else "pid"].values.astype(int)
        reg_lr = reg_lrs[np.arange(len(idx)), idx]
        abl_lr = abl_lrs[np.arange(len(idx)), idx]
        ctrb = reg_lr - abl_lr
        out[det] = ctrb
    return out


def make_contrib(df, corr=True):
    """Makes columns for the detector contributions in the given DataFrame.

    Args:
        df (pandas.DataFrame): DataFrame to which the columns will be added.
        corr (bool): Whether to compute contribution to the likelihood
            ratio of the _correct_ hypothesis (True) or the _chosen_ hypothesis
            (False). Defaults to True.
    """
    ctrbs = compute_contrib(df, corr=corr)
    for det, ctrb in ctrbs.items():
        df[f"contrib_{det}"] = ctrb


def make_columns(
    df,
    p_bins=P_BINS,
    theta_bins=THETA_BINS,
    contrib_corr=True,
    column=_column,
):
    """Makes all the additional columns for a given DataFrame.

    Args:
        df (pandas.DataFrame): DataFrame to which the columns will be added.
        p_bins (:func:`numpy.array`): The edges of the momentum bins in GeV.
            Defaults to P_BINS, [0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.5] GeV.
        theta_bins (:func:`numpy.array`): The edges of the theta bins in radians.
            Defaults to THETA_BINS, [17, 28, 40, 60, 77, 96, 115, 133, 150]
            degrees.
        contrib_corr (bool): Whether to compute contribution to the
            likelihood ratio of the _correct_ hypothesis (True) or the _chosen_
            hypothesis (False). Defaults to True.
        column: A function which given the particle and
            detector names returns the corresponding detector log-likelihood
            column name. Defaults to _column, which gives column names of the
            format f"{detector}_{particle}".
    """
    make_labels(df)
    make_bins(df, p_bins=p_bins, theta_bins=theta_bins)
    make_lrs(df, column=column)
    make_binary_lrs(df, column=column)
    make_pid(df)
    make_pid_det(df, column=column)
    make_pid_abl(df)
    make_contrib(df, corr=contrib_corr)


def apply_weights(df, weights, p_bins=P_BINS, theta_bins=THETA_BINS, column=_column):
    """Applies the given weights to the log-likelihood data in the DataFrame.

    Args:
        df (pandas.DataFrame): DataFrame to which the weights are applied.
        weights (dict[tuple(int), :func:`numpy.array`] or :func:`numpy.array`): The calibration weight
            values. If a dict, keys should be a tuple of ints, and each value is
            the six-by-six array of weights for the bin. If a single np.array,
            should be a six-by-six array of weights to be applied globally.
        p_bins (:func:`numpy.array`): The edges of the momentum bins in GeV.
            Defaults to P_BINS, [0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.5] GeV.
        theta_bins (:func:`numpy.array`): The edges of the theta bins in radians.
            Defaults to THETA_BINS, [17, 28, 40, 60, 77, 96, 115, 133, 150]
            degrees.
        column: A function which given the particle and
            detector names returns the corresponding detector log-likelihood
            column name. Defaults to _column, which gives column names of the
            format f"{detector}_{particle}".
    """
    if weights is None:
        return

    # per-bin weights
    if isinstance(weights, dict):
        for p in range(len(p_bins) - 1):
            p_lo, p_hi = p_bins[p], p_bins[p + 1]
            p_mask = (df["p"] >= p_lo) & (df["p"] <= p_hi)

            for theta in range(len(theta_bins) - 1):
                t_lo, t_hi = theta_bins[theta], theta_bins[theta + 1]
                t_mask = (df["theta"] >= t_lo) & (df["theta"] <= t_hi)

                for i, h in enumerate(PARTICLES):
                    for j, d in enumerate(DETECTORS):
                        df.loc[(p_mask & t_mask), column(h, d)] *= weights[p, theta][
                            i, j
                        ]

    # global weights
    else:
        for i, h in enumerate(PARTICLES):
            for j, d in enumerate(DETECTORS):
                df[column(h, d)] *= weights[i, j]


def cut_particles(df, allowed_particles, column=_column):
    """Cuts the log-likelihood data associated with given particle types.

    Args:
        df (pandas.DataFrame): DataFrame to which the cuts will be applied.
        allowed_particles (list(str)): List of allowed particle types. Any
            particle types not present will be cut, unless the list is empty (in
            which case no cuts are applied).
        column: A function which given the particle and
            detector names returns the corresponding detector log-likelihood
            column name. Defaults to _column, which gives column names of the
            format f"{detector}_{particle}".
    """
    if len(allowed_particles) == 0:
        return

    for p in PARTICLES:
        if not (p in allowed_particles):
            for d in DETECTORS:
                df[column(p, d)] = -1e10


def read_h5(filename):
    """Read an HDF5 file in our 'slim' format into a DataFrame.

    Args:
        filename (str): Input filename.

    Returns:
        pandas.DataFrame: DataFrame containing data.
    """
    df = pd.DataFrame()
    with h5py.File(filename, "r") as f:
        for key in ["pdg", "p", "theta", "phi"]:
            df[key] = f[key][()]
        for key in [f"{d}/{p}" for d in DETECTORS for p in PARTICLES]:
            df_key = key.replace("/", "_")
            df[df_key] = f[key][()]
            df[df_key] = df[df_key].fillna(0)
    return df


def read_npz(filename):
    """Read an npz file in our training format into a DataFrame.

    Args:
        filename (str): Input filename.

    Returns:
        pandas.DataFrame: DataFrame containing data.
    """
    data = np.load(filename)
    df = pd.DataFrame(
        data=data["X"], columns=[f"{d}_{p}" for p in PARTICLES for d in DETECTORS],
    )
    df["labels"] = data["y"]
    df["p"] = data["p"]
    df["theta"] = data["theta"]

    df["pdg"] = df["labels"]
    for i, pdg in enumerate(PDG_CODES):
        df.loc[df["labels"] == i, "pdg"] = pdg

    return df


def produce_analysis_df(
    df,
    compute_cols=True,
    drop_nans=True,
    drop_outside_bins=True,
    weights=None,
    allowed_particles=[],
    p_bins=P_BINS,
    theta_bins=THETA_BINS,
    column=None,
):
    """Prepares a DataFrame for PID analysis by applying weights, computing and
    adding additional columns, cutting NaNs, and more.

    Args:
        df (pandas.DataFrame): DataFrame to prepare for analysis.
        compute_cols (bool): Whether to compute and add additional
            columns. Defaults to True.
        drop_nans (bool): Whether to drop rows that contain NaNs.
            Defaults to True.
        drop_outside_bins (bool): Whether to drop rows for particles
            outside of the momentum and theta bins. Defaults to True.
        weights (:func:`numpy.array`): Calibration weights to be applied to the
            detector log-likelihoods. Defaults to None.
        allowed_particles (list(str)): If not empty, specifies the
            allowed particle types. Any not allowed particle types will be
            excluded from the PID calculations. If empty, all particle types are
            considered.  Defaults to [].
        p_bins (:func:`numpy.array`): The edges of the momentum bins in GeV.
            Defaults to P_BINS, [0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.5] GeV.
        theta_bins (:func:`numpy.array`): The edges of the theta bins in radians.
            Defaults to THETA_BINS, [17, 28, 40, 60, 77, 96, 115, 133, 150]
            degrees.
        column: A function which given the particle and
            detector names returns the corresponding detector log-likelihood
            column name. Defaults to _column, which gives column names of the
            format f"{detector}_{particle}".

    Returns:
        pandas.DataFrame: Return the prepared DataFrame. (Not all modifications in
            this method are in-place.)
    """
    if column is not None:
        for p in PARTICLES:
            for d in DETECTORS:
                df[f"{d}_{p}"] = df[column(p, d)]

    apply_weights(df, weights, p_bins=p_bins, theta_bins=theta_bins)
    cut_particles(df, allowed_particles)

    if compute_cols:
        make_columns(
            df,
            p_bins=p_bins,
            theta_bins=theta_bins,
            contrib_corr=True,
        )
        if drop_outside_bins:
            df = df.loc[
                np.logical_and.reduce(
                    [
                        df["p_bin"].values >= 0,
                        df["p_bin"].values < len(p_bins) - 1,
                        df["theta_bin"].values >= 0,
                        df["theta_bin"].values < len(theta_bins) - 1,
                    ]
                )
            ]

    if drop_nans:
        df = df.dropna()
        df = df[df["labels"] >= 0]
    return df
