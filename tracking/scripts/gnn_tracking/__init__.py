##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F


def manual_knn(x, k):
    """
    Compute k-nearest neighbours for each node using a full pairwise distance matrix.

    For each node i, returns the indices of its k nearest neighbours (including itself
    at distance 0). Does not support batched graphs - assumes a single graph with no
    batch index vector.

    :param x: Node feature tensor of shape [N, D].
    :param k: Number of nearest neighbours to retrieve per node.
    :return:  Edge index tensor of shape [2, N*k], where row 0 contains source
              (sender) indices and row 1 contains target (receiver) indices.
    """
    d = torch.cdist(x, x)
    i = d.topk(k, largest=False).indices.reshape(-1)
    j = torch.arange(x.shape[0])
    j = torch.broadcast_to(j[:, np.newaxis], (x.shape[0], k)).reshape(-1)
    return torch.stack([j, i], axis=0)


class BatchNorm(nn.Module):
    """
    Thin wrapper around nn.BatchNorm1d to match the interface of
    ``torch_geometric.nn.norm.BatchNorm``, which accepts a single tensor argument.

    This allows ``BatchNorm`` to be used interchangeably with the PyG version
    without requiring ``torch_geometric`` as a dependency at inference time.

    Reference:
        https://pytorch-geometric.readthedocs.io/en/latest/_modules/torch_geometric/nn/norm/batch_norm.html#BatchNorm
    """

    def __init__(self, input_dim, momentum):
        """
        :param input_dim: Number of features (channels) of the input tensor.
        :param momentum:  Momentum for the running mean/variance update in ``BatchNorm1d``.
        """
        super().__init__()
        self.module = nn.BatchNorm1d(input_dim, momentum=momentum)

    def forward(self, x):
        """
        :param x: Input tensor of shape [N, input_dim].
        :return:  Batch-normalised tensor of the same shape.
        """
        return self.module(x)


class GravNetConv(nn.Module):
    """
    Single GravNet convolutional layer.

    Each node learns a position in a latent *space* (via ``lin_s``) and a message
    vector (via ``lin_h``).  Edges are formed by connecting every node to its k
    nearest neighbours in that latent space.  Edge weights decay exponentially
    with squared latent-space distance, so geometrically close nodes exchange
    stronger messages.  Aggregated messages (``mean`` and ``max``) are concatenated
    and projected to the output dimension, with a residual path from the input.

    This implementation replaces the scatter-based aggregation of the original
    PyG ``GravNetConv`` with a reshape-based aggregation that is compatible with
    ONNX export and pure-PyTorch inference:
      - Every node has exactly k neighbours (uniform neighbourhood size).
      - The KNN result is sorted by receiving node index, so messages can be
        reshaped directly to [N, k, propagate_dimensions] without scatter.

    Reference:
        Learning Representations of Irregular Particle-detector Geometry
        with Distance-weighted Graph Networks
        https://pytorch-geometric.readthedocs.io/en/latest/_modules/torch_geometric/nn/conv/gravnet_conv.html#GravNetConv
    """

    def __init__(self, in_channels=128, out_channels=128, space_dimensions=3, propagate_dimensions=8, k=4):
        """
        :param in_channels:          Number of input features per node.
        :param out_channels:         Number of output features per node.
        :param space_dimensions:     Dimensionality of the learned latent space used for KNN.
        :param propagate_dimensions: Dimensionality of the message vectors exchanged along edges.
        :param k:                    Number of nearest neighbours per node.
        """
        super().__init__()
        self.in_channels = in_channels
        self.out_channels = out_channels
        self.k = k
        self.lin_s = nn.Linear(in_channels, space_dimensions)
        self.lin_h = nn.Linear(in_channels, propagate_dimensions)
        self.lin_out1 = nn.Linear(in_channels, out_channels, bias=False)
        self.lin_out2 = nn.Linear(2 * propagate_dimensions, out_channels)

    def forward(self, x):
        """
        :param x: Node feature tensor of shape [N, in_channels].
        :return:  Updated node feature tensor of shape [N, out_channels].
        """
        # For less than k nodes we set k = n, since topk requires at least k elements
        k = torch.minimum(torch.tensor(x.size(0)), torch.tensor(self.k))

        h = self.lin_h(x)
        s = self.lin_s(x)

        ei, ej = manual_knn(s, k)

        # Edge weights: Gaussian kernel in latent space.
        # The factor of 10 sharpens the kernel, giving a better-spread weight distribution.
        edge_weight = (s[ei] - s[ej]).pow(2).sum(-1)
        edge_weight = torch.exp(-10. * edge_weight)
        msg = h[ej] * edge_weight.unsqueeze(1)  # [N*k, propagate_dimensions]

        # Reshape-based aggregation: valid because every node has exactly k neighbours
        # and messages are ordered by receiving node index (guaranteed by ``manual_knn``).
        # Concatenate mean- and max-pooled messages to capture different statistics.
        out = torch.cat(
            [
                msg.reshape(-1, k, msg.shape[-1]).mean(axis=1),
                msg.reshape(-1, k, msg.shape[-1]).amax(axis=1),
            ], axis=-1
        )  # [N, 2 * propagate_dimensions]

        return self.lin_out1(x) + self.lin_out2(out)


