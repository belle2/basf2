##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import basf2 as b2
import numpy as np
import copy
from itertools import combinations
from variables import variables as vm


def get_object_list(pointerVec):
    """
    Workaround to avoid memory problems in basf2.

    Args:
        pointerVec: Input particle list.

    Returns:
        list: Output python list.
    """
    from ROOT import Belle2

    objList = []
    size = (
        pointerVec.getListSize()
        if isinstance(pointerVec, Belle2.ParticleList)
        else len(pointerVec)
    )
    for i in range(size):
        objList.append(pointerVec[i])
    return objList


def pdg_to_lca_converter(pdg):
    """
    Converts PDG code to LCAS classes.

    .. tip:: If you want to modify the LCAS classes, it's here.
        Don't forget to update the number of edge classes accordingly in the yaml file.

    Args:
        pdg (int): PDG code to convert.

    Returns:
        int or None: Corresponding LCAS class, or None if PDG not present in ``pdg_lca_match``.
    """
    # MC PDG code
    pdg_lca_match = {
        443: 1,  # J/psi
        111: 1,  # pi^0
        310: 2,  # K_S^0
        # 130: 2, #K_L^0
        421: 3,  # D^0
        411: 3,  # D^+
        431: 3,  # D_s^+
        413: 4,  # D^+*
        423: 4,  # D^0*
        433: 4,  # D_s^*+
        521: 5,  # B^+
        511: 5,  # B^0
        300553: 6,  # Upsilon(4S)
    }

    pdg = abs(pdg)
    if pdg in pdg_lca_match:
        return pdg_lca_match[pdg]
    else:
        return None


def _update_levels(levels, hist, pdg):
    """
    Assigns LCAS level to each particle in the decay tree.

    Arguments are automatically set.
    """
    for i, n in enumerate(hist):
        if n in levels.keys():
            continue

        lca = pdg_to_lca_converter(pdg[n])
        if lca:
            levels[n] = lca
        else:
            for j in range(i + 1):
                lca = pdg_to_lca_converter(pdg[hist[i - j]])
                if lca:
                    levels[n] = lca
                    break

    return levels


def write_hist(
    particle,
    leaf_hist={},
    levels={},
    hist=[],
    pdg={},
    leaf_pdg={},
    semilep_flag=False,
):
    """
    Recursive function to traverse down to the leaves saving the history.

    Args:
        particle: The current particle being inspected.
            Other arguments are automatically set.
    """

    neutrino_pdgs = [12, 14, 16, 18]

    # Check if there's any neutrinos in the tree and set semileptonic flag
    # Need to only include primary particle since they're what couns in the LCA
    if (abs(particle.getPDG()) in neutrino_pdgs) and particle.isPrimaryParticle():
        semilep_flag = True

    # Need to create a true copy, no just a ref to the object
    hist = copy.deepcopy(hist)
    leaf_hist = copy.deepcopy(leaf_hist)
    leaf_pdg = copy.deepcopy(leaf_pdg)
    levels = copy.deepcopy(levels)
    pdg = copy.deepcopy(pdg)

    # Below is deciding what to save to the LCA matrix
    # If primary or secondary with no daughters:
    # save
    # If primary with no primary daughters:
    # save and continue down daughters
    # else:
    # continue down daughters

    # Check if we're a primary particle with no primary daughters
    prim_no_prim_daughters = (
        len(
            [d for d in get_object_list(particle.getDaughters()) if d.isPrimaryParticle()]
        )
        == 0
    )

    # If it's a leaf we save it
    # Also check that it has no primary daughters
    if (
        # particle.isPrimaryParticle() and
        (particle.getNDaughters() == 0)
        or prim_no_prim_daughters  # Even if saving secondaries we need to record primaries with no primary daughters
    ):
        # Won't save neutrinos or very low energy photons to LCA
        if abs(particle.getPDG()) not in neutrino_pdgs and not (
            particle.getPDG() == 22 and particle.getEnergy() < 1e-4
        ):
            # Leaves get their history added
            leaf_hist[particle.getArrayIndex()] = hist
            leaf_pdg[particle.getArrayIndex()] = particle.getPDG()

            # And now that we have a full history down to the leaf
            # we can update the levels
            levels = _update_levels(levels, hist, pdg)

    # Here is deciding whether to continue traversing down the decay tree
    # Don't want to do this if all daughters are secondaries since we're not saving them
    if (
        particle.getNDaughters() != 0
    ) and not prim_no_prim_daughters:  # Don't travers if all daughters are secondaries
        # Only append primaries to history
        if particle.isPrimaryParticle():
            hist.append(particle.getArrayIndex())
            # Save all PDG values of the in between primary particles
            pdg[particle.getArrayIndex()] = particle.getPDG()

        # Now iterate over daughters passing history down
        daughters = get_object_list(particle.getDaughters())
        for daughter in daughters:
            (
                leaf_hist,
                levels,
                pdg,
                leaf_pdg,
                semilep_flag,
            ) = write_hist(
                daughter,
                leaf_hist,
                levels,
                hist,
                pdg,
                leaf_pdg,
                semilep_flag,
            )

    return (
        leaf_hist,
        levels,
        pdg,
        leaf_pdg,
        semilep_flag,
    )


