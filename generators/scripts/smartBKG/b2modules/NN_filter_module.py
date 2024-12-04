##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import numpy as np
import torch

import basf2 as b2
from ROOT import Belle2
from ROOT.Belle2 import DBAccessorBase, DBStoreEntry

from smartBKG import PREPROC_CONFIG, MODEL_CONFIG
from smartBKG.utils.preprocess import load_particle_list, preprocessed
from smartBKG.models.gatgap import GATGAPModel
from smartBKG.utils.dataset import ArrayDataset

DEVICE = torch.device("cpu")


class NNFilterModule(b2.Module):
    """
    Goals:
       1. Build a graph from an event composed of MCParticles
       2. Apply the well-trained model for reweighting or sampling method to get a score
       3. Execute reweighting or sampling process to get a weight

    Arguments:
       model_file(str): Path to the saved model
       model_config(dict): Parameters to build the model
       preproc_config(dict): Parameters for preprocessing
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
        :param model_file: Path to the saved model file.
        :param model_config: Parameters for building the model.
        :param preproc_config: Parameters for preprocessing.
        :param threshold: Threshold for event selection using reweighting method, value *None* indicating sampling mehtod.
        :param extra_info_var: Name of eventExtraInfo to save model prediction to.
        :param global_tag: Tag in ConditionDB where the well-trained model was stored.
        :param payload: Payload for the well-trained model in global tag.
        """
        super().__init__()
        #: Path to the saved model file
        self.model_file = model_file
        #: Parameters for building the model
        self.model_config = model_config
        #: Parameters for preprocessing
        self.preproc_config = preproc_config
        #: Threshold for event selection using reweighting method, value *None*  indicating sampling method
        self.threshold = threshold
        #: Name of eventExtraInfo to save model prediction to
        self.extra_info_var = extra_info_var
        #: Payload for the well-trained model in global tag
        self.payload = payload

        # set additional database conditions for trained neural network
        b2.conditions.prepend_globaltag(global_tag)

    def initialize(self):
        """
        Initialise module before any events are processed
        """
        # read trained model parameters from database
        if not self.model_file:
            accessor = DBAccessorBase(DBStoreEntry.c_RawFile, self.payload, True)
            self.model_file = accessor.getFilename()
        trained_parameters = torch.load(self.model_file, map_location=DEVICE)

        #: model with trained parameters
        self.model = GATGAPModel(**self.model_config)
        self.model.load_state_dict(trained_parameters['model_state_dict'])

        #: StoreArray to save weights to
        self.EventExtraInfo = Belle2.PyStoreObj('EventExtraInfo')
        self.EventExtraInfo.isRequired()
        #: Initialise event metadata from data store
        self.EventInfo = Belle2.PyStoreObj('EventMetaData')
        #: node features
        self.out_features = self.preproc_config['features']
        if 'PDG' in self.preproc_config['features']:
            self.out_features.remove('PDG')

    def event(self):
        """
        Collect information from database, build graphs, make predictions and select through sampling or threshold
        """
        # Need to create the eventExtraInfo entry for each event
        if not self.EventExtraInfo.isValid():
            self.EventExtraInfo.create()
        df_dict = load_particle_list(mcplist=Belle2.PyStoreArray("MCParticles"), evtNum=self.EventInfo.getEvent(), label=True)
        single_input = preprocessed(df_dict, particle_selection=self.preproc_config['cuts'])
        graph = ArrayDataset(single_input, batch_size=1)[0][0]
        # Output pass probability
        pred = torch.sigmoid(self.model(graph)).detach().numpy().squeeze()

        # Save the pass probability to EventExtraInfo
        self.EventExtraInfo.addExtraInfo(self.extra_info_var, pred)

        # Module returns bool of whether prediciton passes threshold for use in basf2 path flow control
        if self.threshold:
            self.return_value(int(pred >= self.threshold))
        else:
            self.return_value(int(pred >= np.random.rand()))
