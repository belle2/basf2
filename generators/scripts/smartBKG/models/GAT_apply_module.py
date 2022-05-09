#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Make sure to have basf2 available before open jupyter/python or call
# this module. In terminal: "source /cvmfs/belle.cern.ch/tools/b2setup
# release-xx-xx-xx". Current release-06-00-03
import os
import numpy as np

import pandas as pd
from collections import defaultdict
import dgl
import torch
import basf2 as b2
from ROOT import Belle2
from ROOT.Belle2 import DBAccessorBase, DBStoreEntry

from smartBKG import tokenize_dict, preproc_config
from smartBKG.models.gatgap import SimpleGATModel

os.environ["DGLBACKEND"] = "pytorch"
device = torch.device("cpu")
# print(torch.cuda.is_available())

model_config = {
    "units": 32, "attention_heads": 4, "n_layers": 6, "use_gap": True
}


def check_status_bit(status_bit):
    """
    Returns True if conditions are satisfied (not an unusable particle)
    """
    return (
        (status_bit & 1 << 4 == 0) &  # IsVirtual
        (status_bit & 1 << 5 == 0) &  # Initial
        (status_bit & 1 << 6 == 0) &  # ISRPhoton
        (status_bit & 1 << 7 == 0)  # FSRPhoton
    )


