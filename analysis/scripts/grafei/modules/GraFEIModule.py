##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import itertools
import numpy as np
import yaml
import warnings
import basf2 as b2
from ROOT import Belle2
from variables import variables as vm
import torch
from torch_geometric.data import Batch
from grafei.modules.LCASaverModule import get_object_list, write_hist
from grafei.model.geometric_network import GraFEIModel
from grafei.model.normalize_features import normalize_features
from grafei.model.edge_features import compute_edge_features
from grafei.model.lca_to_adjacency import lca_to_adjacency, InvalidLCAMatrix, select_good_decay
from grafei.model.tree_utils import masses_to_classes

warnings.filterwarnings(
    action="ignore", category=RuntimeWarning, message="Mean of empty slice.*"
)


class GraFEIModule(b2.Module):
    """
    Applies graFEI model to a particle list in basf2.
    GraFEI information is stored as extraInfos.

    Args:
        particle_list (str): Name of particle list.
        cfg_path (str): Path to config file. If `None` the config file in the global tag is used.
        param_file (str): Path to parameter file containing the model. If `None` the parameter file in the global tag is used.
        sig_side_lcas (list): List containing LCAS matrix of signal-side.
        sig_side_masses (list): List containing mass hypotheses of signal-side.
        gpu (bool): Whether to run on a GPU.
        payload_config_name (str): Name of config file payload. The default should be kept, except in basf2 examples.
        payload_model_name (str): Name of model file payload. The default should be kept, except in basf2 examples.
    """

    def __init__(
        self,
        particle_list,
        cfg_path=None,
        param_file=None,
        sig_side_lcas=None,
        sig_side_masses=None,
        gpu=False,
        payload_config_name="graFEIConfigFile",
        payload_model_name="graFEIModelFile",
    ):
        """
        Initialization.
        """
        super().__init__()
        #: Input particle list
        self.particle_list = particle_list
        #: Config yaml file path
        self.cfg_path = cfg_path
        #: PyTorch parameter file path
        self.param_file = param_file
        #: Chosen sig-side LCAS
        self.sig_side_lcas = torch.tensor(sig_side_lcas) if sig_side_lcas else None
        #: Chosen sig-side mass hypotheses
        self.sig_side_masses = sig_side_masses
        #: If running on GPU
        self.gpu = gpu
        #: Config file name in the payload
        self.payload_config_name = payload_config_name
        #: Model file name in the payload
        self.payload_model_name = payload_model_name

    def initialize(self):
        """
        Called at the beginning.
        """
        # Get weights and configs from the DB if they are not provided from the user
        if not self.cfg_path:
            config = Belle2.DBAccessorBase(
                Belle2.DBStoreEntry.c_RawFile, self.payload_config_name, True
            )
            self.cfg_path = config.getFilename()
        if not self.param_file:
            model = Belle2.DBAccessorBase(
                Belle2.DBStoreEntry.c_RawFile, self.payload_model_name, True
            )
            self.param_file = model.getFilename()

        #: Figure out if we re running on data or MC
        self.storeTrueInfo = Belle2.Environment.Instance().isMC()

        #: Figure out which device all this is running on - CPU or GPU
        self.device = torch.device(
            "cuda" if (self.gpu and torch.cuda.is_available()) else "cpu"
        )

        # Load configs
        cfg_file = open(self.cfg_path, "r")
        #: Config file
        self.configs = yaml.safe_load(cfg_file)

        #: Top MC particle
        self.mc_particle = None
        #: Max LCAS level
        self.max_level = None
        # B or Ups reco? 0 = Ups, 1 = B0, 2 = B+
        if self.configs["model"]["B_reco"] == 0:
            self.mc_particle = "Upsilon(4S):MC"
            self.max_level = 6
        elif self.configs["model"]["B_reco"] == 1:
            self.mc_particle = "B0:MC"
            self.max_level = 5
        elif self.configs["model"]["B_reco"] == 2:
            self.mc_particle = "B+:MC"
            self.max_level = 5
        else:
            b2.B2FATAL("The B_reco setting in the config file is incorrect.")

        #: Normalize features
        self.normalize = self.configs["dataset"]["config"]["normalize"]

        #: Mixed precision
        self.use_amp = self.configs["train"][
            "mixed_precision"
        ] and self.device == torch.device("cuda")

        #: Node features
        self.node_features = self.configs["dataset"]["config"]["features"]
        #: Edge features
        self.edge_features = self.configs["dataset"]["config"]["edge_features"]
        #: Global features
        self.glob_features = self.configs["dataset"]["config"]["global_features"]

        # Naming convention
        self.node_features = [f"feat_{name}" for name in self.node_features] if self.node_features else []
        self.edge_features = [f"edge_{name}" for name in self.edge_features] if self.edge_features else []
        self.glob_features = [f"glob_{name}" for name in self.glob_features] if self.glob_features else []
        #: Discarded node features
        self.discarded_features = ["feat_x", "feat_y", "feat_z", "feat_px", "feat_py", "feat_p"]

        # Extract the number of features
        n_infeatures = len(self.node_features)
        e_infeatures = len(self.edge_features)
        g_infeatures = len(self.glob_features)

        #: The model
        # The correct edge_classes is taken from the config file
        self.model = GraFEIModel(
            nfeat_in_dim=n_infeatures,
            efeat_in_dim=e_infeatures,
            gfeat_in_dim=g_infeatures,
            **self.configs["model"],
        )

        # Load paramaters' values
        self.model.load_state_dict(
            torch.load(self.param_file, map_location=self.device)["model"]
        )

        # Activate evaluation mode
        self.model.eval()
        # Push model to GPU in case
        self.model.to(self.device)

        b2.B2DEBUG(10, "Model structure:\n", {self.model})

    def event(self):
        """
        Called at the beginning of each event.
        """
        b2.B2DEBUG(10, "---- Processing new event ----")

        # Get the B candidate list
        candidate_list = get_object_list(Belle2.PyStoreObj(self.particle_list).obj())

        # Get the particle candidate(s)
        for candidate in candidate_list:
            # Get FSPs
            p_list = get_object_list(candidate.getFinalStateDaughters())

            # Number of FSPs
            n_nodes = len(p_list)

            # Particle nature
            masses = np.array([abs(p.getPDGCode()) for p in p_list])

            # Number of charged and photons
            graFEI_nFSP = n_nodes
            graFEI_nPhotons_preFit = (masses == 22).sum()
            graFEI_nCharged_preFit = graFEI_nFSP - graFEI_nPhotons_preFit
            graFEI_nElectrons_preFit = (masses == 11).sum()
            graFEI_nMuons_preFit = (masses == 13).sum()
            graFEI_nPions_preFit = (masses == 211).sum()
            graFEI_nKaons_preFit = (masses == 321).sum()
            graFEI_nProtons_preFit = (masses == 2212).sum()
            graFEI_nLeptons_preFit = graFEI_nElectrons_preFit + graFEI_nMuons_preFit
            graFEI_nOthers_preFit = graFEI_nCharged_preFit - \
                (graFEI_nLeptons_preFit + graFEI_nPions_preFit + graFEI_nKaons_preFit + graFEI_nProtons_preFit)

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

            # Trivial decay tree
            if n_nodes < 2:
                b2.B2WARNING(
                    f"Skipping candidate with {n_nodes} reconstructed FSPs"
                )

                continue

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

            # Create Batch object to be passed to model
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

            # Assign new mass hypotheses as extraInfo
            for i, p in enumerate(p_list):
                p.addExtraInfo("graFEI_massHypothesis", predicted_masses[i])

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
            predicted_matched_noPhotons = predicted_matched[masses != 22]

            # Get number of predicted as unmatched
            graFEI_nPredictedUnmatched = (~predicted_matched).sum()
            graFEI_nPredictedUnmatched_noPhotons = (
                (~predicted_matched_noPhotons).sum()
                if predicted_matched_noPhotons.size != 0
                else 0
            )

            # Get LCA of predicted matched only
            predicted_LCA_square_matched = predicted_LCA_square[predicted_matched]
            predicted_LCA_square_matched = predicted_LCA_square_matched[:, predicted_matched]

            # Get predicted masses of predicted matched only
            predicted_masses_matched = predicted_masses[predicted_matched]

            # Check if LCA describes a tree graph
            graFEI_validTree = 0
            if not torch.all(predicted_LCA_square == 0):
                try:
                    adjacency = lca_to_adjacency(predicted_LCA_square_matched)
                    graFEI_validTree = 1
                except InvalidLCAMatrix:
                    pass

            # Check if event is good, depending on the chosen sig-side LCA matrix/masses
            graFEI_goodEvent = 0
            if graFEI_validTree:
                # Check if the event is good
                good_decay, root_level, sig_side_fsps = select_good_decay(predicted_LCA_square_matched,
                                                                          predicted_masses_matched,
                                                                          self.sig_side_lcas,
                                                                          self.sig_side_masses)
                graFEI_goodEvent = int((self.max_level == root_level) and good_decay)

                if graFEI_goodEvent:
                    # Find sig- and tag-side FSPs (1 = sig-side, 0 = tag-side)
                    p_list_matched = list(np.array(p_list)[predicted_matched])
                    for i, particle in enumerate(p_list_matched):
                        if i in sig_side_fsps:
                            particle.addExtraInfo("graFEI_sigSide", 1)
                        else:
                            particle.addExtraInfo("graFEI_sigSide", 0)

                b2.B2DEBUG(11, "This LCA describes a valid tree")
                b2.B2DEBUG(
                    11,
                    "Predicted LCA on matched particles:\n",
                    predicted_LCA_square_matched,
                )
                b2.B2DEBUG(11, "Adjacency matrix:\n", adjacency)

            # Particles not assigned to B decays get -1
            for particle in p_list:
                if not particle.hasExtraInfo("graFEI_sigSide"):
                    particle.addExtraInfo("graFEI_sigSide", -1)

            # Define B probabilities
            graFEI_probEdgeProd = edge_probability_unique.prod().item()
            graFEI_probEdgeMean = edge_probability_unique.mean().item()
            graFEI_probEdgeGeom = torch.pow(edge_probability_unique.prod(), 1/n_nodes).item()

            # Add extra info for each B candidate
            candidate.addExtraInfo("graFEI_probEdgeProd", graFEI_probEdgeProd)
            candidate.addExtraInfo("graFEI_probEdgeMean", graFEI_probEdgeMean)
            candidate.addExtraInfo("graFEI_probEdgeGeom", graFEI_probEdgeGeom)
            candidate.addExtraInfo("graFEI_validTree", graFEI_validTree)
            candidate.addExtraInfo("graFEI_goodEvent", graFEI_goodEvent)
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

            # Add MC truth information
            if self.storeTrueInfo:
                # Get the true IDs of the ancestors (if it's a B)
                parentID = np.array([vm.evaluate("ancestorBIndex", p) for p in p_list], dtype=int)
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
                # then it's ok and the decay could still have a perfectLCA
                B_indices = parentID[np.logical_and(evt_primary, predicted_matched)]
                b2.B2DEBUG(
                    10, "Ancestor ID of predicted matched particles:\n", B_indices
                )
                B_indices = list(set(B_indices))

                # Initialize truth-matching variables
                graFEI_truth_perfectLCA = 0  # 1 if LCA perfectly reconstructed
                graFEI_truth_isSemileptonic = -1  # 0 if hadronic, 1 is semileptonic, -1 if not matched
                graFEI_truth_nFSP = -1  # Number of true FSPs
                graFEI_truth_perfectMasses = int((predicted_masses.numpy() == p_masses).all()
                                                 )  # Check if all the masses are predicted correctly
                graFEI_truth_nPhotons = (p_masses == 6).sum()
                graFEI_truth_nElectrons = (p_masses == 1).sum()
                graFEI_truth_nMuons = (p_masses == 2).sum()
                graFEI_truth_nPions = (p_masses == 3).sum()
                graFEI_truth_nKaons = (p_masses == 4).sum()
                graFEI_truth_nProtons = (p_masses == 5).sum()
                graFEI_truth_nOthers = (p_masses == 0).sum()

                # Get the generated B's
                gen_list = Belle2.PyStoreObj(self.mc_particle)

                # Iterate over generated Ups
                if self.mc_particle == "Upsilon(4S):MC" and gen_list.getListSize() > 1:
                    b2.B2WARNING(
                        f"Found {gen_list.getListSize()} true Upsilon(4S) in the generated MC (??)")

                if gen_list.getListSize() > 0:
                    # Here we look if the candidate has a perfectly reconstructed LCA
                    for genP in gen_list.obj():
                        mcp = genP.getMCParticle()
                        # If storing true info on B decays and we have matched particles coming
                        # from different Bs the decay will not have a perfectLCA
                        if self.mc_particle != "Upsilon(4S):MC" and len(B_indices) != 1:
                            break

                        # Get array index of MC particle
                        array_index = mcp.getArrayIndex()

                        # If we are reconstructing Bs, skip the other in the event
                        if self.mc_particle != "Upsilon(4S):MC" and array_index != B_indices[0]:
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
                                vm.evaluate("ancestorBIndex", p) == array_index
                                for p in p_list
                            ]
                        ) if self.mc_particle != "Upsilon(4S):MC" else evt_primary

                        primaries_from_right_cand = np.logical_and(evt_primary, x_rows)

                        # Set the rows
                        true_LCA_square = np.where(
                            primaries_from_right_cand, true_LCA_square, 0
                        )
                        # Set the columns
                        true_LCA_square = np.where(
                            primaries_from_right_cand[:, None], true_LCA_square, 0
                        )

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
                candidate.addExtraInfo("graFEI_truth_nPhotons", graFEI_truth_nPhotons)
                candidate.addExtraInfo("graFEI_truth_nElectrons", graFEI_truth_nElectrons)
                candidate.addExtraInfo("graFEI_truth_nMuons", graFEI_truth_nMuons)
                candidate.addExtraInfo("graFEI_truth_nPions", graFEI_truth_nPions)
                candidate.addExtraInfo("graFEI_truth_nKaons", graFEI_truth_nKaons)
                candidate.addExtraInfo("graFEI_truth_nProtons", graFEI_truth_nProtons)
                candidate.addExtraInfo("graFEI_truth_nOthers", graFEI_truth_nOthers)
