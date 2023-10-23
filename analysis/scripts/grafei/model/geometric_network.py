import torch
from torch_geometric.nn import MetaLayer
from .geometric_layers import NodeLayer, EdgeLayer, GlobalLayer


class GeometricNetwork(torch.nn.Module):
    """
    The network is composed of:
        1) First meta layer to increase the number of nodes and edges features
        2) num_ML different meta layers
        3) A last meta layer to decrease the number of nodes and edges features to efeat_out_dim

    MetaLayer: torch geomtric class that takes 2 layers as an input, and just calls their forward in its own forward
    https://pytorch-geometric.readthedocs.io/en/latest/modules/nn.html#torch_geometric.nn.meta.MetaLayer
    """

    def __init__(
        self,
        nfeat_in_dim,
        efeat_in_dim,
        gfeat_in_dim,
        edge_classes,
        x_classes,
        hidden_layer_dim=128,
        num_hid_layers=1,
        num_ML=1,
        dropout=0.0,
        normalize=None,
        symmetrize=True,
        global_layer=True,
        **kwargs
    ):
        super(GeometricNetwork, self).__init__()
        """
        nfeat_in_dim (efeat_in_dim): number of input feat per node (edge)
        hidden_layer_dim: number of node (edge) features when they enter/exit every MetaLayer
        num_hid_layers: number of hidden layers in every MetaLayer
        hidden_layer_dim: number of hidden neurons in every MetaLayer
        num_ML: number of MetaLayers used (except for the first and the last)
        edge_classes: number of final classes (i.e. number of different edge labels in the LCA matrix)
        """

        self.num_ML = num_ML
        self.symmetrize = symmetrize
        self.global_layer = global_layer

        self.first_ML = MetaLayer(
            EdgeLayer(
                nfeat_in_dim,
                efeat_in_dim,
                gfeat_in_dim,
                hidden_layer_dim,
                hidden_layer_dim,
                num_hid_layers,
                dropout,
                normalize,
            ),
            NodeLayer(
                nfeat_in_dim,
                hidden_layer_dim,
                gfeat_in_dim,
                hidden_layer_dim,
                hidden_layer_dim,
                num_hid_layers,
                dropout,
                normalize,
            ),
            GlobalLayer(
                hidden_layer_dim,
                hidden_layer_dim,
                gfeat_in_dim,
                hidden_layer_dim,
                hidden_layer_dim,
                num_hid_layers,
                dropout,
                normalize,
            )
            if self.global_layer
            else None,
        )
        self.ML_list = torch.nn.ModuleList(
            [
                MetaLayer(
                    EdgeLayer(
                        hidden_layer_dim,
                        hidden_layer_dim,
                        hidden_layer_dim if self.global_layer else 0,
                        hidden_layer_dim,
                        hidden_layer_dim,
                        num_hid_layers,
                        dropout,
                        normalize,
                    ),  # Increased input dimensionality because of skip connections
                    NodeLayer(
                        hidden_layer_dim,
                        hidden_layer_dim,
                        hidden_layer_dim if self.global_layer else 0,
                        hidden_layer_dim,
                        hidden_layer_dim,
                        num_hid_layers,
                        dropout,
                        normalize,
                    ),
                    GlobalLayer(
                        hidden_layer_dim,
                        hidden_layer_dim,
                        hidden_layer_dim,
                        hidden_layer_dim,
                        hidden_layer_dim,
                        num_hid_layers,
                        dropout,
                        normalize,
                    )
                    if self.global_layer
                    else None,
                )
                for _ in range(self.num_ML)
            ]
        )
        self.last_ML = MetaLayer(
            EdgeLayer(
                hidden_layer_dim,
                hidden_layer_dim,
                hidden_layer_dim if self.global_layer else 0,
                hidden_layer_dim,
                edge_classes,
                num_hid_layers,
                dropout,
                normalize=None,
            ),  # Do not normalize output layer
            NodeLayer(
                hidden_layer_dim,
                edge_classes,
                hidden_layer_dim if self.global_layer else 0,
                hidden_layer_dim,
                x_classes,
                num_hid_layers,
                dropout,
                normalize=None,
            ),
            GlobalLayer(
                x_classes,
                edge_classes,
                hidden_layer_dim,
                hidden_layer_dim,
                1,
                num_hid_layers,
                dropout,
                normalize=None,
            )
            if self.global_layer
            else None,
        )

    def forward(self, batch):
        """
        x: node features [N, F_x]
        edge_index: index of receiver and sender nodes for every edge [2, E]
        edge_attr: edge features [E, F_e]
        u: global attribute [B, F_g]
        torch_batch: vector that maps every node of a batch to its graph.
                            Note that geometric_dataloader returns batch (i.e. mini-batch),
                            and this holds the batch vector, called batch.batch.
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

            # x = torch.cat([x, x_skip], dim=1)  # Skip connections are concatenated
            # edge_attr = torch.cat([edge_attr, edge_skip], dim=1)
            # u = torch.cat([u, u_skip], dim=1)
            x += x_skip  # Skip connections are added
            edge_attr += edge_skip
            u += u_skip

            del x_skip, edge_skip, u_skip

        x, edge_attr, u = self.last_ML(
            x=x, edge_index=edge_index, edge_attr=edge_attr, u=u, batch=torch_batch
        )

        # Edge labels are symmetrized
        if self.symmetrize:
            edge_index_t = edge_index[[1, 0]]  # edge_index transposed

            for i in range(edge_attr.shape[1]):
                edge_matrix = torch.sparse_coo_tensor(
                    edge_index, edge_attr[:, i]
                )  # edge_attr converted to sparse tensor...
                edge_matrix_t = torch.sparse_coo_tensor(
                    edge_index_t, edge_attr[:, i]
                )  # ... and its transposed

                edge_attr[:, i] = (
                    ((edge_matrix + edge_matrix_t) / 2.0).coalesce()
                ).values()  # Symmetrization happens here

        return x, edge_attr, u