class LCASaverModule(b2.Module):
    """
    Save Lowest Common Ancestor matrix of each MC Particle in the given list.

    Args:
        particle_lists (list): Name of particle lists to save features of.
        features (list): List of features to save for each particle.
        mcparticle_list (str): Name of particle list to build LCAs from (will use as root).
        output_file (str): Path to output file to save.
    """

    def __init__(
        self,
        particle_lists,
        features,
        mcparticle_list,
        output_file,
    ):
        """
        Initialization.
        """
        super().__init__()
        #: Input particle lists
        self.particle_lists = particle_lists
        #: Features to save
        self.features = features
        #: MC particle list (Upsilon, B0, B+)
        self.mcparticle_list = mcparticle_list
        #: Output file name
        self.output_file = output_file

        #: Max number of particles
        # It doesn't actually matter what this is as long as it's bigger than
        # the number of particles in any events we'll encounter. ROOT won't save all entries.
        self.max_particles = 500

    def initialize(self):
        """
        Called once at the beginning.
        """
        from ROOT import Belle2, TFile, TTree

        #: Event info
        self.eventinfo = Belle2.PyStoreObj("EventMetaData")

        #: ROOT output file
        self.root_outfile = TFile(self.output_file, "recreate")
        #: ROOT tree
        self.tree = TTree("Tree", "tree")

        #: Event number
        self.event_num = np.zeros(1, dtype=np.int32)
        self.tree.Branch("event", self.event_num, "event/I")
        #: bool containing information whether we reconstruct B or Upsilon
        self.isB = np.zeros(1, dtype=bool)
        self.tree.Branch("isB", self.isB, "isB/b")

        #: Truth information
        self.truth_dict = {}

        # We assume at most two LCA matrices for event
        for i in [1, 2]:
            self.truth_dict[f"n_LCA_leaves_{i}"] = np.zeros(1, dtype=np.int32)
            self.truth_dict[f"LCA_leaves_{i}"] = np.zeros(
                self.max_particles, dtype=np.int32
            )
            self.tree.Branch(
                f"n_LCA_leaves_{i}",
                self.truth_dict[f"n_LCA_leaves_{i}"],
                f"n_LCA_leaves_{i}/I",
            )
            self.tree.Branch(
                f"LCA_leaves_{i}",
                self.truth_dict[f"LCA_leaves_{i}"],
                f"LCA_leaves_{i}[n_LCA_leaves_{i}]/I",
            )

            self.truth_dict[f"n_LCA_{i}"] = np.zeros(1, dtype=np.int32)
            self.truth_dict[f"LCAS_{i}"] = np.zeros(
                self.max_particles**2, dtype=np.uint8
            )
            self.tree.Branch(f"n_LCA_{i}", self.truth_dict[f"n_LCA_{i}"], f"n_LCA_{i}/I")
            self.tree.Branch(
                f"LCAS_{i}", self.truth_dict[f"LCAS_{i}"], f"LCAS_{i}[n_LCA_{i}]/b"
            )

        # Feature data
        #: Number of particles
        self.n_particles = np.zeros(1, dtype=np.int32)
        self.tree.Branch("n_particles", self.n_particles, "n_particles/I")

        #: Particle-is-primary flag
        self.primary = np.zeros(self.max_particles, dtype=bool)
        self.tree.Branch("primary", self.primary, "primary[n_particles]/O")

        #: Leaves in event
        self.leaves = np.zeros(self.max_particles, dtype=np.int32)
        self.tree.Branch("leaves", self.leaves, "leaves[n_particles]/I")

        #: B index
        self.b_index = np.zeros(self.max_particles, dtype=np.int32)
        self.tree.Branch("b_index", self.b_index, "b_index[n_particles]/I")

        #: Features dictionary
        self.feat_dict = {}
        for feat in self.features:
            self.feat_dict[feat] = np.zeros(self.max_particles, np.float32)
            self.tree.Branch(
                f"feat_{feat}", self.feat_dict[feat], f"feat_{feat}[n_particles]/F"
            )

        #: True MC PDG
        self.mc_pdg = np.zeros(self.max_particles, np.float32)
        self.tree.Branch("mcPDG", self.mc_pdg, "mcPDG[n_particles]/F")

    def _reset_LCA(self):
        """
        Resets the value of the LCA to 0.
        """
        for p_index in [1, 2]:
            self.truth_dict[f"LCAS_{p_index}"][: self.max_particles**2] *= 0
            self.truth_dict[f"n_LCA_leaves_{p_index}"][0] *= 0
            self.truth_dict[f"LCA_leaves_{p_index}"][: self.max_particles] *= 0
            self.truth_dict[f"n_LCA_{p_index}"][0] *= 0

    def event(self):
        """
        Called for each event.
        """
        from ROOT import Belle2  # noqa: make Belle2 namespace available
        from ROOT.Belle2 import PyStoreObj

        # Get the particle list (note this is a regular Particle list, not MCParticle)
        p_list = PyStoreObj(self.mcparticle_list)

        self.event_num[0] = self.eventinfo.getEvent()

        # Is Upsilon flag
        if self.mcparticle_list == "Upsilon(4S):MC":
            self.isB[0] = 0
        elif self.mcparticle_list == "B0:MC":
            self.isB[0] = 1
        elif self.mcparticle_list == "B+:MC":
            self.isB[0] = 2

        # Create the LCA
        # IMPORTANT: The ArrayIndex is 0-based.
        # mcplist contains the root particles we are to create LCAs from
        # Reset the LCA list so if only one B is there it does not carry an older version over
        self._reset_LCA()

        if p_list.getListSize() > 0:
            for part in p_list.obj():
                # Get the corresponding MCParticle
                mcp = part.getMCParticle()
                # In this way the LCA variables in the ntuples will be labelled _1 and _2
                # If we train on B decays these will correspond to the two B's
                # while if we train on the Upsilon, _1 will correspond to it and _2 will remain empty
                # becaus getArrayIndex() gives 0 for the Upsilon and 1, 2 for the B's
                array_index = 1 if self.isB[0] == 0 else mcp.getArrayIndex()

                # Call function to write history of leaves in the tree.
                # It internally calls function update_levels to find and save the level of each particle in the tree.
                # Necessary step to build the LCA.
                (
                    lcas_leaf_hist,
                    lcas_levels,
                    _, _, _,
                ) = write_hist(
                    particle=mcp,
                    leaf_hist={},
                    levels={},
                    hist=[],
                    pdg={},
                    leaf_pdg={},
                    semilep_flag=False,
                )

                lcas = np.zeros([len(lcas_leaf_hist), len(lcas_leaf_hist)])

                for x, y in combinations(enumerate(lcas_leaf_hist), 2):
                    lcas_intersection = [
                        i for i in lcas_leaf_hist[x[1]] if i in lcas_leaf_hist[y[1]]
                    ]
                    lcas[x[0], y[0]] = lcas_levels[lcas_intersection[-1]]
                    lcas[y[0], x[0]] = lcas_levels[lcas_intersection[-1]]

                self.truth_dict[f"LCAS_{array_index}"][: lcas.size] = lcas.flatten()

                self.truth_dict[f"n_LCA_leaves_{array_index}"][0] = len(lcas_leaf_hist.keys())
                self.truth_dict[f"LCA_leaves_{array_index}"][
                    : len(lcas_leaf_hist.keys())
                ] = list(lcas_leaf_hist.keys())

                self.truth_dict[f"n_LCA_{array_index}"][0] = lcas.size

            # ### Create the features
            # Where we'll append features
            evt_feats = {f: [] for f in self.features}

            # Ideally this would be saved in evt_feat_dict but the -1 for unmatched
            # particles messes that up
            evt_leaf_dict = {
                "leaves": [],
                "primary": [],
                "b_index": [],
                "mc_pdg": [],
            }

            # IMPORTANT: The ArrayIndex is 0-based.
            # mcplist contains the root particles we are to create LCAs from
            for p_list_name in self.particle_lists:
                # Get the particle list (note this is a regular Particle list, not MCParticle)
                p_list = PyStoreObj(p_list_name)

                for particle in p_list.obj():
                    # Get the B parent index, set to -1 if particle has no MC match
                    b_index = int(vm.evaluate("ancestorBIndex", particle))
                    # Need this to reorder LCA later, returns -1 if no MC match
                    if b_index >= 0:
                        p_index = particle.getMCParticle().getArrayIndex()
                        p_primary = particle.getMCParticle().isPrimaryParticle()
                        # Save particle's PDG code
                        mc_pdg = particle.getMCParticle().getPDG()
                    else:
                        p_index = -1
                        p_primary = False
                        mc_pdg = 0

                    evt_leaf_dict["primary"].append(p_primary)
                    evt_leaf_dict["leaves"].append(p_index)
                    evt_leaf_dict["b_index"].append(b_index)
                    evt_leaf_dict["mc_pdg"].append(mc_pdg)
                    for feat in self.features:
                        evt_feats[feat].append(vm.evaluate(feat, particle))

            n_particles = len(evt_leaf_dict["primary"])
            self.n_particles[0] = n_particles

            self.primary[:n_particles] = evt_leaf_dict["primary"]
            self.leaves[:n_particles] = evt_leaf_dict["leaves"]
            self.b_index[:n_particles] = evt_leaf_dict["b_index"]
            self.mc_pdg[:n_particles] = evt_leaf_dict["mc_pdg"]

            for feat in self.features:
                self.feat_dict[feat][:n_particles] = evt_feats[feat]
            # Add number of final state particles

            # Write everything to the TTree
            self.tree.Fill()

    def terminate(self):
        """
        Called at the end.
        """
        self.root_outfile.Write()
        self.root_outfile.Close()
