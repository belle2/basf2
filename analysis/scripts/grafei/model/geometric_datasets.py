##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import itertools
from pathlib import Path
import numpy as np
import torch
from .tree_utils import masses_to_classes
from .dataset_utils import populate_avail_samples, preload_root_data
from .edge_features import compute_edge_features
from .normalize_features import normalize_features
from torch_geometric.data import Data, InMemoryDataset
import uproot


def _preload(self):
    """
    Creates graph objects and stores them into a python list.
    """

    #: Going to use x_files as an array that always exists
    self.x_files = sorted(self.root.glob("**/*.root"))

    # Select the first N files (useful for testing)
    if self.n_files is not None:
        if self.n_files > len(self.x_files):
            print(
                f"WARNING: n_files specified ({self.n_files}) is greater than files in path given ({len(self.x_files)})"
            )

        self.x_files = self.x_files[: self.n_files]

    if len(self.x_files) == 0:
        raise RuntimeError(f"No files found in {self.root}")

    # Save the features
    with uproot.open(self.x_files[0])["Tree"] as t:
        #: Features
        self.features = [f for f in t.keys() if f.startswith("feat_")]
        #: B reco flag
        self.B_reco = int(t["isB"].array(library="np")[0])
        assert self.B_reco in [0, 1, 2], "B_reco should be 0, 1 or 2, something went wrong"

    #: Discarded features
    self.discarded = [
        f for f in self.features if not f[f.find("_") + 1:] in self.node_features
    ]
    self.features = [
        f"feat_{f}" for f in self.node_features if f"feat_{f}" in self.features
    ]

    print(f"Input node features: {self.features}")
    print(f"Discarded node features: {self.discarded}")

    #: Edge features
    self.edge_features = [f"edge_{f}" for f in self.edge_features]
    #: Global features
    self.global_features = [f"glob_{f}" for f in self.global_features] if self.global_features else []
    print(f"Input edge features: {self.edge_features}")
    print(f"Input global features: {self.global_features}")

    #: Preload data
    self.x, self.y = preload_root_data(
        self.x_files,
        self.features,
        self.discarded,
    )

    #: Need to populate a list of available training samples
    self.avail_samples = populate_avail_samples(
        self.x,
        self.y,
        self.B_reco,
    )

    # Select a subset of available samples if requested
    if self.samples and self.samples < len(self.avail_samples):
        print(f"Selecting random subset of {self.samples} samples")
        self.avail_samples = [
            self.avail_samples[i]
            for i in np.random.choice(
                len(self.avail_samples), self.samples, replace=False
            )
        ]
    elif self.samples and (self.samples >= len(self.avail_samples)):
        print(
            f"WARNING: No. samples specified ({self.samples}) exceeds number of samples loaded ({len(self.avail_samples)})"
        )

    return len(self.avail_samples)


