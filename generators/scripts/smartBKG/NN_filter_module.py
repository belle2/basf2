##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import os
import numpy as np
import pandas as pd

from collections import defaultdict
import basf2 as b2
from ROOT import Belle2
from ROOT.Belle2 import DBAccessorBase, DBStoreEntry

from smartBKG import TOKENIZE_DICT, PREPROC_CONFIG, MODEL_CONFIG


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


class NNFilterModule(b2.Module):
    """
    Goals:
       1. Build a graph from an event composed of MCParticles
       2. Apply the well-trained model for reweighting or sampling method to get a score
       3. Execute reweighting or sampling process to get a weight

    Arguments:
       model_file(str): Path to saved model
       model_config(dict): Parameters to build the model
       preproc_config(dict): Parameters to provide information for preprocessing
       threshold(float): Threshold for event selection using reweighting method, value *None* indicating sampling mehtod
       extra_info_var(str): Name of eventExtraInfo to save model prediction to
       global_tag(str): Tag in ConditionDB where the well trained model was stored
       payload(str): Payload for the well trained model in global tag

    Returns:
       Pass or rejected according to random sampling or selection with the given threshold

    Note:
        Score after the NN filter indicating the probability of the event to pass is saved
        under ``EventExtraInfo.extra_info_var``.

        Use ``eventExtraInfo(extra_info_var)`` in ``modularAnalysis.variablesToNtuple`` or
        ``additionalBranches=["EventExtraInfo"]`` in ``mdst.add_mdst_output`` to have access to the scores.
    """

    def __init__(
        self,
        model_file=None,
        model_config=MODEL_CONFIG,
        preproc_config=PREPROC_CONFIG,
        threshold=None,
        extra_info_var="NN_prediction",
        global_tag="SmartBKG_GATGAP",
        payload="GATGAPgen.pth"
    ):
        """
        Initialise the class.
        :param model_file:  TODO
        :param model_config:  TODO
        :param preproc_config:  TODO
        :param threshold:  TODO
        :param extra_info_var:  TODO
        :param global_tag:  TODO
        :param payload:  TODO
        """
        super().__init__()
        #: TODO
        self.model_file = model_file
        #: TODO
        self.model_config = model_config
        #: TODO
        self.preproc_config = preproc_config
        #: TODO
        self.threshold = threshold
        #: TODO
        self.extra_info_var = extra_info_var
        #: TODO
        self.payload = payload

        # set additional database conditions for trained neural network
        b2.conditions.prepend_globaltag(global_tag)

    def initialize(self):
        """
        Initialise module before any events are processed
        """
        import torch
        from smartBKG.models.gatgap import GATGAPModel

        DEVICE = torch.device("cpu")

        # read trained model parameters from
        if not self.model_file:
            accessor = DBAccessorBase(DBStoreEntry.c_RawFile, self.payload, True)
            self.model_file = accessor.getFilename()
        trained_parameters = torch.load(self.model_file, map_location=DEVICE)

        #: model with trained parameters
        self.model = GATGAPModel(**self.model_config)
        self.model.load_state_dict(trained_parameters['model_state_dict'])

        #: StoreArray to save weights to
        self.EventExtraInfo = Belle2.PyStoreObj('EventExtraInfo')
        if not self.EventExtraInfo.isValid():
            self.EventExtraInfo.registerInDataStore()

        #: generated variables
        self.gen_vars = defaultdict(list)
        #: node features
        self.out_features = self.preproc_config['features']
        if 'PDG' in self.preproc_config['features']:
            self.out_features.remove('PDG')

    def event(self):
        """
        Collect information from database, build graphs, make predictions and select through sampling or threshold
        """
        import torch
        # Initialize for every event
        self.gen_vars.clear()

        # Need to create the eventExtraInfo entry for each event
        self.EventExtraInfo.create()

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
                    TOKENIZE_DICT[int(mcp.getPDG())]
                )

                # Particle level cutting
                df = pd.DataFrame(self.gen_vars).tail(1)
                df.query(" and ".join(self.preproc_config["cuts"]), inplace=True)
                if df.empty:
                    for values in self.gen_vars.values():
                        values.pop()
                    continue

                # Collect indices for graph
                array_indices.append(mcp.getArrayIndex())
                mother = mcp.getMother()
                if mother:
                    mother_indices.append(mother.getArrayIndex())
                else:
                    mother_indices.append(0)

        graph = self.build_graph(
            array_indices=array_indices, mother_indices=mother_indices,
            PDGs=self.gen_vars['PDG'], Features=[self.gen_vars[key] for key in self.out_features],
            symmetrize=True, add_self_loops=True
        )

        # Output pass probability
        pred = torch.sigmoid(self.model(graph)).detach().numpy().squeeze()

        # Save the pass probability to EventExtraInfo
        self.EventExtraInfo.addExtraInfo(self.extra_info_var, pred)

        # Module returns bool of whether prediciton passes threshold for use in basf2 path flow control
        if self.threshold:
            self.return_value(int(pred >= self.threshold))
        else:
            self.return_value(int(pred >= np.random.rand()))

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
        import torch
        import dgl
        os.environ["DGLBACKEND"] = "pytorch"

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
