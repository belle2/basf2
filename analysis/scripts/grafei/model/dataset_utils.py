##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import numpy as np
import uproot


def populate_avail_samples(X, Y, B_reco=0):
    """
    Shifts through the file metadata to populate a list of available dataset samples.

    Args:
        X (list): List of ROOT lazyarray dicts for X (input) data.
        Y (list): List of ROOT lazyarray dicts for Y (ground truth) data.
        B_reco (int): Reconstruction mode flag (set automatically):

            .. math::
               \\text{Upsilon} (4S) = 0,\\ B^0 = 1,\\ B^+ = 2.

    Returns:
        list: List of available samples for training.
    """
    # Must iterate over Y because X contains a b_index of -1 for unmatched particles
    avail_samples = []

    # Iterate over files in self.y
    for i, f in enumerate(Y):
        # Have to get event list differently
        events = X[i]["event"]

        # Iterate over events in current file
        for evt_idx, _ in enumerate(events):
            b_indices = [1] if not B_reco else [1, 2]

            for b_index in b_indices:
                # Check that LCA is not trivial
                lca_rows = f[b_index]["n_LCA"][evt_idx]

                if lca_rows < 2:
                    continue

                # Fetch relevant event properties
                x_attrs = X[i]
                evt_b_index = x_attrs["b_index"][evt_idx]
                evt_primary = x_attrs["primary"][evt_idx]

                # particles coming from one or both Bs
                matched = (evt_b_index != -1) if not B_reco else (evt_b_index == int(b_index))

                # Keeping only those where there are reconstructed particles
                if matched.sum() == 0:
                    continue

                # Skip events/B's with one or less primaries reconstructed
                if np.sum(np.logical_and(matched, evt_primary)) < 2:
                    continue

                # If we made it to here a sample is valid and we add it to the one available
                avail_samples.append((i, evt_idx, b_index))

    return avail_samples


def preload_root_data(root_files, features, discarded):
    """
    Load all data from root files as lazyarrays (not actually read from disk until accessed).

    Args:
        root_files (str): Path to ROOT files.
        features (list): List of feature names.
        discarded (list): List of features present in the ROOT files and not used as input,
            but used to calculate other quantities (e.g. edge features).

    Returns:
        list, list: Lists of dictionaries containing training information for input and ground-truth.
    """
    x = []
    y = []

    for f in root_files:
        with uproot.open(f)["Tree"] as tree:
            # Get event numbers
            event = tree["event"].array(library="np")
            # Create dicts for x and y lazy arrays
            x_dict = {}
            x_dict["event"] = event
            x_dict["features"] = {
                feat: tree[feat].array(library="np") for feat in features
            }
            x_dict["discarded"] = {
                feat: tree[feat].array(library="np") for feat in discarded
            }

            # Need this to initialise numpy features array in __getitem__
            x_dict["leaves"] = tree["leaves"].array(library="np")
            x_dict["primary"] = tree["primary"].array(library="np")
            x_dict["b_index"] = tree["b_index"].array(library="np")
            x_dict["mc_pdg"] = tree["mcPDG"].array(library="np")

            y_dict = {1: {}, 2: {}}
            for i in [1, 2]:
                # Get this LCA
                # Need this to reshape the falttened LCA when loading
                y_dict[i]["n_LCA"] = tree[f"n_LCA_leaves_{i}"].array(library="np")
                y_dict[i]["LCA"] = tree[f"LCAS_{i}"].array(library="np")
                y_dict[i]["LCA_leaves"] = tree[f"LCA_leaves_{i}"].array(library="np")

            x.append(x_dict)
            y.append(y_dict)

    return x, y
