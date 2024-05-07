##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import torch
import torch.nn.functional as F
import dgl
import dgl.nn.pytorch as dglnn
from dgl.nn.pytorch.glob import GlobalAttentionPooling

from smartBKG import TOKENIZE_DICT

NUM_PDG = len(TOKENIZE_DICT)


class GATModule(torch.nn.Module):
    """
    Apply a GAT layer to node features, flatten outputs of attention heads
    and update global features.
    """

    def __init__(
        self,
        in_feats,
        units,
        num_heads,
        in_feats_glob,
        use_gap=False,
    ):
        """
        Initialise the class.

        :param in_feats: Number of features for each node.
        :param units: Number of output units for the GAT layer.
        :param num_heads: Number of attention heads in the GAT layer.
        :param in_feats_glob: Current dimension of global features. Initialized as 0.
        :param use_gap: Whether to use Global Attention Pooling (GAP) for the production of global features.
        """
        super().__init__()
        #: GAT layer to update node features
        self.gat = dglnn.GATConv(in_feats, units, num_heads)
        out_feats = units * num_heads
        #: Fully connected layer for feature aggregation to update global features
        self.fc = torch.nn.Linear(in_feats_glob + out_feats, units)
        #: Whether to use Global Attention Pooling (GAP) for the production of global features
        self.use_gap = use_gap
        if self.use_gap:
            #: Linear gate to produce global features
            self.gap_gate = torch.nn.Linear(out_feats, 1)
            #: Global Attention Pooling layer to produce global features
            self.gap = GlobalAttentionPooling(self.gap_gate)

    def forward(self, graph, feat, feat_glob=None):
        """
        Forward pass of the GAT module.

        Arguments:
            graph (torch.Tensor): DGLGraph representing the decay tree.
            feat (torch.Tensor): Node feataures attached to the graph.
            feat_glob (torch.Tensor): Global features from previous layers.
            `None` for initialized as the global average or attention pooling of the whole graph.

        Returns:
            torch.Tensor: updated node features.
            torch.Tensor: updated global features.
        """
        h = F.leaky_relu(self.gat(graph, feat)).flatten(1)
        hg = feat_glob
        if not self.use_gap:
            with graph.local_scope():
                graph.ndata['h'] = h
                hmean = dgl.mean_nodes(graph, 'h')
        else:
            hmean = self.gap(graph, h)
        if hg is None:
            hg = hmean
        else:
            # Concatenate previous global features with new aggregation
            hg = torch.cat((hg, hmean), axis=1)
        # Update global features
        hg = F.leaky_relu(self.fc(hg))
        return h, hg


class GATGAPModel(torch.nn.Module):
    """
    Input:
       dgl graph built from decay event

    Arguments:
       units(int): Number of units for the output dimension of GAT Convolutional layers
       as well as the dimension of global features
       num_features(int): Number of features attached to each node or particle as NN input
       num_pdg(int): Number of all possible PDG IDs
       emb_size(int): Dimension of embedded PDG space
       attention_heads(int): Number of attention heads for GAT Convolutional layers
       n_layers(int): Number of GAT Convolutional layers
       use_gap(bool): Whether to use Global Attention Pooling (GAP) for the production of global features

    Returns:
       logits(float): Indicating the probability of an event being able to pass the
       corresponding skim, need `sigmoid` to be used as a prediction
    """

    def __init__(
        self,
        units=128,
        num_features=8,
        num_pdg=NUM_PDG,
        emb_size=8,
        attention_heads=4,
        n_layers=5,
        use_gap=False
    ):
        """
        Initialise the class.

        :param units: Number of units for the output dimension of GAT Convolutional layers
        as well as the dimension of global features.
        :param num_features: Number of features attached to each node or particle as NN input.
        :param num_pdg: Number of all possible PDG IDs.
        :param emb_size: Dimension of embedded PDG space.
        :param attention_heads: Number of attention heads for GAT Convolutional layers.
        :param n_layers: Number of GAT Convolutional layers.
        :param use_gap: Whether to use Global Attention Pooling (GAP) for the production of global features.
        """
        super().__init__()
        #: Embedding layer for PDG IDs
        self.pdg_embedding = torch.nn.Embedding(num_pdg + 1, emb_size)
        in_feats = num_features + emb_size
        #: List of GAT modules to update node features
        self.gat_layers = torch.nn.ModuleList()
        in_feats_glob = 0
        for i in range(n_layers):
            self.gat_layers.append(
                GATModule(
                    in_feats=in_feats,
                    units=units,
                    num_heads=attention_heads,
                    in_feats_glob=in_feats_glob,
                    use_gap=use_gap
                )
            )
            in_feats = units * attention_heads
            in_feats_glob = units

        #: Output layer for final prediction
        self.fc_output = torch.nn.Linear(units, 1)

    def forward(self, graph):
        """
        Forward pass of the GATGAPModel.

        Arguments:
            graph (torch.Tensor): DGLGraph representing the decay tree.

        Returns:
            torch.Tensor: the final prediction with size 1.
        """
        h_pdg = graph.ndata["x_pdg"]
        h_feat = graph.ndata["x_feature"]
        h_pdg = self.pdg_embedding(h_pdg.long())
        h = torch.cat((h_pdg, h_feat), axis=1)
        hg = None
        for layer in self.gat_layers:
            h, hg = layer(graph, h, hg)
        return self.fc_output(hg)