class GATApplyModule(b2.Module):
    """
    Goals:
       1. Build a graph from an event composed of MCParticles
       2. Apply the well-trained model for reweighting or sampling method to get a score
       (3. Execute reweighting or sampling process to get a weight)

    Arguments:
       model_file(str): Path to saved model
       threshold: (float) Threshold for event selection using reweighting method, or
                  (None) Indicating sampling mehtod
       extra_info_var(str): Name of eventExtraInfo to save model prediction to
       custom_objects(str): Dictionary of custom objects to load with saved model (needed for custom layers)

    Returns:
       score(float): Score after the NN filter, indicating the probability of the event to pass.
       (label: Pass or rejected according to random sampling or selection with threshold)

    Setting the extra_info_var saves the output prediction to EventExtraInfo,
    users then need to explicitly add this branch to the mdst output to save this.
    """

    def __init__(
        self,
        model_file=None,
        model_config=model_config,
        preproc_config=preproc_config,
        threshold=None,
        extra_info_var="NN_prediction"
    ):
        super().__init__()
        self.model_file = model_file
        self.model_config = model_config
        self.preproc_config = preproc_config
        self.threshold = threshold
        self.extra_info_var = extra_info_var

        # set additional database conditions for trained neural network
        b2.conditions.prepend_globaltag("SmartBKG_GATGAP")

    def initialize(self):
        """
        Initialise module before any events are processed
        """
        # read trained model parameters from
        if not self.model_file:
            payload = "GATGAPgen.pth"
            accessor = DBAccessorBase(DBStoreEntry.c_RawFile, payload, True)
            self.model_file = accessor.getFilename()
        trained_parameters = torch.load(self.model_file, map_location=device)

        # build model with trained parameters
        self.model = SimpleGATModel(**model_config)
        self.model.load_state_dict(trained_parameters['model_state_dict'])

        # Create a StoreArray to save weights to
        self.e_e_info = Belle2.PyStoreObj('EventExtraInfo')
        self.e_e_info.registerInDataStore()

        # Store generated variables as node features in a graph
        self.gen_vars = defaultdict(list)
        self.out_features = preproc_config['features']
        if 'PDG' in preproc_config['features']:
            self.out_features.remove('PDG')

    def event(self):
        """
        Return match of event number to input list
        """
        # Initialize for every event
        self.gen_vars = defaultdict(list)

        # Need to create the eventExtraInfo entry for each event
        self.e_e_info.create()

        mcplist = Belle2.PyStoreArray("MCParticles")

        array_indices = []
        mother_indices = []

        for i, mcp in enumerate(mcplist):
            if mcp.isPrimaryParticle():
                # Check mc particle is useable
                if not check_status_bit(mcp.getStatus()):
                    continue

                prodTime = mcp.getProductionTime()
                # record the production time of root particle for the correction of jitter
                if i == 0:
                    root_prodTime = prodTime
                prodTime -= root_prodTime

                four_vec = mcp.get4Vector()
                prod_vec = mcp.getProductionVertex()

                # build generated variables as node features
                self.gen_vars['prodTime'].append(prodTime)
                self.gen_vars['energy'].append(mcp.getEnergy())
                self.gen_vars['x'].append(prod_vec.x())
                self.gen_vars['y'].append(prod_vec.y())
                self.gen_vars['z'].append(prod_vec.z())
                self.gen_vars['px'].append(four_vec.Px())
                self.gen_vars['py'].append(four_vec.Py())
                self.gen_vars['pz'].append(four_vec.Pz())
                self.gen_vars['PDG'].append(
                    tokenize_dict[int(mcp.getPDG())]
                )

                # Particle level cutting
                df = pd.DataFrame(self.gen_vars).tail(1)
                df.query(" and ".join(preproc_config["cuts"]), inplace=True)
                if df.empty:
                    [self.gen_vars[key].pop() for key in self.gen_vars.keys()]
                    continue

                # Collect indices for graph
                array_indices.append(mcp.getArrayIndex())
                mother = mcp.getMother()
                if mother:
                    mother_indices.append(mother.getArrayIndex())
                else:
                    mother_indices.append(0)

        # Check the whole event
        if len(self.gen_vars['PDG']) == 0:
            print("Interpreted as pure fail dataframe.")
            self.endRun()

        graph = self.build_graph(
            array_indices=array_indices, mother_indices=mother_indices,
            PDGs=self.gen_vars['PDG'], Features=[self.gen_vars[key] for key in self.out_features],
            symmetrize=True, add_self_loops=True
        )

        # Output pass probability
        pred = torch.sigmoid(self.model(graph)).detach().numpy().squeeze()

        # Save the pass probability to EventExtraInfo
        self.e_e_info.addExtraInfo(self.extra_info_var, pred)

        # Module returns bool of whether prediciton passes threshold for use in basf2 path flow control
        if self.threshold:
            self.return_value(int(pred >= self.threshold))
        else:
            self.return_value(int(pred >= np.random.rand()))

    def terminate(self):
        # del self.model
        # print(self.e_e_info)
        print('GATModule finished')

    def mapped_mother_indices(self, array_indices, mother_indices):
        """
        Map the mother indices to an enumerated list. The one-hot encoded version
        of that list then corresponds to the adjacency matrix.

        Example:
           >>> mapped_mother_indices(
           ...    [0, 1, 3, 5, 6, 7, 8, 9, 10],
           ...    [0, 0, 0, 1, 1, 1, 5, 5, 7]
           ... )
           [0, 0, 0, 1, 1, 1, 3, 3, 5]

        Args:
           array_indices: list or array of indices. Each index has to be unique.
           mother_indices: list or array of mother indices.

        Returns:
           List of mapped indices
        """
        idx_dict = {v: i for i, v in enumerate(array_indices)}
        return [idx_dict[m] for m in mother_indices]

    def build_graph(self, array_indices, mother_indices, PDGs, Features,
                    symmetrize=True, add_self_loops=True):
        """
        Build graph from preprocessed particle information
        """
        # Build adjacency mapping
        adjacency = self.mapped_mother_indices(array_indices, mother_indices)

        # Build graph
        src = adjacency
        dst = np.arange(len(src))
        src_new, dst_new = src, dst
        if symmetrize:
            src_new, dst_new = (
                np.concatenate([src, dst]),
                np.concatenate([dst, src])
            )
        # remove self-loops (the Y(4S)) to avoid duplicated self loops
        src_new, dst_new = map(
            np.array, zip(*[(s, d) for s, d in zip(src_new, dst_new) if not s == d])
        )
        if add_self_loops:
            src_new, dst_new = (
                np.concatenate([src_new, dst]),
                np.concatenate([dst_new, dst])
            )
        graph = dgl.graph((src_new, dst_new))
        graph.ndata["x_pdg"] = torch.tensor(PDGs, dtype=torch.int32)
        graph.ndata["x_feature"] = torch.tensor(np.transpose(Features), dtype=torch.float32)

        return graph