def _process_graph(self, idx):
    """
    Actually builds the graph object.

    Args:
        idx (int): Index of training example to be processed.

    Returns:
        torch_geometric.data.Data: Graph object to be used in training.
    """

    file_id, evt, p_index = self.avail_samples[idx]

    x_item = self.x[file_id]
    y_item = self.y[file_id][p_index]

    evt_b_index = x_item["b_index"][evt]
    evt_leaves = x_item["leaves"][evt]
    evt_primary = x_item["primary"][evt]

    y_leaves = y_item["LCA_leaves"][evt]
    # Use this to correctly reshape LCA (might be able to just use shape of y_leaves?)
    n_LCA = y_item["n_LCA"][evt]

    # Get the rows of the X inputs to fetch
    # This is a boolean numpy array
    x_rows = (evt_b_index != -1) if not self.B_reco else evt_b_index == int(p_index)

    # Find the unmatched particles
    unmatched_rows = evt_b_index == -1

    if np.any(unmatched_rows) and self.B_reco:
        # Create a random boolean array the same size as the number of leaves
        rand_mask = np.random.choice(a=[False, True], size=unmatched_rows.size)
        # AND the mask with the unmatched leaves
        # This selects a random subset of the unmatched leaves
        unmatched_rows = np.logical_and(unmatched_rows, rand_mask)

    # Add the unmatched rows to the current decay's rows
    x_rows = np.logical_or(x_rows, unmatched_rows)

    # Here we actually load the data

    # Initialise event's X array
    x = np.empty((x_rows.sum(), len(self.features)))
    x_dis = np.empty((x_rows.sum(), len(self.discarded)))

    # And populate it
    for idx, feat in enumerate(self.features):
        x[:, idx] = x_item["features"][feat][evt][x_rows]
    for idx, feat in enumerate(self.discarded):
        x_dis[:, idx] = x_item["discarded"][feat][evt][x_rows]

    # Same for edge and global features
    x_edges = (
        compute_edge_features(
            self.edge_features,
            self.features + self.discarded,
            np.concatenate([x, x_dis], axis=1),
        )
        if self.edge_features is not []
        else []
    )
    x_global = (
        np.array(
            [
                [
                    x_item["global"][feat + f"_{p_index}"][evt]
                    for feat in self.global_features
                ]
            ]
        )
        if self.global_features != []
        else []
    )

    x_leaves = evt_leaves[x_rows]

    # Set nans to zero, this is a surrogate value, may change in future
    np.nan_to_num(x, copy=False)
    np.nan_to_num(x_edges, copy=False)
    np.nan_to_num(x_global, copy=False)

    # Normalize any features that should be
    if self.normalize is not None:
        normalize_features(
            self.normalize,
            self.features,
            x,
            self.edge_features,
            x_edges,
            self.global_features,
            x_global,
        )

    # Reorder LCA

    # Get LCA indices in order that the leaves appear in reconstructed particles
    # Secondaries aren't in the LCA leaves list so they get a 0
    locs = np.array(
        [
            np.where(y_leaves == i)[0].item() if (i in y_leaves) else 0
            for i in x_leaves
        ]
    )

    # Get the LCA in the correct subset order
    # If we're not allowing secondaries this is all we need
    # If we are this will contain duplicates (since secondary locs are set to 0)
    # We can't load the firs locs directly (i.e. y_item[locs, :]) because locs is (intentionally) unsorted
    y_edge = y_item["LCA"][evt].reshape((n_LCA, n_LCA)).astype(int)
    # Get the true mcPDG pf FSPs
    y_mass = masses_to_classes(x_item["mc_pdg"][evt][x_rows])

    # Get the specified row/cols, this inserts dummy rows/cols for secondaries
    y_edge = y_edge[locs, :][:, locs]
    # if self.allow_secondaries:
    # Set everything that's not primary (unmatched and secondaries) rows.cols to 0
    # Note we only consider the subset of leaves that made it into x_rows
    y_edge = np.where(evt_primary[x_rows], y_edge, 0)  # Set the rows
    y_edge = np.where(evt_primary[x_rows][:, None], y_edge, 0)  # Set the columns

    # Set diagonal to -1 (actually not necessary but ok...)
    np.fill_diagonal(y_edge, -1)

    n_nodes = x.shape[0]

    # Target edge tensor: shape [E]
    edge_y = torch.tensor(
        y_edge[np.eye(n_nodes) == 0],
        dtype=torch.long
    )
    # Fill tensor with edge indices: shape [N*(N-1), 2] == [E, 2]
    edge_index = torch.tensor(
        list(itertools.permutations(range(n_nodes), 2)),
        dtype=torch.long,
    )

    # Target global tensor: shape [B, F_g]
    u_y = torch.tensor(
        [[1]], dtype=torch.float
    )

    # Target node tensor: shape [N]
    x_y = torch.tensor(y_mass, dtype=torch.long)

    g = Data(
        x=torch.tensor(x, dtype=torch.float),
        edge_index=edge_index.t().contiguous(),
        edge_attr=torch.tensor(x_edges, dtype=torch.float),
        u=torch.tensor(x_global, dtype=torch.float),
        x_y=x_y,
        edge_y=edge_y,
        u_y=u_y,
    )

    return g


class GraphDataSet(InMemoryDataset):
    """
    Dataset handler for converting Belle II data to PyTorch geometric InMemoryDataset.

    The ROOT format expects the tree in every file to be named ``Tree``,
    and all node features to have the format ``feat_FEATNAME``.

    .. note:: This expects the files under root to have the structure ``root/**/<file_name>.root``
        where the root path is different for train and val.
        The ``**/`` is to handle subdirectories, e.g. ``sub00``.

    Args:
        root (str): Path to ROOT files.
        n_files (int): Load only ``n_files`` files.
        samples (int): Load only ``samples`` events.
        features (list): List of node features names.
        edge_features (list): List of edge features names.
        global_features (list): List of global features names.
        normalize (bool): Whether to normalize input features.
    """

    def __init__(
        self,
        root,
        n_files=None,
        samples=None,
        features=[],
        edge_features=[],
        global_features=[],
        normalize=None,
        **kwargs,
    ):
        """
        Initialization.
        """
        assert isinstance(
            features, list
        ), f'Argument "features" must be a list and not {type(features)}'
        assert len(features) > 0, "You need to use at least one node feature"

        #: Root path
        self.root = Path(root)

        #: Normalize
        self.normalize = normalize

        #: Number of files
        self.n_files = n_files
        #: Node features
        self.node_features = features
        #: Edge features
        self.edge_features = edge_features
        #: Global features
        self.global_features = global_features
        #: Samples
        self.samples = samples

        # Delete processed files, in case
        file_path = Path(self.root, "processed")
        files = list(file_path.glob("*.pt"))
        for f in files:
            f.unlink(missing_ok=True)

        # Needs to be called after having assigned all attributes
        super().__init__(root, None, None, None)

        #: Data and Slices
        self.data, self.slices = torch.load(self.processed_paths[0])

    @property
    def processed_file_names(self):
        """
        Name of processed file.
        """
        return ["processed_data.pt"]

    def process(self):
        """
        Processes the data to create graph objects and stores them in ``root/processed/processed_data.pt``
        where the root path is different for train and val.

        Called internally by PyTorch.
        """
        num_samples = _preload(self)
        data_list = [_process_graph(self, i) for i in range(num_samples)]
        data, slices = self.collate(data_list)
        torch.save((data, slices), self.processed_paths[0])

        #: delete attributes
        del self.x, self.y, self.avail_samples, data_list, data, slices
