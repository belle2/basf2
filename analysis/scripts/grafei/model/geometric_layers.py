import torch
import torch.nn as nn
import torch.nn.functional as F
from torch_scatter import scatter


def _init_weights(layer, normalize):
    """
    Initializes the weights and biases.
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
    """
        Updates edge features in MetaLayer:

        .. math::
            e_{ij}^{'} = \\phi^{e}(e_{ij}, v_{i}, v_{j}, u),

        where :math:`\\phi^{e}` is a neural network of the form

        .. figure:: figs/MLP_structure.png
          :width: 42em
          :align: center

        Args:
            nfeat_in_dim (int): Node features input dimension (number of node features in input).
            efeat_in_dim (int): Edge features input dimension (number of edge features in input).
            gfeat_in_dim (int): Gloabl features input dimension (number of global features in input).
            efeat_hid_dim (int): Edge features dimension in hidden layers.
            efeat_out_dim (int): Edge features output dimension.
            num_hid_layers (int): Number of hidden layers.
            dropout (float): Dropout rate :math:`r \\in [0,1]`.
            normalize (str): Type of normalization (batch/layer).


        :return: Updated edge features tensor.
        :rtype: `Tensor <https://pytorch.org/docs/stable/tensors.html#torch.Tensor>`_
    """

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

        _init_weights(self, self.normalize)

    def forward(self, src, dest, edge_attr, u, batch):
        """"""

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
    """
        Updates node features in MetaLayer:

        .. math::
            v_{i}^{'} = \\phi^{v}(v_{i}, \\rho^{e \\to v}(v_{i}), u)

        with

        .. math::
            \\rho^{e \\to v}(v_{i}) = \\frac{\\sum_{j=1,\\ j \\neq i}^{N} (e_{ji} + e _{ij})}{2 \\cdot (N-1)},

        where :math:`\\phi^{v}` is a neural network of the form

        .. figure:: figs/MLP_structure.png
          :width: 42em
          :align: center

        Args:
            nfeat_in_dim (int): Node features input dimension (number of node features in input).
            efeat_in_dim (int): Edge features input dimension (number of edge features in input).
            gfeat_in_dim (int): Gloabl features input dimension (number of global features in input).
            nfeat_hid_dim (int): Node features dimension in hidden layers.
            nfeat_out_dim (int): Node features output dimension.
            num_hid_layers (int): Number of hidden layers.
            dropout (float): Dropout rate :math:`r \\in [0,1]`.
            normalize (str): Type of normalization (batch/layer).

        :return: Updated node features tensor.
        :rtype: `Tensor <https://pytorch.org/docs/stable/tensors.html#torch.Tensor>`_
    """

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

        _init_weights(self, self.normalize)

    def forward(self, x, edge_index, edge_attr, u, batch):
        """"""

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
    """
        Updates node features in MetaLayer:

        .. math::
            u_{i}^{'} = \\phi^{u}(\\rho^{e \\to u}(e), \\rho^{v \\to u}(v), u)

        with

        .. math::
            \\rho^{e \\to u}(e) = \\frac{\\sum_{i,j=1,\\ i \\neq j}^{N} e_{ij}}{N \\cdot (N-1)},\\\\
            \\rho^{v \\to u}(e) = \\frac{\\sum_{i=1}^{N} v_{i}}{N},

        where :math:`\\phi^{u}` is a neural network of the form

        .. figure:: figs/MLP_structure.png
          :width: 42em
          :align: center

        Args:
            nfeat_in_dim (int): Node features input dimension (number of node features in input).
            efeat_in_dim (int): Edge features input dimension (number of edge features in input).
            gfeat_in_dim (int): Gloabl features input dimension (number of global features in input).
            nfeat_hid_dim (int): Global features dimension in hidden layers.
            nfeat_out_dim (int): Global features output dimension.
            num_hid_layers (int): Number of hidden layers.
            dropout (float): Dropout rate :math:`r \\in [0,1]`.
            normalize (str): Type of normalization (batch/layer).

        :return: Updated global features tensor.
        :rtype: `Tensor <https://pytorch.org/docs/stable/tensors.html#torch.Tensor>`_
    """

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

        _init_weights(self, self.normalize)

    def forward(self, x, edge_index, edge_attr, u, batch):
        """"""

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
