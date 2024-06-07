##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import math
import awkward as ak
import numpy as np
import dgl
import torch

from smartBKG import DEFAULT_NODE_FEATURES


def get_batched_graph(array, node_features, index_column=("particles", "motherIndex")):
    """
    Generate a batched DGL graph from an awkward array.

    Arguments:
        array (awkward array): containing particle information.
        node_features (dict): mapping field names to columns in the array.
        index_column (list): Column names for indexing.

    Returns:
        dgl.DGLGraph: Batched DGL graph.

    Note:
        This function assumes the input array has a nested structure representing particle relations.
    """
    mother_index = array[index_column]
    array_index = ak.local_index(mother_index, axis=1)

    src, dst = (
        ak.concatenate([mother_index, array_index], axis=1),
        ak.concatenate([array_index, mother_index], axis=1),
    )
    # remove edges to mothers that have been removed
    # (represented by index -1)
    # also remove self-loops (src == dst)
    mask = (src != -1) & (dst != -1) & (src != dst)
    src = src[mask]
    dst = dst[mask]

    # now add a single self-loop for each array index
    src, dst = (
        ak.concatenate([src, array_index], axis=1),
        ak.concatenate([dst, array_index], axis=1),
    )

    # calculate offsets explicitly such that it works also for ListArray
    offsets = np.append(0, np.cumsum(ak.num(mother_index).to_numpy())[:-1])

    # add offsets such that we get a single graph of disconnected node groups
    src_flat, dst_flat = (
        torch.tensor(ak.to_numpy(ak.flatten(src + offsets))),
        torch.tensor(ak.to_numpy(ak.flatten(dst + offsets))),
    )

    batched = dgl.graph((src_flat, dst_flat))
    batched.set_batch_num_nodes(torch.tensor(ak.num(mother_index).to_numpy()))
    batched.set_batch_num_edges(torch.tensor(ak.num(src).to_numpy()))
    for field, columns in node_features.items():
        feats = array[columns]
        if len(feats.fields) == 0:
            flat_feats = ak.to_numpy(ak.flatten(feats), allow_missing=False)
        else:
            flat_feats = np.stack(
                [
                    ak.to_numpy(ak.flatten(x), allow_missing=False)
                    for x in ak.unzip(feats)
                ],
                axis=1
            )
        batched.ndata[field] = torch.tensor(flat_feats, dtype=torch.float32)

    return batched


class ArrayDataset(torch.utils.data.IterableDataset):
    """
    Dataset initialized from a pre-processed awkward array.

    Use `torch.utils.data.DataLoader` with `collate_fn=lambda x: x[0]`
    and `batch_size=1` to iterate through it.

    Yields a tuple of a batched dgl graph and labels. Optionally also weights if
    `weighted=True`. This requires a column `weight` in the array.
    """

    def __init__(
        self,
        array,
        batch_size=1024,
        shuffle=True,
        seed=None,
        weighted=False,
    ):
        """
        Initialize the ArrayDataset for Pytorch training and inference.

        :param array: Awkward array containing the dataset.
        :param batch_size (int): Batch size for the iterable dataset.
        :param shuffle (bool): Whether to shuffle the data.
        :param seed: Random seed for shuffling.
        :param weighted (bool): Whether the dataset includes weights.
        """
        #: Awkward array containing the dataset
        self.array = array
        #: Batch size for the iterable dataset
        self.batch_size = batch_size
        #: Whether to shuffle the data
        self.shuffle = shuffle
        #: Random seed for shuffling, consistent seed for all workers
        self.seed = seed if seed is not None else np.random.SeedSequence().entropy
        #: Whether the dataset includes weights
        self.weighted = weighted

    def __len__(self):
        """
        Get the number of batches.

        Returns:
            int: Number of batches.
        """
        return int(math.ceil(len(self.array) / self.batch_size))

    def maybe_permuted(self, array):
        """
        Possibly permute the array based on the shuffle parameter.

        Arguments:
            array (awkward array): Input array.

        Returns:
            array: Permuted or original array.
        """
        if not self.shuffle or len(self.array) == 1:
            return array
        perm = np.random.default_rng(self.seed).permutation(len(array))
        return self.array[perm]

    @staticmethod
    def to_tensor(array):
        """
        Convert an awkward array to a torch tensor.

        Arguments:
            array (awkward array): Input awkward array.

        Returns:
            torch.Tensor: Converted tensor.
        """
        return torch.tensor(
            ak.to_numpy(array, allow_missing=False),
            dtype=torch.float32,
        ).reshape(-1, 1)

    def __iter__(self):
        """
        Iterate over batches with changing random seeds.

        Yields:
            tuple: Batched dgl graph, labels, and optionally weights.
        """
        worker_info = torch.utils.data.get_worker_info()
        if worker_info is not None:
            num_workers = worker_info.num_workers
            worker_id = worker_info.id
        else:
            num_workers = 1
            worker_id = 0
        array = self.maybe_permuted(self.array)
        starts = list(range(0, len(self.array), self.batch_size))
        per_worker = np.array_split(starts, num_workers)
        for start in per_worker[worker_id]:
            ak_slice = array[start: start + self.batch_size]
            output = [
                get_batched_graph(ak_slice, DEFAULT_NODE_FEATURES),
                self.to_tensor(ak_slice.label),
            ]
            if self.weighted:
                output.append(self.to_tensor(ak_slice.weight))
            yield tuple(output)
        # increase the seed to get a new order of instances in the next iteration
        # note: need to use persistent_workers=True in the DataLoader for this to take effect
        self.seed += 1

    def __getitem__(self, slicer):
        """
        Get a single instance or a new ArrayDataset for a slice.

        Arguments:
            slicer (int or slice): Index or slice.

        Returns:
            ArrayDataset: New ArrayDataset instance.
        """
        kwargs = dict(
            batch_size=self.batch_size,
            shuffle=self.shuffle,
            seed=self.seed,
            weighted=self.weighted,
        )
        array = self.maybe_permuted(self.array)
        if not isinstance(slicer, int):
            return ArrayDataset(array[slicer], **kwargs)
        slicer = slice(slicer, slicer + 1)
        kwargs["batch_size"] = 1
        return next(iter(ArrayDataset(array[slicer], **kwargs)))
