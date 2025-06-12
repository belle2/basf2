import numpy as np
import os

import torch
import torch.nn as nn
import torch.nn.functional as F

from torch_geometric.nn import GravNetConv, BatchNorm, global_mean_pool


class CDCNet(nn.Module):
    '''
    Modular CDCNet for Object Condensation.
    '''

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
        '''
        The model: number of input features, hidden dimensions and blocks
        are adjustable

        input_dim:          number of input features per node
        k:                  number of k-nearest neighbours for GravNetConv
        nblocks:            number of blocks the network consists
        coord_dim:          dimension for object condensation coordinates
        space_dimensions:   number of space dimensions for GravNetConv
        momentum:           momentum for batch normalization

        GravNet is from:
        Learning Representations of Irregular Particle-detector Geometry
        with Distance-weighted Graph Networks
        https://arxiv.org/abs/1902.07987
        '''
        super().__init__()

        self.batch_norm_0 = BatchNorm(input_dim, momentum=0.6)

        # First block to start with input dimension
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
        self.p_ccoords_layer = nn.Linear(dim1, coord_dim)  # predict latent space coordinates

        # These are the output layers for the track parameters
        self.p_p_layer = nn.Linear(dim1, 3)  # predict track momentum
        self.p_vertex_layer = nn.Linear(dim1, 3)  # predict track starting point
        self.p_charge_layer = nn.Linear(dim1, 1)  # predict track charge

    def forward(self, x, batch):

        features = []
        x = self.batch_norm_0(x)

        # Global exchange
        out = global_mean_pool(x, batch)
        x = torch.cat([x, out[batch]], dim=-1)

        # Apply Grav Net Blocks
        for i, block in enumerate(self.blocks):
            if i > 0:
                # Do this for every block except input
                out = global_mean_pool(x, batch)
                x = torch.cat([x, out[batch]], dim=-1)
            x = F.elu(block[0](x))
            x = F.elu(block[1](x))
            # First batch normalization
            x = block[2](x)
            x = F.elu(block[3](x))
            # Grav Net block
            x = block[4](x, batch)
            # Second batch normalization
            x = block[5](x)
            # Append output
            features.append(F.elu(block[6](x)))  # skip connections

        # Concatenate features and put through final dense neural network
        x = torch.cat(features, dim=1)
        x = F.elu(self.dense_cat(x))

        # Here are the networks for object condensation predictions
        p_beta = torch.sigmoid(self.p_beta_layer(x))
        p_ccoords = self.p_ccoords_layer(x)

        # Here are the networks for track parameters predictions
        p_p = self.p_p_layer(x)
        p_vertex = self.p_vertex_layer(x)
        p_charge = torch.sigmoid(self.p_charge_layer(x))

        # Concatenate all the predictions and return them
        predictions = torch.cat(
            (
                p_beta,
                p_ccoords,
                p_p,
                p_vertex,
                p_charge,
            ),
            dim=1,
        )
        return predictions


class GNNWrapper:

    def __init__(self, model_path, config, device='cpu'):
        self._device = device
        self._config = config
        self._net = CDCNet(
            input_dim=len(config['dataset']['input_features']),
            k=config['model']['k'],
            nblocks=config['model']['blocks'],
            coord_dim=config['model']['coord_dim'],
            dim1=config['model']['dim1'],
            dim2=config['model']['dim2'],
            space_dimensions=config['model'].get('space_dimensions', 4),
            momentum=config['model'].get('momentum', 0.6),
        ).to(self._device)

        loaded_model = torch.load(model_path, map_location=self._device)

        state_dict = loaded_model['model_state_dict']
        keys = list(state_dict.keys())
        new_state_dict = {}
        for i in range(len(keys)):
            key = keys[i]
            if key.startswith('module.'):
                key = key.replace('module.', '', 1)
            new_state_dict[key] = state_dict[keys[i]]

        self._net.load_state_dict(new_state_dict)
        self._net.eval()

    def predict(self, X):
        print("test")
        with torch.no_grad():
            features = torch.tensor(X, dtype=torch.float32).to(self._device).reshape(-1, 7)
            print(features)
            print("TEST")
            batch = torch.zeros(len(features), dtype=torch.int64).to(self._device)
            predictions = self._net(features, batch).numpy().flatten()
            return predictions


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    return None


def feature_importance(state):
    return []


def begin_fit(state, Xtest, Stest, ytest, wtest, nBatches):
    return None


def partial_fit(state, X, S, y, w, epoch, batch):
    return False


def end_fit(state):
    '''
    Load the trained model and prepare for the basf2 integration.
    '''
    import yaml

    model_path = r'/work/lreuter/CAT_event_generation/models/train_bucket36/run_bucket_36_training_mix_pretrained/best_model.pt'
    config_path = r'/work/lreuter/CAT_event_generation/models/train_bucket36/run_bucket_36_training_mix_pretrained/config.yaml'

    with open(config_path) as f:
        config = yaml.safe_load(f)

    return GNNWrapper(model_path, config)


def load(state):
    '''
    Return the trained model.
    '''
    return state


def apply(state, X):
    '''
    Apply the trained model to the input features.
    '''
    p = state.predict(X)
    return np.require(p, dtype=np.float32, requirements=['A', 'W', 'C', 'O'])


def produce_dummy_file(file_name):
    import ROOT

    tfile = ROOT.TFile(file_name, 'RECREATE')
    ttree = ROOT.TTree('dummy', 'dummy')

    variables = ['a', 'b', 'c']
    data = {var: np.zeros(1, dtype=np.float32) for var in variables}
    for var, arr in data.items():
        ttree.Branch(var, arr, f'{var}/F')
    target = np.zeros(1, dtype=np.int32)
    ttree.Branch('prediction', target, 'prediction/I')

    for _ in range(10):
        for var in variables:
            data[var][0] = 1.
        target[0] = 1
        ttree.Fill()

    ttree.Write()
    tfile.Close()


if __name__ == '__main__':

    import basf2
    import basf2_mva

    # NOTE: do not use testing payloads in production! Any results obtained like this WILL NOT BE PUBLISHED
    basf2.conditions.testing_payloads = ['localdb/database.txt']

    produce_dummy_file('dummy.root')

    # Configuration for fake training
    general_options = basf2_mva.GeneralOptions()
    general_options.m_identifier = 'CATFinderWeightfile'
    # These options here are useless, but we need to make basf2 happy, so...
    general_options.m_datafiles = basf2_mva.vector('dummy.root')
    general_options.m_treename = 'dummy'
    general_options.m_variables = basf2_mva.vector(*['a', 'b', 'c'])
    general_options.m_target_variable = 'prediction'

    python_options = basf2_mva.PythonOptions()
    python_options.m_framework = 'torch'
    python_options.m_steering_file = os.path.abspath(__file__)

    basf2_mva.teacher(general_options, python_options)

    print(f'CATFinder model correctly exported into {general_options.m_identifier} payload')