class CDCNet(nn.Module):
    """
    GNN for object condensation and track parameter regression on CDC hits.

    Architecture overview
    ---------------------
    1. Input batch normalisation.
    2. Global mean pooling is concatenated to node features before each block
       (global exchange), giving every node access to event-level context.
    3. A stack of ``nblocks`` GravNet blocks, each consisting of:
         - Two linear + ELU layers,
         - BatchNorm,
         - One linear + ELU layer,
         - GravNetConv (learned latent-space KNN + distance-weighted message passing),
         - BatchNorm.
       The output of each block is collected for skip connections.
    4. All block outputs are concatenated and passed through a dense layer.
    5. Five output heads predict:
         - ``beta``   : condensation score in [0, 1] (sigmoid), shape [N, 1].
         - ``ccoords``: latent clustering coordinates, shape [N, coord_dim].
         - ``p``      : 3-momentum vector (px, py, pz), shape [N, 3].
         - ``vertex`` : production vertex (vx, vy, vz), shape [N, 3].
         - ``charge`` : particle charge probability in [0, 1] (sigmoid), shape [N, 1].

    ONNX export constraints
    -----------------------
    Several design choices were made to allow the model to be exported to ONNX
    and run in pure PyTorch without torch_geometric:
      - Custom ``GravNetConv`` using ``manual_knn`` and reshape-based aggregation
        instead of scatter-based aggregation.
      - Batch size fixed to 1 (no batch index vector).
      - Standard ``nn.BatchNorm1d`` and global mean pooling instead of PyG variants.

    References
    ----------
    - Learning Representations of Irregular Particle-detector Geometry
      with Distance-weighted Graph Networks (https://doi.org/10.1140/epjc/s10052-019-7113-9)
    - End-to-End Multi-track Reconstruction Using Graph Neural Networks
      at Belle II (https://doi.org/10.1007/s41781-025-00135-6)
    """

    def __init__(
        self,
        input_dim,
        k=10,
        dim1=64,
        dim2=32,
        nblocks=4,
        coord_dim=2,
        space_dimensions=4,
        momentum=0.6,
    ):
        """
        :param input_dim:        Number of input features per node.
        :param k:                Number of nearest neighbours for each GravNetConv layer.
        :param dim1:             Hidden feature dimension inside each GravNet block.
        :param dim2:             Output feature dimension of each GravNet block
                                 (collected for skip connections).
        :param nblocks:          Number of GravNet blocks stacked in the network.
        :param coord_dim:        Dimensionality of the predicted clustering coordinates.
        :param space_dimensions: Dimensionality of the latent space learned by GravNetConv
                                 for KNN construction.
        :param momentum:         Momentum for all BatchNorm layers.
        """
        super().__init__()

        self.batch_norm_0 = BatchNorm(input_dim, momentum=0.6)

        # First block to start with input dimension
        # Input is (node features || global mean) → 2 * input_dim
        self.blocks = nn.ModuleList(
            [
                # Start with the first block according to input dimension
                nn.ModuleList(
                    [
                        nn.Linear(2 * input_dim, dim1),
                        nn.Linear(dim1, dim1),
                        BatchNorm(dim1, momentum=momentum),
                        nn.Linear(dim1, dim1),
                        GravNetConv(
                            in_channels=dim1,
                            out_channels=dim1 * 2,
                            space_dimensions=space_dimensions,
                            k=k,
                            propagate_dimensions=dim1,
                        ),
                        BatchNorm(dim1 * 2, momentum=momentum),
                        nn.Linear(dim1 * 2, dim2),
                    ]
                )
            ]
        )

        # Loop over remaining blocks as they are currently built the same
        # Input is (node features || global mean) → 4 * dim1
        # because node features are already dim1*2 after the first GravNetConv
        self.blocks.extend(
            nn.ModuleList(
                [
                    # Add according to number of blocks
                    nn.ModuleList(
                        [
                            nn.Linear(4 * dim1, dim1),
                            nn.Linear(dim1, dim1),
                            BatchNorm(dim1, momentum=momentum),
                            nn.Linear(dim1, dim1),
                            GravNetConv(
                                in_channels=dim1,
                                out_channels=dim1 * 2,
                                space_dimensions=space_dimensions,
                                k=k,
                                propagate_dimensions=dim1,
                            ),
                            # Edges, so need dim1 times 2
                            BatchNorm(dim1 * 2, momentum=momentum),
                            nn.Linear(dim1 * 2, dim2),
                        ]
                    )
                    for _ in range(nblocks - 1)
                ]
            )
        )

        # There are skip connections between the blocks,
        # this layer combines them, therefore scales with nblocks
        self.dense_cat = nn.Linear(dim2 * (nblocks), dim1)

        # These are the output layers for object condensation
        self.p_beta_layer = nn.Linear(dim1, 1)  # predict condensation point
        self.p_coords_layer = nn.Linear(dim1, coord_dim)  # predict latent space coordinates

        # These are the output layers for the track parameters
        self.p_p_layer = nn.Linear(dim1, 3)  # predict track momentum
        self.p_vertex_layer = nn.Linear(dim1, 3)  # predict track starting point
        self.p_charge_layer = nn.Linear(dim1, 1)  # predict track charge

    def forward(self, x):
        """
        Forward pass through CDCNet.

        :param x: Node feature tensor of shape [N, input_dim].
        :return:  Five-tuple of tensors, all of shape [N, *]:
                    - beta   [N, 1]:         condensation score in [0, 1].
                    - coords [N, coord_dim]: latent clustering coordinates.
                    - p      [N, 3]:         predicted 3-momentum (px, py, pz).
                    - vertex [N, 3]:         predicted production vertex (vx, vy, vz).
                    - charge [N, 1]:         charge probability in [0, 1].
        """

        features = []
        x = self.batch_norm_0(x)

        # Global exchange before the first block: broadcast event mean to all nodes
        out = x.mean(axis=0, keepdim=True)
        x = torch.cat(torch.broadcast_tensors(x, out), dim=-1)

        for i, block in enumerate(self.blocks):
            if i > 0:
                # Global exchange before each subsequent block
                out = x.mean(axis=0, keepdim=True)
                x = torch.cat(torch.broadcast_tensors(x, out), dim=-1)
            x = F.elu(block[0](x))
            x = F.elu(block[1](x))
            x = block[2](x)          # BatchNorm
            x = F.elu(block[3](x))
            x = block[4](x)          # GravNetConv
            x = block[5](x)          # BatchNorm
            features.append(F.elu(block[6](x)))  # collect for skip connection

        # Concatenate features and put through final dense neural network
        x = torch.cat(features, dim=1)
        x = F.elu(self.dense_cat(x))

        # Here are the networks for object condensation predictions
        p_beta = torch.sigmoid(self.p_beta_layer(x))
        p_coords = self.p_coords_layer(x)

        # Here are the networks for track parameters predictions
        p_p = self.p_p_layer(x)
        p_vertex = self.p_vertex_layer(x)
        p_charge = torch.sigmoid(self.p_charge_layer(x))

        return (
            p_beta,
            p_coords,
            p_p,
            p_vertex,
            p_charge,
        )
