from __future__ import division  # int divided by int = float, just to be sure
import torch
import torch.nn as nn
import torch.nn.functional as F
from torch_scatter import scatter


def init_weights(layer, normalize):
    """
    Initialize the weights and biases.

    Args:
        layer (torch.nn.Module): pytorch module
        normalize (string): whether to fill biases or not
    """
    for m in layer.modules():
        if isinstance(m, nn.Linear):
            nn.init.xavier_normal_(m.weight.data)
            if normalize is None:
                m.bias.data.fill_(0.1)
        elif isinstance(m, nn.BatchNorm1d) or isinstance(m, nn.LayerNorm):
            m.weight.data.fill_(1)
            m.bias.data.zero_()


class EdgeLayer(nn.Module):
    def __init__(
        self,
        nfeat_in_dim,
        efeat_in_dim,
        gfeat_in_dim,
        efeat_hid_dim,
        efeat_out_dim,
        num_hid_layers,
        dropout,
        normalize,
    ):
        """
        Module to update edges in MetaLayer.
           e_ij' = f(e_ij, v_i, v_j)

        Args:
            nfeat_in_dim (int): node features input dimension (number of node features in input)
            ... (same for edge and global features)
            efeat_hid_dim (int): edge features dimension in hidden layers
            efeat_out_dim (int): edge features output dimension
            num_hid_layers (int) number of hidden layers
            dropout (float): dropout rate
            normalize (string): type of normalization (batch/layer)

        Returns:
            out (torch.tensor): updated edge features tensor
        """
        super(EdgeLayer, self).__init__()

        self.nonlin_function = F.elu
        self.num_hid_layers = num_hid_layers
        self.dropout_prob = dropout
        self.normalize = normalize

        out_bias = (
            True if self.normalize is None else False
        )  # Bias in lin_out activated only if batchnorm/layernorm not used

        self.lin_in = nn.Linear(
            efeat_in_dim + 2 * nfeat_in_dim + gfeat_in_dim, efeat_hid_dim
        )
        self.lins_hid = nn.ModuleList(
            [
                nn.Linear(efeat_hid_dim, efeat_hid_dim)
                for _ in range(self.num_hid_layers)
            ]
        )
        self.lin_out = nn.Linear(efeat_hid_dim, efeat_out_dim, bias=out_bias)

        if self.normalize == "batchnorm":
            self.norm = nn.BatchNorm1d(efeat_out_dim)
        elif self.normalize == "layernorm":
            self.norm = nn.LayerNorm(efeat_out_dim)

        init_weights(self, self.normalize)

    def forward(self, src, dest, edge_attr, u, batch):
        # src, dest: [E, F_x], where E is the number of edges.
        # edge_attr: [E, F_e]
        # u: [B, F_u], where B is the number of graphs.
        # batch: [E] with max entry B - 1.

        out = (
            torch.cat([edge_attr, src, dest, u[batch]], dim=1)
            if u.shape != torch.Size([0])
            else torch.cat([edge_attr, src, dest], dim=1)
        )

        out = self.nonlin_function(self.lin_in(out))
        out = F.dropout(out, self.dropout_prob, training=self.training)

        out_skip = out

        for lin_hid in self.lins_hid:
            out = self.nonlin_function(lin_hid(out))
            out = F.dropout(out, self.dropout_prob, training=self.training)

        if self.num_hid_layers > 1:
            out += out_skip

        if self.normalize is not None:
            out = self.nonlin_function(self.norm(self.lin_out(out)))
        else:
            out = self.nonlin_function(self.lin_out(out))

        return out


