import torch
from torch_geometric.nn import MetaLayer
from .geometric_layers import NodeLayer, EdgeLayer, GlobalLayer


class GeometricNetwork(torch.nn.Module):
    def __init__(
        self,
        nfeat_in_dim,
        efeat_in_dim,
        gfeat_in_dim,
        edge_classes,
        x_classes=7,
        hidden_layer_dim=128,
        num_hid_layers=1,
        num_ML=1,
        dropout=0.0,
        normalize=None,
        symmetrize=True,
        global_layer=True,
        **kwargs
    ):
        """
        Actual implementation of the model.
        The network is composed of:
            1) First meta layer to increase the number of nodes and edges features
            2) num_ML different meta layers
            3) A last meta layer to decrease the number of nodes and edges features

        MetaLayer: torch geomtric class that takes layers as an input, and calls their forward in its own forward
        https://pytorch-geometric.readthedocs.io/en/latest/modules/nn.html#torch_geometric.nn.meta.MetaLayer

        Args:
            nfeat_in_dim (int): node features dimension (number of node features)
            ... (same for edge and global features)
            edge_classes (int): number of edge classes (i.e. number of different edge labels in the LCAS matrix)
            x_classes (int): number of node classes (i.e. number of different mass hypotheses)
            hidden_layer_dim (int): number of features when they enter/exit every MetaLayer
            num_hid_layers (int): number of hidden layers in every MetaLayer
            num_ML (int): number of MetaLayers used (except for the first and the last)
            droput (float): dropout rate
            normalize (string): type of normalization used (layer/batch)
            symmetrize (bool): whether to symmetrize CAS matrix at the end
            global_layer (bool): whether or not to use global layer

        Returns:
            x (torch.tensor): node features after model evaluation
            edge_attr (torch.tensor): edge features after model evaluation
            u (torch.tensor): global features after model evaluation

        """
        super(GeometricNetwork, self).__init__()

        self.symmetrize = symmetrize
        self.x_classes = x_classes
        self.edge_classes = edge_classes

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
            if global_layer
            else None,
        )
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
                        normalize,
                    ),  # Increased input dimensionality because of skip connections
                    NodeLayer(
                        hidden_layer_dim,
                        hidden_layer_dim,
                        hidden_layer_dim if global_layer else 0,
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
                    if global_layer
                    else None,
                )
                for _ in range(num_ML)
            ]
        )
        self.last_ML = MetaLayer(
            EdgeLayer(
                hidden_layer_dim,
                hidden_layer_dim,
                hidden_layer_dim if global_layer else 0,
                hidden_layer_dim,
                edge_classes,
                num_hid_layers,
                dropout,
                normalize=None,
            ),  # Do not normalize output layer
            NodeLayer(
                hidden_layer_dim,
                edge_classes,
                hidden_layer_dim if global_layer else 0,
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
            if global_layer
            else None,
        )

    def forward(self, batch):
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

    def getNXClasses(self):
        """
        Returns the number of node classes in the model.
        """
        return self.x_classes

    def getNEClasses(self):
        """
        Returns the number of edge classes in the model.
        """
        return self.edge_classes
