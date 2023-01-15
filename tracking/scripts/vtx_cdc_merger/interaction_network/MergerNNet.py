#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


"""
This module implements the PyTorch modules that define a sparse
message-passing graph neural network.
"""

import torch
import torch.nn as nn
from torch_scatter import scatter_add


def make_mlp(input_size, sizes,
             hidden_activation='ReLU',
             output_activation='ReLU',
             layer_norm=False):
    """Construct an MLP with specified fully-connected layers."""
    hidden_activation = getattr(nn, hidden_activation)
    if output_activation is not None:
        output_activation = getattr(nn, output_activation)
    layers = []
    n_layers = len(sizes)
    sizes = [input_size] + sizes
    # Hidden layers
    for i in range(n_layers - 1):
        layers.append(nn.Linear(sizes[i], sizes[i + 1]))
        if layer_norm:
            layers.append(nn.LayerNorm(sizes[i + 1]))
        layers.append(hidden_activation())
    # Final layer
    layers.append(nn.Linear(sizes[-2], sizes[-1]))
    if output_activation is not None:
        if layer_norm:
            layers.append(nn.LayerNorm(sizes[-1]))
        layers.append(output_activation())
    return nn.Sequential(*layers)


class EdgeNetwork(nn.Module):
    """
    A module which computes weights for edges of the graph.
    For each edge, it selects the associated nodes' features
    and applies some fully-connected network layers with a final
    sigmoid activation.
    """

    def __init__(self, input_dim, hidden_dim=8, hidden_activation='ReLU',
                 layer_norm=True):
        super(EdgeNetwork, self).__init__()
        self.network = make_mlp(input_dim * 2,
                                [hidden_dim, hidden_dim, hidden_dim, 1],
                                hidden_activation=hidden_activation,
                                output_activation=None,
                                layer_norm=layer_norm)

    def forward(self, x, edge_index):
        # Select the features of the associated nodes
        start, end = edge_index
        edge_inputs = torch.cat([x[start], x[end]], dim=1)
        return self.network(edge_inputs).squeeze(-1)


class NodeNetwork(nn.Module):
    """
    A module which computes new node features on the graph.
    For each node, it aggregates the neighbor node features
    (separately on the input and output side), and combines
    them with the node's previous features in a fully-connected
    network to compute the new features.
    """

    def __init__(self, input_dim, output_dim, hidden_activation='ReLU',
                 layer_norm=True):
        super(NodeNetwork, self).__init__()
        self.network = make_mlp(input_dim * 3, [output_dim] * 4,
                                hidden_activation=hidden_activation,
                                output_activation=hidden_activation,
                                layer_norm=layer_norm)

    def forward(self, x, e, edge_index):
        start, end = edge_index
        # Aggregate edge-weighted incoming/outgoing features
        mi = scatter_add(e[:, None] * x[start], end, dim=0, dim_size=x.shape[0])
        mo = scatter_add(e[:, None] * x[end], start, dim=0, dim_size=x.shape[0])
        node_inputs = torch.cat([mi, mo, x], dim=1)
        return self.network(node_inputs)


class GlobalNetwork(nn.Module):
    """
    A module which computes global features of the graph.
    It aggregates the node features and feeds them into
    a decoder net.
    """

    def __init__(self, input_dim, hidden_dim=8,
                 hidden_activation='Tanh', layer_norm=True):
        super(GlobalNetwork, self).__init__()
        # Setup the qvalue layers
        self.network = make_mlp(input_dim,
                                [hidden_dim, hidden_dim, hidden_dim, 3],
                                hidden_activation=hidden_activation,
                                output_activation=None,
                                layer_norm=layer_norm)

    def forward(self, x, edge_index):

        # We follow here:  https://arxiv.org/pdf/1806.01261.pdf
        #
        # The global output should be the expected score that can
        # be reached from this state and the probability for stopping
        # at this state.
        #
        # Sum up the representations
        # here I have assumed that x is 2D and the each row is
        # representation of an input, so the following operation
        # will reduce the number of rows to 1, but it will keep
        # the tensor as a 2D tensor.
        x = torch.sum(x, dim=0, keepdim=True)

        # compute the output
        return self.network(x)


