import torch
import torch.nn.functional as F
import dgl
import dgl.nn.pytorch as dglnn
from dgl.nn.pytorch.glob import GlobalAttentionPooling
from smartBKG import tokenize_dict

NUM_PDG = len(tokenize_dict)


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
        super().__init__()
        self.gat = dglnn.GATConv(in_feats, units, num_heads)
        out_feats = units * num_heads
        self.fc = torch.nn.Linear(in_feats_glob + out_feats, units)
        self.use_gap = use_gap
        if self.use_gap:
            self.gap_gate = torch.nn.Linear(out_feats, 1)
            self.gap = GlobalAttentionPooling(self.gap_gate)

    def forward(self, graph, feat, feat_glob=None):
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
            # concatenate previous state with new aggregation
            hg = torch.cat((hg, hmean), axis=1)
        # update global state
        hg = F.leaky_relu(self.fc(hg))
        return h, hg


class SimpleGATModel(torch.nn.Module):
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
        super().__init__()
        self.pdg_embedding = torch.nn.Embedding(num_pdg + 1, emb_size)
        in_feats = num_features + emb_size
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

        self.fc_output = torch.nn.Linear(units, 1)

    def forward(self, graph):
        h_pdg = graph.ndata["x_pdg"]
        h_feat = graph.ndata["x_feature"]
        h_pdg = self.pdg_embedding(h_pdg.long())
        h = torch.cat((h_pdg, h_feat), axis=1)
        hg = None
        for layer in self.gat_layers:
            h, hg = layer(graph, h, hg)
        return self.fc_output(hg)


def get_embedded_feats(model, graph):
    """
    Return features that go into graph network (concatenation of pdg
    embeddings and other node features)

    Note: Assumes the model has the pdg embedding layer as a member
    named `pdg_embedding`
    """
    g = graph
    h_pdg = g.ndata["x_pdg"]
    h_feat = g.ndata["x_feature"]
    h_pdg = model.pdg_embedding(h_pdg.long())
    return torch.cat((h_pdg, h_feat), axis=1)


def get_edge_attention(graph_layer, graph, feat):
    """
    Return the edge attention weights of GAT layers
    """
    import dgl.function as fn
    # copied relevant parts from source code
    # https://docs.dgl.ai/_modules/dgl/nn/pytorch/conv/gatconv.html#GATConv
    self = graph_layer
    with graph.local_scope():
        from dgl.ops import edge_softmax
        h_src = self.feat_drop(feat)
        feat_src = feat_dst = self.fc(h_src).view(-1, self._num_heads, self._out_feats)
        el = (feat_src * self.attn_l).sum(dim=-1).unsqueeze(-1)
        er = (feat_dst * self.attn_r).sum(dim=-1).unsqueeze(-1)
        graph.srcdata.update({'ft': feat_src, 'el': el})
        graph.dstdata.update({'er': er})
        # compute edge attention, el and er are a_l Wh_i and a_r Wh_j respectively.
        graph.apply_edges(fn.u_add_v('el', 'er', 'e'))
        e = self.leaky_relu(graph.edata.pop('e'))
        # compute softmax
        return self.attn_drop(edge_softmax(graph, e))
