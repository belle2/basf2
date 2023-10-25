#!/usr/bin/env python
# -*- coding: utf-8 -*-

import itertools
import numpy as np
import yaml
import warnings
import basf2 as b2
from ROOT import Belle2
from variables import variables as vm
import torch
from grafei.scripts.FlagBDecayModule import getObjectList
from grafei.model.geometric_network import GeometricNetwork
from grafei.model.normalize_features import normalize_features
from grafei.model.edge_features import compute_edge_features
from grafei.model.lca2adjacency import lca2adjacency, InvalidLCAMatrix, n_intermediate_particles
from grafei.model.tree_utils import masses_to_classes
# from grafei.model.tree_utils import node_masses
from grafei.scripts.RootSaverModule import write_hist

warnings.filterwarnings(
    action="ignore", category=RuntimeWarning, message="Mean of empty slice.*"
)


class Batch:
    """Helper to pass argument to geometric model"""

    def __init__(self, x=None, edge_attr=None, edge_index=None, u=None, batch=None):
        self.x = x
        self.edge_attr = edge_attr
        self.edge_index = edge_index
        self.u = u
        self.batch = batch


class graFEISaverModule(b2.Module):
    """Apply graFEI model to some given particle lists in basf2"""

    def __init__(
        self,
        particle_list: str,
        cfg_path=None,
        param_file=None,
        store_true_info=False,
        gpu=False,
    ):
        """Class Constructor.

        Args:
            particle_list (list): Name of particle list to run graFEI on
            cfg_path (str): path to config file
            param_file (str): path to file containing weight files for the model
        """
        super().__init__()
        self.particle_list = particle_list
        self.cfg_path = cfg_path
        self.param_file = param_file
        self.storeTrueInfo = store_true_info
        self.gpu = gpu

        assert self.storeTrueInfo in [False, "B0:MC", "B+:MC", "Upsilon(4S):MC"], \
            "You should select one of B0, B+ or Upsilon(4S) for truth-matching, or choose not to store truth-matching info"

    def initialize(self):
        # Get weights and configs from the DB if they are not provided from the user
        if not self.cfg_path:
            config = Belle2.DBAccessorBase(
                Belle2.DBStoreEntry.c_RawFile, "graFEIConfigFile", True
            )
            self.cfg_path = config.getFilename()
        if not self.param_file:
            model = Belle2.DBAccessorBase(
                Belle2.DBStoreEntry.c_RawFile, "graFEIModelFile", True
            )
            self.param_file = model.getFilename()

        # Figure out which device all this is running on
        self.device = torch.device(
            "cuda" if (self.gpu and torch.cuda.is_available()) else "cpu"
        )

        # Load configs
        cfg_file = open(self.cfg_path, "r")
        self.configs = yaml.safe_load(cfg_file)

        # Normalize features
        self.normalize = self.configs["dataset"]["config"]["normalize"]

        # Mixed precision
        self.use_amp = self.configs["train"][
            "mixed_precision"
        ] and self.device == torch.device("cuda")

        # Get features
        self.node_features = self.configs["dataset"]["config"]["features"]
        self.edge_features = self.configs["dataset"]["config"]["edge_features"]
        self.glob_features = self.configs["dataset"]["config"]["global_features"]

        # Set name convention
        self.node_features = [f"feat_{name}" for name in self.node_features] if self.node_features else []
        self.edge_features = [f"edge_{name}" for name in self.edge_features] if self.edge_features else []
        self.glob_features = [f"glob_{name}" for name in self.glob_features] if self.glob_features else []
        self.discarded_features = ["feat_x", "feat_y", "feat_z", "feat_px", "feat_py", "feat_p"]

        # Extract the number of features
        n_infeatures = len(self.node_features)
        e_infeatures = len(self.edge_features)
        g_infeatures = len(self.glob_features)

        # Build the model
        self.model = GeometricNetwork(
            nfeat_in_dim=n_infeatures,
            efeat_in_dim=e_infeatures,
            gfeat_in_dim=g_infeatures,
            edge_classes=self.configs["dataset"]["edge_classes"],
            x_classes=self.configs["dataset"]["x_classes"],
            **self.configs["geometric_model"],
        )

        # Load paramaters' values
        self.model.load_state_dict(
            torch.load(self.param_file, map_location=self.device)["model"]
        )

        # Activate evaluation mode
        self.model.eval()
        # Push model to GPU in case
        self.model.to(self.device)

        # Print stuff
        print()
        print("**************************************************")
        print("**************************************************")
        print("*                                                *")
        print("*                     graFEI                     *")
        print("*                                                *")
        print("**************************************************")
        print("**************************************************")
        print()

        b2.B2DEBUG(10, "Model structure:\n", {self.model})

    def terminate(self):
        """Called once after all the processing is complete"""
        pass

    def event(self):
        b2.B2DEBUG(10, "---- Processing new event ----")

        # Get the B candidate list
        candidate_list = getObjectList(Belle2.PyStoreObj(self.particle_list).obj())

        # Get the particle candidate(s)
        for candidate in candidate_list:
            # Get FSPs
            p_list = getObjectList(candidate.getFinalStateDaughters())

            # Get particle nature
            photons = np.array([(abs(p.getPDGCode()) == 22) for p in p_list])
            electrons = np.array([(abs(p.getPDGCode()) == 11) for p in p_list])
            muons = np.array([(abs(p.getPDGCode()) == 13) for p in p_list])
            pions = np.array([(abs(p.getPDGCode()) == 211) for p in p_list])
            kaons = np.array([(abs(p.getPDGCode()) == 321) for p in p_list])
            protons = np.array([(abs(p.getPDGCode()) == 2212) for p in p_list])

            # Number of FSPs, charged and photons
            n_nodes = len(p_list)

            graFEI_nFSP = n_nodes
            graFEI_nPhotons_preFit = sum(photons)
            graFEI_nCharged_preFit = graFEI_nFSP - graFEI_nPhotons_preFit
            graFEI_nElectrons_preFit = sum(electrons)
            graFEI_nMuons_preFit = sum(muons)
            graFEI_nPions_preFit = sum(pions)
            graFEI_nKaons_preFit = sum(kaons)
            graFEI_nProtons_preFit = sum(protons)
            graFEI_nLeptons_preFit = graFEI_nElectrons_preFit + graFEI_nMuons_preFit
            graFEI_nOthers_preFit = graFEI_nCharged_preFit - \
                (graFEI_nLeptons_preFit + graFEI_nPions_preFit + graFEI_nKaons_preFit + graFEI_nProtons_preFit)

            # Trivial decay tree
            if n_nodes < 2:
                b2.B2WARNING(
                    f"Skipping candidate with {n_nodes} reconstructed FSPs"
                )

                candidate.addExtraInfo("graFEI_probEdgeProd", np.nan)
                candidate.addExtraInfo("graFEI_probEdgeMean", np.nan)
                candidate.addExtraInfo("graFEI_probEdgeGeom", np.nan)
                candidate.addExtraInfo("graFEI_validTree", np.nan)
                candidate.addExtraInfo("graFEI_6inLCAS", np.nan)
                candidate.addExtraInfo("graFEI_5inLCAS", np.nan)
                candidate.addExtraInfo("graFEI_4inLCAS", np.nan)
                candidate.addExtraInfo("graFEI_3inLCAS", np.nan)
                candidate.addExtraInfo("graFEI_2inLCAS", np.nan)
                candidate.addExtraInfo("graFEI_1inLCAS", np.nan)
                candidate.addExtraInfo("graFEI_nIntermediates", np.nan)
                candidate.addExtraInfo("graFEI_nFSP", graFEI_nFSP)
                candidate.addExtraInfo("graFEI_nCharged_preFit", graFEI_nCharged_preFit)
                candidate.addExtraInfo("graFEI_nPhotons_preFit", graFEI_nPhotons_preFit)
                candidate.addExtraInfo("graFEI_nElectrons_preFit", graFEI_nElectrons_preFit)
                candidate.addExtraInfo("graFEI_nMuons_preFit", graFEI_nMuons_preFit)
                candidate.addExtraInfo("graFEI_nPions_preFit", graFEI_nPions_preFit)
                candidate.addExtraInfo("graFEI_nKaons_preFit", graFEI_nKaons_preFit)
                candidate.addExtraInfo("graFEI_nProtons_preFit", graFEI_nProtons_preFit)
                candidate.addExtraInfo("graFEI_nLeptons_preFit", graFEI_nLeptons_preFit)
                candidate.addExtraInfo("graFEI_nOthers_preFit", graFEI_nOthers_preFit)
                candidate.addExtraInfo("graFEI_nPhotons_postFit", np.nan)
                candidate.addExtraInfo("graFEI_nCharged_postFit", np.nan)
                candidate.addExtraInfo("graFEI_nElectrons_postFit", np.nan)
                candidate.addExtraInfo("graFEI_nMuons_postFit", np.nan)
                candidate.addExtraInfo("graFEI_nPions_postFit", np.nan)
                candidate.addExtraInfo("graFEI_nKaons_postFit", np.nan)
                candidate.addExtraInfo("graFEI_nProtons_postFit", np.nan)
                candidate.addExtraInfo("graFEI_nLeptons_postFit", np.nan)
                candidate.addExtraInfo("graFEI_nOthers_postFit", np.nan)
                candidate.addExtraInfo("graFEI_nPredictedUnmatched", np.nan)
                candidate.addExtraInfo("graFEI_nPredictedUnmatched_noPhotons", np.nan)
                candidate.addExtraInfo("graFEI_depthLCA", np.nan)
                candidate.addExtraInfo("graFEI_nBtag_daughters", np.nan)
                candidate.addExtraInfo("graFEI_nBsig_daughters", np.nan)
                candidate.addExtraInfo("graFEI_notB_daughters", np.nan)

                # if self.storeTrueInfo and Belle2.Environment.Instance().isMC():
                if self.storeTrueInfo:
                    candidate.addExtraInfo("graFEI_truth_perfectLCA", np.nan)
                    candidate.addExtraInfo("graFEI_truth_perfectMasses", np.nan)
                    candidate.addExtraInfo("graFEI_truth_perfectEvent", np.nan)
                    candidate.addExtraInfo("graFEI_truth_isSemileptonic", np.nan)
                    candidate.addExtraInfo("graFEI_truth_nFSP", np.nan)
                    candidate.addExtraInfo("graFEI_truth_depthLCA", np.nan)
                    candidate.addExtraInfo("graFEI_truth_nPhotons", np.nan)
                    candidate.addExtraInfo("graFEI_truth_nElectrons", np.nan)
                    candidate.addExtraInfo("graFEI_truth_nMuons", np.nan)
                    candidate.addExtraInfo("graFEI_truth_nPions", np.nan)
                    candidate.addExtraInfo("graFEI_truth_nKaons", np.nan)
                    candidate.addExtraInfo("graFEI_truth_nProtons", np.nan)
                    candidate.addExtraInfo("graFEI_truth_nOthers", np.nan)

                return

            # Initialize node features array
            x_nodes = np.empty((n_nodes, len(self.node_features)))
            x_dis = np.empty((n_nodes, len(self.discarded_features)))

            # Fill node features array
            for p, particle in enumerate(p_list):
                for f, feat in enumerate(self.node_features):
                    feat = feat[feat.find("feat_") + 5:]
                    x_nodes[p, f] = vm.evaluate(feat, particle)
                for f, feat in enumerate(self.discarded_features):
                    feat = feat[feat.find("feat_") + 5:]
                    x_dis[p, f] = vm.evaluate(feat, particle)
            b2.B2DEBUG(11, "Node features:\n", x_nodes)

            # Fill edge features array
            x_edges = (compute_edge_features(self.edge_features, self.node_features + self.discarded_features,
                                             np.concatenate([x_nodes, x_dis], axis=1)) if self.edge_features != [] else [])
            edge_index = torch.tensor(list(itertools.permutations(range(n_nodes), 2)), dtype=torch.long)
            b2.B2DEBUG(11, "Edge features:\n", x_edges)

            # Fill global features # TODO: get them from basf2
            x_global = (
                np.array([[n_nodes]], dtype=float)
                if self.glob_features != []
                else []
            )
            b2.B2DEBUG(11, "Global features:\n", x_global)

            # Fill tensor to assign each node to a graph (trivial since we have only one graph per decay)
            torch_batch = torch.zeros(size=[n_nodes], dtype=torch.long)

            # Set nans to zero, this is a surrogate value, may change in future
            np.nan_to_num(x_nodes, copy=False)
            np.nan_to_num(x_edges, copy=False)
            np.nan_to_num(x_global, copy=False)

            # Normalize any features that should be
            if self.normalize is not None:
                normalize_features(
                    self.normalize,
                    self.node_features,
                    x_nodes,
                    self.edge_features,
                    x_edges,
                    self.glob_features,
                    x_global,
                )

            # Convert everything to torch tensors and/or send to some device in case
            x = torch.tensor(x_nodes, dtype=torch.float).to(self.device)
            edge_index = edge_index.t().contiguous().to(self.device)
            edge_attr = torch.tensor(x_edges, dtype=torch.float).to(self.device)
            u = torch.tensor(x_global, dtype=torch.float).to(self.device)
            torch_batch = torch_batch.to(self.device)

            # Create object to be passed to model
            batch = Batch(
                x=x, edge_index=edge_index, edge_attr=edge_attr, u=u, batch=torch_batch
            )

            # Evaluate model
            with torch.no_grad():
                x_pred, e_pred, u_pred = self.model(batch)
                # if self.use_amp:
                #    with autocast(enabled=True):
                #        x_pred, e_pred, u_pred = self.model(batch)
                # else:
                #    x_pred, e_pred, u_pred = self.model(batch)

            # Select edges from predictions
            edge_probs = torch.softmax(e_pred, dim=1)
            edge_probability, predicted_LCA = edge_probs.max(dim=1)

            # Select masses from predictions
            mass_probs = torch.softmax(x_pred, dim=1)
            mass_probability, predicted_masses = mass_probs.max(dim=1)
            b2.B2DEBUG(10, "Predicted mass classes:\n", predicted_masses)
            b2.B2DEBUG(11, "Mass class probabilities:\n", mass_probability)

            # Count number of predicted particles for each mass hypothesis
            graFEI_nPhotons_postFit = (predicted_masses == 6).sum()
            graFEI_nCharged_postFit = graFEI_nFSP - graFEI_nPhotons_postFit
            graFEI_nElectrons_postFit = (predicted_masses == 1).sum()
            graFEI_nMuons_postFit = (predicted_masses == 2).sum()
            graFEI_nPions_postFit = (predicted_masses == 3).sum()
            graFEI_nKaons_postFit = (predicted_masses == 4).sum()
            graFEI_nProtons_postFit = (predicted_masses == 5).sum()
            graFEI_nLeptons_postFit = graFEI_nElectrons_postFit + graFEI_nMuons_postFit
            graFEI_nOthers_postFit = (predicted_masses == 0).sum()

            # Get square matrices
            edge_probability_square = torch.sparse_coo_tensor(
                edge_index, edge_probability
            ).to_dense()
            predicted_LCA_square = torch.sparse_coo_tensor(
                edge_index, predicted_LCA, dtype=int
            ).to_dense()
            b2.B2DEBUG(10, "Predicted LCA:\n", predicted_LCA_square)
            b2.B2DEBUG(11, "Edge class probabilities:\n", edge_probability_square)

            # Remove symmetric elements from probability
            edge_probability_unique = edge_probability_square[
                edge_probability_square.tril(diagonal=-1) > 0
            ]

            # Get particles predicted as matched by the model
            predicted_matched = np.array(
                [False if torch.all(i == 0) else True for i in predicted_LCA_square]
            )
            b2.B2DEBUG(10, "Predicted matched particles:\n", predicted_matched)
            # Same but ignoring photons
            predicted_matched_noPhotons = predicted_matched[~photons]

            # Get number of predicted as unmatched
            graFEI_nPredictedUnmatched = (~predicted_matched).sum()
            graFEI_nPredictedUnmatched_noPhotons = (
                (~predicted_matched_noPhotons).sum()
                if predicted_matched_noPhotons.size != 0
                else 0
            )

            # Particle list for matched
            matched_list = [j for i, j in enumerate(p_list) if predicted_matched[i]]

            # Get LCA of predicted matched only
            predicted_LCA_square_matched = predicted_LCA_square[predicted_matched]
            predicted_LCA_square_matched = predicted_LCA_square_matched[:, predicted_matched]

            # Check if LCA describes a tree graph
            graFEI_validTree = 0
            if not torch.all(predicted_LCA_square == 0):
                try:
                    adjacency = lca2adjacency(predicted_LCA_square_matched)
                    graFEI_validTree = 1
                except InvalidLCAMatrix:
                    pass

            # Check if each stage is present in the LCAS
            graFEI_6inLCAS = 0
            graFEI_5inLCAS = 0
            graFEI_4inLCAS = 0
            graFEI_3inLCAS = 0
            graFEI_2inLCAS = 0
            graFEI_1inLCAS = 0
            graFEI_nIntermediates = 0
            graFEI_depthLCA = -1
            graFEI_nB1_daughters = 0
            graFEI_nB2_daughters = 0
            graFEI_notB_daughters = 0
            # masses_ordered = dict((i, []) for i in range(1, 7))
            if graFEI_validTree:
                level_intermediates, lca_idx_of_daughters, graFEI_depthLCA = n_intermediate_particles(
                    predicted_LCA_square_matched
                )

                graFEI_6inLCAS = level_intermediates[6]
                graFEI_5inLCAS = level_intermediates[5]
                graFEI_4inLCAS = level_intermediates[4]
                graFEI_3inLCAS = level_intermediates[3]
                graFEI_2inLCAS = level_intermediates[2]
                graFEI_1inLCAS = level_intermediates[1]
                graFEI_nIntermediates = (
                    graFEI_1inLCAS + graFEI_2inLCAS + graFEI_3inLCAS + graFEI_4inLCAS + graFEI_5inLCAS
                )
                b2.B2DEBUG(11, "This LCA describes a valid tree")
                b2.B2DEBUG(
                    11,
                    "Predicted LCA on matched particles:\n",
                    predicted_LCA_square_matched,
                )
                b2.B2DEBUG(11, "Adjacency matrix:\n", adjacency)
                b2.B2DEBUG(
                    11, "Number of intermediate particles:\n", level_intermediates
                )
                b2.B2DEBUG(
                    11,
                    "Indices of FSPs coming from each ancestor:\n",
                    lca_idx_of_daughters,
                )

                # # Compute invariant mass of nodes in LCA
                # nodes_masses = [
                #     round(i, 4) for i in node_masses(matched_list, lca_idx_of_daughters)
                # ]

                # Group the invariant masses
                # it = iter(nodes_masses)
                # masses_ordered = [
                #     list(itertools.islice(it, 0, i))
                #     for i in level_intermediates.values()
                # ]
                # masses_ordered = dict(
                #     (i, j) for i, j in zip(level_intermediates.keys(), masses_ordered)
                # )

                # Assign FSPs to B mesons with extraInfo
                # Only do that if we have exactly 1 Upsilon(4S) reconstructed
                if graFEI_6inLCAS == 1:
                    # Case with only one B
                    if graFEI_5inLCAS == 1:
                        graFEI_nB1_daughters = len(lca_idx_of_daughters[1])
                        for i, particle in enumerate(matched_list):
                            particle.addExtraInfo("graFEI_fromWhichB", 1 if i in lca_idx_of_daughters[1] else -1)
                    # Case with 2 Bs
                    elif graFEI_5inLCAS == 2:
                        # B1 has always >= particles than B2
                        whichB = 1 if len(lca_idx_of_daughters[1]) >= len(lca_idx_of_daughters[2]) else 2
                        graFEI_nB1_daughters = len(lca_idx_of_daughters[whichB])
                        graFEI_nB2_daughters = len(lca_idx_of_daughters[3 - whichB])
                        for i, particle in enumerate(matched_list):
                            if i in lca_idx_of_daughters[1]:
                                particle.addExtraInfo("graFEI_fromWhichB", whichB)
                            elif i in lca_idx_of_daughters[2]:
                                particle.addExtraInfo("graFEI_fromWhichB", 3 - whichB)

            # Particles not assigned to B decays get -1
            for particle in p_list:
                if not particle.hasExtraInfo("graFEI_fromWhichB"):
                    particle.addExtraInfo("graFEI_fromWhichB", -1)
                    graFEI_notB_daughters += 1

            # b2.B2DEBUG(11, "Invariant masses of intermediates:\n", masses_ordered)
            b2.B2DEBUG(11, "B1 number of daughters:", graFEI_nB1_daughters)
            b2.B2DEBUG(11, "B2 number of daughters:", graFEI_nB2_daughters)
            b2.B2DEBUG(11, "Not B number of daughters:", graFEI_notB_daughters)

            # Define B probabilities
            graFEI_probEdgeProd = edge_probability_unique.prod().item()
            graFEI_probEdgeMean = edge_probability_unique.mean().item()
            graFEI_probEdgeGeom = torch.pow(edge_probability_unique.prod(), 1/n_nodes).item()
            # graFEI_B_probGlobal = torch.sigmoid(u_pred[0][0]).item()

            # Add extra info for each B candidate (one for each event)
            candidate.addExtraInfo("graFEI_probEdgeProd", graFEI_probEdgeProd)
            candidate.addExtraInfo("graFEI_probEdgeMean", graFEI_probEdgeMean)
            candidate.addExtraInfo("graFEI_probEdgeGeom", graFEI_probEdgeGeom)
            candidate.addExtraInfo("graFEI_validTree", graFEI_validTree)
            candidate.addExtraInfo("graFEI_6inLCAS", graFEI_6inLCAS)
            candidate.addExtraInfo("graFEI_5inLCAS", graFEI_5inLCAS)
            candidate.addExtraInfo("graFEI_4inLCAS", graFEI_4inLCAS)
            candidate.addExtraInfo("graFEI_3inLCAS", graFEI_3inLCAS)
            candidate.addExtraInfo("graFEI_2inLCAS", graFEI_2inLCAS)
            candidate.addExtraInfo("graFEI_1inLCAS", graFEI_1inLCAS)
            candidate.addExtraInfo("graFEI_nIntermediates", graFEI_nIntermediates)
            candidate.addExtraInfo("graFEI_nFSP", graFEI_nFSP)
            candidate.addExtraInfo("graFEI_nPhotons_preFit", graFEI_nPhotons_preFit)
            candidate.addExtraInfo("graFEI_nCharged_preFit", graFEI_nCharged_preFit)
            candidate.addExtraInfo("graFEI_nElectrons_preFit", graFEI_nElectrons_preFit)
            candidate.addExtraInfo("graFEI_nMuons_preFit", graFEI_nMuons_preFit)
            candidate.addExtraInfo("graFEI_nPions_preFit", graFEI_nPions_preFit)
            candidate.addExtraInfo("graFEI_nKaons_preFit", graFEI_nKaons_preFit)
            candidate.addExtraInfo("graFEI_nProtons_preFit", graFEI_nProtons_preFit)
            candidate.addExtraInfo("graFEI_nLeptons_preFit", graFEI_nLeptons_preFit)
            candidate.addExtraInfo("graFEI_nPhotons_postFit", graFEI_nPhotons_postFit)
            candidate.addExtraInfo("graFEI_nCharged_postFit", graFEI_nCharged_postFit)
            candidate.addExtraInfo("graFEI_nElectrons_postFit", graFEI_nElectrons_postFit)
            candidate.addExtraInfo("graFEI_nMuons_postFit", graFEI_nMuons_postFit)
            candidate.addExtraInfo("graFEI_nPions_postFit", graFEI_nPions_postFit)
            candidate.addExtraInfo("graFEI_nKaons_postFit", graFEI_nKaons_postFit)
            candidate.addExtraInfo("graFEI_nProtons_postFit", graFEI_nProtons_postFit)
            candidate.addExtraInfo("graFEI_nLeptons_postFit", graFEI_nLeptons_postFit)
            candidate.addExtraInfo("graFEI_nOthers_postFit", graFEI_nOthers_postFit)
            candidate.addExtraInfo("graFEI_nPredictedUnmatched", graFEI_nPredictedUnmatched)
            candidate.addExtraInfo("graFEI_nPredictedUnmatched_noPhotons", graFEI_nPredictedUnmatched_noPhotons)
            candidate.addExtraInfo("graFEI_depthLCA", graFEI_depthLCA)
            candidate.addExtraInfo("graFEI_nBtag_daughters", graFEI_nB1_daughters)
            candidate.addExtraInfo("graFEI_nBsig_daughters", graFEI_nB2_daughters)
            candidate.addExtraInfo("graFEI_notB_daughters", graFEI_notB_daughters)

            # # Invariant masses of intermediates are stored in dedicated extraInfo, one for each intermediate
            # for lcas_level in range(1, 7):
            #     for i, mass in enumerate(masses_ordered[lcas_level], 1):
            #         candidate.addExtraInfo(f"graFEI_{lcas_level}Mass_{i}", mass)

            # Add MC truth information
            # if self.storeTrueInfo and Belle2.Environment.Instance().isMC():
            if self.storeTrueInfo:
                # Get the true IDs of the ancestors (if it's a B)
                parentID = np.array([p.getExtraInfo("BParentGenID") for p in p_list], dtype=int)
                b2.B2DEBUG(10, "Ancestor true ID:\n", parentID)

                # Get particle indices
                p_indices = np.array(
                    [
                        p.getMCParticle().getArrayIndex() if parentID[i] >= 0 else -1
                        for (i, p) in enumerate(p_list)
                    ]
                )
                # Get particle masses
                p_masses = masses_to_classes(
                    np.array(
                        [
                            p.getMCParticle().getPDG() if parentID[i] >= 0 else -1
                            for (i, p) in enumerate(p_list)
                        ]
                    )
                )
                b2.B2DEBUG(10, "True mass classes:\n", p_masses)
                # And primary information
                evt_primary = np.array(
                    [
                        p.getMCParticle().isPrimaryParticle()
                        if parentID[i] >= 0
                        else False
                        for (i, p) in enumerate(p_list)
                    ]
                )
                b2.B2DEBUG(10, "Is primary particle:\n", evt_primary)

                # Get unique B indices associated to each predicted matched particle which is also a primary
                # The idea is that if a primary particle coming from the other B is categorized as unmatched,
                # then it's ok and the decay could be truth-matched
                B_indices = parentID[np.logical_and(evt_primary, predicted_matched)]
                b2.B2DEBUG(
                    10, "Ancestor ID of predicted matched particles:\n", B_indices
                )
                B_indices = list(set(B_indices))

                # Initialize truth-matching variables
                graFEI_truth_perfectLCA = 0  # 1 if LCA perfectly reconstructed
                graFEI_truth_isSemileptonic = -1  # 0 if hadronic, 1 is semileptonic, -1 if not matched
                graFEI_truth_nFSP = -1  # Number of true FSPs
                graFEI_truth_depthLCA = -1  # Number of different classes in the LCA
                graFEI_truth_perfectMasses = int(
                    (predicted_masses.numpy() == p_masses).all()
                )  # Check if all the masses are predicted correctly
                graFEI_truth_nPhotons = (p_masses == 6).sum()
                graFEI_truth_nElectrons = (p_masses == 1).sum()
                graFEI_truth_nMuons = (p_masses == 2).sum()
                graFEI_truth_nPions = (p_masses == 3).sum()
                graFEI_truth_nKaons = (p_masses == 4).sum()
                graFEI_truth_nProtons = (p_masses == 5).sum()
                graFEI_truth_nOthers = (p_masses == 0).sum()

                # Get the generated B's
                gen_list = Belle2.PyStoreObj(self.storeTrueInfo)

                # Iterate over generated Ups
                if self.storeTrueInfo == "Upsilon(4S):MC" and gen_list.getListSize() > 1:
                    b2.B2WARNING(
                        f"Found {gen_list.getListSize()} true Upsilon(4S) in the generated MC (??)")

                if gen_list.getListSize() > 0:
                    # Here we look if the candidate has a perfectly reconstructed LCA
                    for mcp in gen_list.obj():
                        # If storing true info on B decays and we have paricles coming from different Bs
                        # the decay will not have a perfectLCA
                        if self.storeTrueInfo != "Upsilon(4S):MC" and len(B_indices) != 1:
                            break

                        # Get array index of MC particle
                        array_index = mcp.getArrayIndex()

                        # If we are reconstructing Bs, skip the other in the event
                        if self.storeTrueInfo != "Upsilon(4S):MC" and array_index != B_indices[0]:
                            continue

                        # Write leaf history
                        (
                            leaf_hist,
                            levels,
                            _,
                            _,
                            semilep_flag,
                        ) = write_hist(
                            particle=mcp,
                            leaf_hist={},
                            levels={},
                            hist=[],
                            pdg={},
                            leaf_pdg={},
                            semilep_flag=False,
                            )

                        # Skip B decays with trivial LCA (should be always false except for B -> nunu ?)
                        if len(leaf_hist) < 2:
                            continue

                        # Initialize LCA...
                        true_LCA_square = np.zeros(
                            [len(leaf_hist), len(leaf_hist)], dtype=int
                        )

                        # Number of true FSPs
                        graFEI_truth_nFSP = len(leaf_hist)

                        # ... and fill it!
                        for x, y in itertools.combinations(enumerate(leaf_hist), 2):
                            intersection = [
                                i for i in leaf_hist[x[1]] if i in leaf_hist[y[1]]
                            ]
                            true_LCA_square[x[0], y[0]] = levels[intersection[-1]]
                            true_LCA_square[y[0], x[0]] = levels[intersection[-1]]

                        try:
                            _, _, graFEI_truth_depthLCA = n_intermediate_particles(
                                torch.tensor(true_LCA_square, dtype=int)
                            )
                        except InvalidLCAMatrix:
                            pass

                        x_leaves = p_indices
                        y_leaves = list(leaf_hist.keys())

                        # Get LCA indices in order that the leaves appear in reconstructed particles
                        # Secondaries aren't in the LCA leaves list so they get a 0
                        locs = np.array(
                            [
                                np.where(y_leaves == i)[0].item()
                                if (i in y_leaves)
                                else 0
                                for i in x_leaves
                            ],
                            dtype=int,
                        )

                        # Insert dummy rows for secondaries
                        true_LCA_square = true_LCA_square[locs, :][:, locs]

                        # Set everything that's not primary (unmatched and secondaries) rows.cols to 0
                        # Note we only consider the subset of leaves that made it into x_rows
                        x_rows = np.array(
                            [
                                p.getExtraInfo("BParentGenID") == array_index
                                for p in p_list
                            ]
                        ) if self.storeTrueInfo != "Upsilon(4S):MC" else evt_primary

                        primaries_from_right_cand = np.logical_and(evt_primary, x_rows)

                        true_LCA_square = np.where(
                            primaries_from_right_cand, true_LCA_square, 0
                        )  # Set the rows
                        true_LCA_square = np.where(
                            primaries_from_right_cand[:, None], true_LCA_square, 0
                        )  # Set the columns

                        # Convert LCA to tensor
                        true_LCA_square = torch.tensor(true_LCA_square, dtype=int)
                        b2.B2DEBUG(10, "True LCA:\n", true_LCA_square)

                        # Check if perfect LCA
                        if (true_LCA_square == predicted_LCA_square).all():
                            graFEI_truth_perfectLCA = 1
                            b2.B2DEBUG(10, "LCA perfectly reconstructed!")

                        # Assign semileptonic flag
                        graFEI_truth_isSemileptonic = int(semilep_flag)

                # Perfect event = perfectLCA and perfectMasses
                graFEI_truth_perfectEvent = int(graFEI_truth_perfectLCA and graFEI_truth_perfectMasses)

                # Write extra info
                candidate.addExtraInfo("graFEI_truth_perfectLCA", graFEI_truth_perfectLCA)
                candidate.addExtraInfo("graFEI_truth_perfectMasses", graFEI_truth_perfectMasses)
                candidate.addExtraInfo("graFEI_truth_perfectEvent", graFEI_truth_perfectEvent)
                candidate.addExtraInfo("graFEI_truth_isSemileptonic", graFEI_truth_isSemileptonic)
                candidate.addExtraInfo("graFEI_truth_nFSP", graFEI_truth_nFSP)
                candidate.addExtraInfo("graFEI_truth_depthLCA", graFEI_truth_depthLCA)
                candidate.addExtraInfo("graFEI_truth_nPhotons", graFEI_truth_nPhotons)
                candidate.addExtraInfo("graFEI_truth_nElectrons", graFEI_truth_nElectrons)
                candidate.addExtraInfo("graFEI_truth_nMuons", graFEI_truth_nMuons)
                candidate.addExtraInfo("graFEI_truth_nPions", graFEI_truth_nPions)
                candidate.addExtraInfo("graFEI_truth_nKaons", graFEI_truth_nKaons)
                candidate.addExtraInfo("graFEI_truth_nProtons", graFEI_truth_nProtons)
                candidate.addExtraInfo("graFEI_truth_nOthers", graFEI_truth_nOthers)