class MergerNNet(nn.Module):
    def __init__(self, input_dim=3, hidden_dim=64, n_graph_iters=3,
                 hidden_activation='ReLU', layer_norm=True):
        super(MergerNNet, self).__init__()
        self.n_graph_iters = n_graph_iters

        # TODO: input_dim not used. instead inputs hardcoded, but this is error prone.
        # This should be read from the scalers directly

        # Setup the input network
        self.input_network_vxd_hits = make_mlp(8, [hidden_dim],
                                               output_activation=hidden_activation,
                                               layer_norm=layer_norm)

        self.input_network_cdc_hits = make_mlp(5, [hidden_dim],
                                               output_activation=hidden_activation,
                                               layer_norm=layer_norm)

        self.input_network_vxd_tracks = make_mlp(7, [hidden_dim],
                                                 output_activation=hidden_activation,
                                                 layer_norm=layer_norm)

        self.input_network_cdc_tracks = make_mlp(7, [hidden_dim],
                                                 output_activation=hidden_activation,
                                                 layer_norm=layer_norm)

        # Setup the edge network
        self.edge_network = EdgeNetwork(hidden_dim, hidden_dim,
                                        hidden_activation, layer_norm=layer_norm)
        # Setup the node layers
        self.node_network = NodeNetwork(hidden_dim, hidden_dim,
                                        hidden_activation, layer_norm=layer_norm)

        # Setup the edge decoder network
        self.edge_decoder = EdgeNetwork(hidden_dim, hidden_dim,
                                        hidden_activation, layer_norm=layer_norm)

        # Setup the node decoder network
        self.node_decoder = make_mlp(hidden_dim, [hidden_dim, hidden_dim, hidden_dim, 1],
                                     hidden_activation=hidden_activation,
                                     output_activation=None,
                                     layer_norm=layer_norm)

    def forward(self, vxd_hits, vxd_trackids, vxd_tracks, cdc_hits, cdc_trackids, cdc_tracks):
        """Apply forward pass of the model"""

        # Apply input network to get hidden representation for vxd tracks
        vxd_hits = self.input_network_vxd_hits(vxd_hits)
        vxd_tracks = self.input_network_vxd_tracks(vxd_tracks)

        x_vxd = scatter_add(vxd_hits, vxd_trackids, dim=0, dim_size=len(vxd_tracks))
        x_vxd = torch.add(x_vxd, vxd_tracks)

        # Apply input network to get hidden representation for vxd tracks
        cdc_hits = self.input_network_cdc_hits(cdc_hits)
        cdc_tracks = self.input_network_cdc_tracks(cdc_tracks)

        x_cdc = scatter_add(cdc_hits, cdc_trackids, dim=0, dim_size=len(cdc_tracks))
        x_cdc = torch.add(x_cdc, cdc_tracks)

        # Get global vxd+cdc batch of tracks (nodes in graph)
        x = torch.cat((x_vxd, x_cdc), 0)

        # Make graph by connecting each vxd node to each cdc node
        edge_index = torch.zeros(2, 0, dtype=torch.long)
        for i_vxd in range(len(x_vxd)):
            start = torch.zeros(1, len(x_cdc), dtype=torch.long) + i_vxd
            end = torch.arange(0, len(x_cdc), dtype=torch.long) + len(x_vxd)
            end = torch.unsqueeze(end, 0)
            inc = torch.cat((start, end), 0)
            edge_index = torch.cat((edge_index, inc), 1)

        # Loop over message passing layers
        for _ in range(self.n_graph_iters):
            x = self.step_gnn(x, edge_index)

        # Final edge logits
        e = self.edge_decoder(x, edge_index)

        # Final node logits
        a = self.node_decoder(x).squeeze()

        return torch.sigmoid(e), torch.sigmoid(a)

    def step_gnn(self, x, edge_index):
        # Previous hidden state
        x0 = x

        # Apply edge network
        logit_e = self.edge_network(x, edge_index)
        e = torch.sigmoid(logit_e)

        # Apply node network
        x = self.node_network(x, e, edge_index)

        # Residual connection
        x = x + x0

        return x
