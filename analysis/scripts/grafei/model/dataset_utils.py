import numpy as np
import uproot


# def populate_avail_samples(X, Y, hadronic=False, allow_missing=False, duplicates='skip', fixed_size=False):
def populate_avail_samples(
    X,
    Y,
    ups_reco=False,
    # allow_background=False,
):
    """Sift through the file metadata to populate a list of available dataset samples

    Args:
        X(list): List of h5py file objects or ROOT lazyarray dicts for X (input) data
        Y(list): List of h5py file objects or ROOT lazyarray dicts for Y (ground truth) data
    """
    # Must iterate over Y because X contains a b_index of -1 for unmatched particles
    avail_samples = []

    for i, f in enumerate(Y):  # iterate over files in self.y
        # Have to get event list differently
        events = X[i]["event"]

        for evt_idx, _ in enumerate(events):  # iterate over events in current file
            b_indices = [1] if ups_reco else [1, 2]

            for b_index in b_indices:
                # Check that LCA is not trivial
                lca_rows = f[b_index]["n_LCA"][evt_idx]

                if lca_rows < 2:
                    continue

                # Fetch relevant event properties
                # isUps = f["isUps"][evt_idx]

                x_attrs = X[i]
                evt_b_index = x_attrs["b_index"][evt_idx]
                evt_primary = x_attrs["primary"][evt_idx]

                # Keeping only signal events
                # if not allow_background and not isUps:
                #     continue

                # particles coming from one or both Bs
                matched = np.logical_or((evt_b_index == 1), (evt_b_index == 2)) if ups_reco else (evt_b_index == int(b_index))

                # Keeping only those where there are reconstructed particles
                if matched.sum() == 0:
                    continue

                # Skip events/B's with one or less primaries reconstructed
                if np.sum(np.logical_and(matched, evt_primary)) < 2:
                    continue

                # If we made it to here a sample is valid and we add it to the one available
                avail_samples.append((i, evt_idx, b_index))

    return avail_samples


def preload_root_data(
    root_files, features, discarded, global_features=[],
):
    """Load all data from root files as lazyarrays (not actually read from disk until accessed)

    This is intended to make the data in roughly the same format as it is for HDF5.
    Main difference is the missing event key, instead each entry is an array with n_events rows
    x[file_index] = {'event', 'features', 'all_features', 'n_particles'}
    y[file_index][b_index] = {'n_LCA', 'LCA', 'LCA_leaves'}
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
            # x_dict['n_particles'] = tree['n_particles'].array(library='np')
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
                # y_dict[i]["isB"] = tree[f"isB_{i}"].array(library="np")

            x.append(x_dict)
            y.append(y_dict)

    return x, y
