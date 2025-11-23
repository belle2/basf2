##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import torch
from torch_geometric.nn import MetaLayer
from .geometric_layers import NodeLayer, EdgeLayer, GlobalLayer


class GraFEIModel(torch.nn.Module):
    """
    Actual implementation of the model,
    based on the
    `MetaLayer <https://pytorch-geometric.readthedocs.io/en/latest/generated/torch_geometric.nn.models.MetaLayer.html>`_
    class.

    .. seealso::
        `Relational inductive biases, deep learning, and graph networks <https://arxiv.org/abs/1806.01261>`_

    The network is composed of:

    1. A first MetaLayer to increase the number of nodes and edges features;
    2. A number of intermediate MetaLayers (tunable in config file);
    3. A last MetaLayer to decrease the number of node and edge features to the desired output dimension.

    .. figure:: figs/graFEI.png
        :width: 42em
        :align: center

    Each MetaLayer is in turn composed of `EdgeLayer`, `NodeLayer` and `GlobalLayer` sub-blocks.

    Args:
        nfeat_in_dim (int): Node features dimension (number of input node features).
        efeat_in_dim (int): Edge features dimension (number of input edge features).
        gfeat_in_dim (int): Global features dimension (number of input global features).
        edge_classes (int): Edge features output dimension (i.e. number of different edge labels in the LCAS matrix).
        x_classes (int): Node features output dimension (i.e. number of different mass hypotheses).
        hidden_layer_dim (int): Intermediate features dimension (same for node, edge and global).
        num_hid_layers (int): Number of hidden layers in every MetaLayer.
        num_ML (int): Number of intermediate MetaLayers.
        dropout (float): Dropout rate :math:`r \\in [0,1]`.
        global_layer (bool): Whether to use global layer.

    :return: Node, edge and global features after model evaluation.
    :rtype: tuple(`Tensor <https://pytorch.org/docs/stable/tensors.html#torch.Tensor>`_)
    """

    def __init__(
        self,
        nfeat_in_dim,
        efeat_in_dim,
        gfeat_in_dim,
        edge_classes=6,
        x_classes=7,
        hidden_layer_dim=128,
        num_hid_layers=1,
        num_ML=1,
        dropout=0.0,
        global_layer=True,
        **kwargs
    ):
        """
        Initialization.
        """
        super(GraFEIModel, self).__init__()

        #: First MetaLayer
        self.first_ML = MetaLayer(
            EdgeLayer(
                nfeat_in_dim,
                efeat_in_dim,
                gfeat_in_dim,
                hidden_layer_dim,
                hidden_layer_dim,
                num_hid_layers,
                dropout,
            ),
            NodeLayer(
                nfeat_in_dim,
                hidden_layer_dim,
                gfeat_in_dim,
                hidden_layer_dim,
                hidden_layer_dim,
                num_hid_layers,
                dropout,
            ),
            GlobalLayer(
                hidden_layer_dim,
                hidden_layer_dim,
                gfeat_in_dim,
                hidden_layer_dim,
                hidden_layer_dim,
                num_hid_layers,
                dropout,
            )
            if global_layer
            else None,
        )

        #: Intermediate MetaLayers
        self.ML_list = torch.nn.ModuleList(
            [
                MetaLayer(
                    EdgeLayer(
                        hidden_layer_dim,
                        hidden_layer_dim,
                        hidden_layer_dim if global_layer else 0,
                        hidden_layer_dim,
                        hidden_layer_dim,
                        num_hid_layers,
                        dropout,
                    ),
                    NodeLayer(
                        hidden_layer_dim,
                        hidden_layer_dim,
                        hidden_layer_dim if global_layer else 0,
                        hidden_layer_dim,
                        hidden_layer_dim,
                        num_hid_layers,
                        dropout,
                    ),
                    GlobalLayer(
                        hidden_layer_dim,
                        hidden_layer_dim,
                        hidden_layer_dim,
                        hidden_layer_dim,
                        hidden_layer_dim,
                        num_hid_layers,
                        dropout,
                    )
                    if global_layer
                    else None,
                )
                for _ in range(num_ML)
            ]
        )

        #: Output MetaLayer
        self.last_ML = MetaLayer(
            EdgeLayer(
                hidden_layer_dim,
                hidden_layer_dim,
                hidden_layer_dim if global_layer else 0,
                hidden_layer_dim,
                edge_classes,
                num_hid_layers,
                dropout,
                normalize=False,  # Do not normalize output layer
            ),
            NodeLayer(
                hidden_layer_dim,
                edge_classes,
                hidden_layer_dim if global_layer else 0,
                hidden_layer_dim,
                x_classes,
                num_hid_layers,
                dropout,
                normalize=False,  # Do not normalize output layer
            ),
            GlobalLayer(
                x_classes,
                edge_classes,
                hidden_layer_dim,
                hidden_layer_dim,
                1,
                num_hid_layers,
                dropout,
                normalize=False,  # Do not normalize output layer
            )
            if global_layer
            else None,
        )

    def forward(self, batch):
        """
        Called internally by PyTorch to propagate the input through the network.
        """
        x, u, edge_index, edge_attr, torch_batch = (
            batch.x,
            batch.u,
            batch.edge_index,
            batch.edge_attr,
            batch.batch,
        )

        x, edge_attr, u = self.first_ML(
            x=x, edge_index=edge_index, edge_attr=edge_attr, u=u, batch=torch_batch
        )

        del batch

        for ML in self.ML_list:
            x_skip = x
            edge_skip = edge_attr
            u_skip = u

            x, edge_attr, u = ML(
                x=x, edge_index=edge_index, edge_attr=edge_attr, u=u, batch=torch_batch
            )

            # Skip connections are added
            x += x_skip
            edge_attr += edge_skip
            u += u_skip

            del x_skip, edge_skip, u_skip

        x, edge_attr, u = self.last_ML(
            x=x, edge_index=edge_index, edge_attr=edge_attr, u=u, batch=torch_batch
        )

        # Edge labels are symmetrized
        edge_index_t = edge_index[[1, 0]]  # edge_index transposed

        for i in range(edge_attr.shape[1]):
            # edge_attr converted to sparse tensor...
            edge_matrix = torch.sparse_coo_tensor(
                edge_index, edge_attr[:, i]
            )
            # ... and its transposed
            edge_matrix_t = torch.sparse_coo_tensor(
                edge_index_t, edge_attr[:, i]
            )

            # Symmetrization happens here
            edge_attr[:, i] = (
                ((edge_matrix + edge_matrix_t) / 2.0).coalesce()
            ).values()

        return x, edge_attr, u