class NodeLayer(nn.Module):
    def __init__(
        self,
        nfeat_in_dim,
        efeat_in_dim,
        gfeat_in_dim,
        nfeat_hid_dim,
        nfeat_out_dim,
        num_hid_layers,
        dropout,
        normalize,
    ):
        """
        Module to update nodes in MetaLayer.
           v_i' = f(v_i, Avg(e_i1, ..., e_ik, e_1i, ..., e_ki))

        Args:
            nfeat_in_dim (int): node features input dimension (number of node features in input)
            ... (same for edge and global features)
            nfeat_hid_dim (int): node features dimension in hidden layers
            nfeat_out_dim (int): node features output dimension
            num_hid_layers (int) number of hidden layers
            dropout (float): dropout rate
            normalize (string): type of normalization (batch/layer)

        Returns:
            out (torch.tensor): updated node features tensor
        """
        super(NodeLayer, self).__init__()

        self.nonlin_function = F.elu
        self.num_hid_layers = num_hid_layers
        self.dropout_prob = dropout
        self.normalize = normalize

        out_bias = (
            True if self.normalize is None else False
        )  # Bias in lin_out activated only if batchnorm/layernorm not used

        self.lin_in = nn.Linear(
            gfeat_in_dim + nfeat_in_dim + efeat_in_dim, nfeat_hid_dim
        )
        self.lins_hid = nn.ModuleList(
            [
                nn.Linear(nfeat_hid_dim, nfeat_hid_dim)
                for _ in range(self.num_hid_layers)
            ]
        )
        self.lin_out = nn.Linear(nfeat_hid_dim, nfeat_out_dim, bias=out_bias)

        if self.normalize == "batchnorm":
            self.norm = nn.BatchNorm1d(nfeat_out_dim)
        elif self.normalize == "layernorm":
            self.norm = nn.LayerNorm(nfeat_out_dim)

        init_weights(self, self.normalize)

    def forward(self, x, edge_index, edge_attr, u, batch):
        # x: [N, F_x], where N is the number of nodes.
        # edge_index: [2, E] with max entry N - 1.
        # edge_attr: [E, F_e]
        # u: [B, F_u]
        # batch: [N] with max entry B - 1.

        # Edge labels are averaged (dim_size = N: number of nodes in the graph)
        out = scatter(
            edge_attr, edge_index[1], dim=0, dim_size=batch.size(0), reduce="mean"
        )
        out = (
            torch.cat([x, out, u[batch]], dim=1)
            if u.shape != torch.Size([0])
            else torch.cat([x, out], dim=1)
        )

        out = self.nonlin_function(self.lin_in(out))
        out = F.dropout(out, self.dropout_prob, training=self.training)

        out_skip = out

        for lin_hid in self.lins_hid:
            out = self.nonlin_function(lin_hid(out))
            out = F.dropout(out, self.dropout_prob, training=self.training)

        if self.num_hid_layers > 1:
            out += out_skip

        if self.normalize is not None:
            out = self.nonlin_function(self.norm(self.lin_out(out)))
        else:
            out = self.nonlin_function(self.lin_out(out))

        return out


class GlobalLayer(nn.Module):
    def __init__(
        self,
        nfeat_in_dim,
        efeat_in_dim,
        gfeat_in_dim,
        gfeat_hid_dim,
        gfeat_out_dim,
        num_hid_layers,
        dropout,
        normalize,
    ):
        """
        Module to update global attribute in MetaLayer.
           u_i' = f(Avg(v_11, ..., v_jk), Avg(e_11, ..., e_jk))

        Args:
            nfeat_in_dim (int): node features input dimension (number of node features in input)
            ... (same for edge and global features)
            gfeat_hid_dim (int): global features dimension in hidden layers
            gfeat_out_dim (int): global features output dimension
            num_hid_layers (int) number of hidden layers
            dropout (float): dropout rate
            normalize (string): type of normalization (batch/layer)

        Returns:
            out (torch.tensor): updated global features tensor
        """
        super(GlobalLayer, self).__init__()

        self.nonlin_function = F.elu
        self.sigmoid = torch.sigmoid
        self.relu = F.relu
        self.num_hid_layers = num_hid_layers
        self.dropout_prob = dropout
        self.normalize = normalize

        out_bias = (
            True if self.normalize is None else False
        )  # Bias in lin_out activated only if batchnorm/layernorm not used

        self.lin_in = nn.Linear(
            nfeat_in_dim + efeat_in_dim + gfeat_in_dim, gfeat_hid_dim
        )
        self.lins_hid = nn.ModuleList(
            [
                nn.Linear(gfeat_hid_dim, gfeat_hid_dim)
                for _ in range(self.num_hid_layers)
            ]
        )
        self.lin_out = nn.Linear(gfeat_hid_dim, gfeat_out_dim, bias=out_bias)
        if self.normalize == "batchnorm":
            self.norm = nn.BatchNorm1d(gfeat_out_dim)
        elif self.normalize == "layernorm":
            self.norm = nn.LayerNorm(gfeat_out_dim)

        init_weights(self, self.normalize)

    def forward(self, x, edge_index, edge_attr, u, batch):
        # x: [N, F_x], where N is the number of nodes.
        # edge_index: [2, E] with max entry N - 1.
        # edge_attr: [E, F_e]
        # u: [B, F_u]
        # batch: [N] with max entry B - 1.

        node_mean = scatter(
            x, batch, dim=0, reduce="mean"
        )  # Nodes are averaged over graph
        edge_mean = scatter(
            edge_attr, edge_index[1], dim=0, reduce="mean"
        )  # Edges are averaged over nodes
        edge_mean = scatter(
            edge_mean, batch, dim=0, reduce="mean"
        )  # Edges are averaged over graph
        out = (
            torch.cat([u, node_mean, edge_mean], dim=1)
            if u.shape != torch.Size([0])
            else torch.cat([node_mean, edge_mean], dim=1)
        )

        out = self.nonlin_function(self.lin_in(out))
        out = F.dropout(out, self.dropout_prob, training=self.training)

        out_skip = out

        for lin_hid in self.lins_hid:
            out = self.nonlin_function(lin_hid(out))
            out = F.dropout(out, self.dropout_prob, training=self.training)

        if self.num_hid_layers > 1:
            out += out_skip

        if self.normalize is not None:
            out = self.nonlin_function(self.norm(self.lin_out(out)))
        else:
            out = self.lin_out(out)

        return out
