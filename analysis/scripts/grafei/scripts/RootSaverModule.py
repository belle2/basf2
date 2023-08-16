import ROOT as root
from ROOT import Belle2
import basf2 as b2
import numpy as np
import copy
from itertools import combinations
from variables import variables as vm
from grafei.scripts.FlagBDecayModule import getObjectList

# taken from MC PDG code
FEI_pdg_converter = {
    443: 1,  # J/psi
    111: 1,  # pi^0 #different groups because decay is very different
    310: 2,  # K_S^0
    # 130, #K_L^0 #this should be part of the first step particles with e,mu etc, its a flag for FEI
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


def update_levels(levels, hist):
    """This updates the highest level each node in the decay exists at

    Args:
        levels (dict): Dictionary with ArrayIndex:max_level
        hist (list): A leaf's history of ancestor ArrayIndexes up to the root, leaf itself not included.
    """
    # Reverse and start enumeration at 1 so the enumeration
    # number is the level of each node in the history
    for i, n in enumerate(reversed(hist), 1):
        if n not in levels:
            levels[n] = i
        else:
            # Want to record the max level a node exists on only
            levels[n] = max(levels[n], i)

    return levels


def update_levels_LCAS(levels, hist, pdg, intermediate_skipped=False):
    for i, n in enumerate(hist):
        if n not in levels.keys():
            temp_pdg = abs(
                pdg[n]
            )  # Take the absolute value of the pdg for the converter
            if temp_pdg in FEI_pdg_converter:
                levels[n] = FEI_pdg_converter[temp_pdg]
            else:
                intermediate_skipped = True
                for j in range(i + 1):
                    temp_pdg = abs(pdg[hist[i - j]])
                    if temp_pdg in FEI_pdg_converter:
                        levels[n] = FEI_pdg_converter[temp_pdg]
                        break

    return levels, intermediate_skipped


def write_hist(
    particle,
    leaf_hist={},
    levels={},
    hist=[],
    pdg={},
    leaf_pdg={},
    leaf_E={},
    leaf_theta={},
    leaf_mother_pdg={},
    semilep_flag=False,
    electron=False,
    intermediate_skipped=False,
    LCAS=False,
    save_secondaries=False,
):
    """Recursive function to traverse down to the leaves saving the history

    Args:
        particle (MCParticle): The current particle being inspected
        semilep_flag (bool): Whether or not the current decay is semileptonic
        intermediate_skipped (bool): Wheter or not an intermediate particle was skipped in the reconstruction (excited states...)
    """

    neutrino_pdgs = [12, 14, 16, 18]

    # Check if there's any neutrinos in the tree and set semileptonic flag
    # Need to only include primary particle since they're what couns in the LCA
    if (abs(particle.getPDG()) in neutrino_pdgs) and particle.isPrimaryParticle():
        semilep_flag = True

    if (abs(particle.getPDG()) == 11) and particle.isPrimaryParticle():
        electron = True

    # Need to create a true copy, no just a ref to the object
    hist = copy.deepcopy(hist)
    leaf_hist = copy.deepcopy(leaf_hist)
    leaf_pdg = copy.deepcopy(leaf_pdg)
    leaf_E = copy.deepcopy(leaf_E)
    leaf_theta = copy.deepcopy(leaf_theta)
    leaf_mother_pdg = copy.deepcopy(leaf_mother_pdg)
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
            [d for d in getObjectList(particle.getDaughters()) if d.isPrimaryParticle()]
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
            leaf_E[particle.getArrayIndex()] = particle.getEnergy()
            leaf_theta[particle.getArrayIndex()] = particle.getMomentum().Theta()
            leaf_mother_pdg[particle.getArrayIndex()] = (
                particle.getMother().getPDG() if particle.getMother() else 0
            )

            # And now that we have a full history down to the leaf
            # we can update the levels
            if LCAS:
                # print("update LCAS")
                levels, intermediate_skipped = update_levels_LCAS(
                    levels, hist, pdg, intermediate_skipped
                )
            else:
                levels = update_levels(levels, hist)

    # Here is deciding whether to continue traversing down the decay tree
    # Don't want to do this if all daughters are secondaries and we're not saving them
    if (
        particle.getNDaughters() != 0
    ) and not (  # if saving secondaries we can traverse down no matter what
        (not save_secondaries) and prim_no_prim_daughters
    ):  # When not saving secondaries, don't travers if all daughters are secondaries
        # Only append primaries to history
        if particle.isPrimaryParticle():
            hist.append(particle.getArrayIndex())
            # save all PDG values of the in between primary particles
            pdg[particle.getArrayIndex()] = particle.getPDG()

        # Now iterate over daughters passing history down
        daughters = getObjectList(particle.getDaughters())
        for daughter in daughters:
            if LCAS:
                (
                    leaf_hist,
                    levels,
                    pdg,
                    leaf_pdg,
                    leaf_E,
                    leaf_theta,
                    leaf_mother_pdg,
                    semilep_flag,
                    electron,
                    intermediate_skipped,
                ) = write_hist(
                    daughter,
                    leaf_hist,
                    levels,
                    hist,
                    pdg,
                    leaf_pdg,
                    leaf_E,
                    leaf_theta,
                    leaf_mother_pdg,
                    semilep_flag,
                    electron,
                    intermediate_skipped,
                    True,
                )
            else:
                (
                    leaf_hist,
                    levels,
                    pdg,
                    leaf_pdg,
                    leaf_E,
                    leaf_theta,
                    leaf_mother_pdg,
                    semilep_flag,
                    electron,
                ) = write_hist(
                    daughter,
                    leaf_hist,
                    levels,
                    hist,
                    pdg,
                    leaf_pdg,
                    leaf_E,
                    leaf_theta,
                    leaf_mother_pdg,
                    semilep_flag,
                    electron,
                )
    if LCAS:
        return (
            leaf_hist,
            levels,
            pdg,
            leaf_pdg,
            leaf_E,
            leaf_theta,
            leaf_mother_pdg,
            semilep_flag,
            electron,
            intermediate_skipped,
        )
    else:
        return (
            leaf_hist,
            levels,
            pdg,
            leaf_pdg,
            leaf_E,
            leaf_theta,
            leaf_mother_pdg,
            semilep_flag,
            electron,
        )


class RootSaverModule(b2.Module):
    """Save Lowest Common Ancestor matrix of each MC Particle in the given list"""

    def __init__(
        self,
        particle_lists,
        features,
        b_parent_var,
        mcparticle_list,
        output_file,
        save_secondaries=False,
        LCAS=False,
        bkg_prob=0.0,
    ):
        """Class Constructor.

        Args:
            particle_lists (list): Name of particle lists to save features of
            features (list): List of features to save for each particle
            b_parent_var (str): Name of variable used to flag ancestor B meson and split particles
            output_file (str): Path to output file to save
            mcparticle_list (str): Name of particle list to build LCAs from (will use as root)
            output_file (str): Path to output file to save
            save_secondaries (bool): Whether to save secondaries in the LCA, this is NOT recommended as it triples the LCA size
        """
        super().__init__()
        self.particle_lists = particle_lists
        self.features = features
        self.b_parent_var = b_parent_var
        self.mcparticle_list = mcparticle_list
        self.output_file = output_file
        self.save_secondaries = save_secondaries
        self.LCAS = LCAS
        self.bkg_probability = bkg_prob

        # Set a max num particles, it doesn't actually matter what this is as long as it's bigger than
        # any events we'll encounter. ROOT won't save all entries
        # And yeah it's dumb but that's because root fucking sucks
        self.max_particles = 500

        """ (James: not sure if should do) delete output file if it already exists, since we will apend later """

    def initialize(self):
        """Create a member to access event info StoreArray"""
        self.eventinfo = Belle2.PyStoreObj("EventMetaData")

        # Create the output file, fails if exists
        # self.h5_outfile = h5py.File(self.output_file, 'w-')
        self.root_outfile = root.TFile(self.output_file, "recreate")
        self.tree = root.TTree("Tree", "tree")

        # Reader's note: Root is weird in that we fill the same object with new values and call
        # Fill(), then it copies the values into the next entry (Leaf?) of the Branch

        # General data
        self.event_num = np.zeros(1, dtype=np.int32)
        self.tree.Branch("event", self.event_num, "event/I")

        # We use a placeholder to point each Branch to initially, we'll use the n_xxx for vectors to tell ROOT
        # how many entries in the array to save via the xxx[n_xxx] string

        # LCA data
        # Note we assume one Upsilon(4S) per event (reasonable)
        self.truth_dict = {}

        self.truth_dict["isUps"] = np.zeros(1, dtype=np.int32)
        self.tree.Branch("isUps", self.truth_dict["isUps"], "isUps/I")
        if self.LCAS:
            self.truth_dict["intermediate_skipped"] = np.zeros(1, dtype=np.bool)
            self.tree.Branch(
                "intermediate_skipped",
                self.truth_dict["intermediate_skipped"],
                "intermediate_skipped/O",
            )

        self.truth_dict["n_LCA_leaves"] = np.zeros(1, dtype=np.int32)
        self.truth_dict["LCA_leaves"] = np.zeros(
            self.max_particles, dtype=np.int32
        )
        self.tree.Branch(
            "n_LCA_leaves",
            self.truth_dict["n_LCA_leaves"],
            "n_LCA_leaves/I",
        )
        self.tree.Branch(
            "LCA_leaves",
            self.truth_dict["LCA_leaves"],
            "LCA_leaves[n_LCA_leaves]/I",
        )

        self.truth_dict["n_LCA"] = np.zeros(1, dtype=np.int32)
        self.truth_dict["LCA"] = np.zeros(
            self.max_particles**2, dtype=np.uint8
        )
        if self.LCAS:
            self.truth_dict["LCAS"] = np.zeros(
                self.max_particles**2, dtype=np.uint8
            )
        self.tree.Branch("n_LCA", self.truth_dict["n_LCA"], "n_LCA/I")
        self.tree.Branch(
            "LCA", self.truth_dict["LCA"], "LCA[n_LCA]/b"
        )
        if self.LCAS:
            self.tree.Branch(
                "LCAS", self.truth_dict["LCAS"], "LCAS[n_LCA]/b"
            )

        # print(f'LCA dictionary initialized as {self.truth_dict}')

        # Feature data
        # Here use one number to indicate how many particles were reconstructed
        self.n_particles = np.zeros(1, dtype=np.int32)
        self.tree.Branch("n_particles", self.n_particles, "n_particles/I")

        self.primary = np.zeros(self.max_particles, dtype=np.bool)
        self.tree.Branch("primary", self.primary, "primary[n_particles]/O")

        self.leaves = np.zeros(self.max_particles, dtype=np.int32)
        self.tree.Branch("leaves", self.leaves, "leaves[n_particles]/I")

        self.b_index = np.zeros(self.max_particles, dtype=np.int32)
        self.tree.Branch("b_index", self.b_index, "b_index[n_particles]/I")

        # Note the convention to identify features
        self.feat_dict = {}
        for feat in self.features:
            self.feat_dict[feat] = np.zeros(self.max_particles, np.float32)
            self.tree.Branch(
                f"feat_{feat}", self.feat_dict[feat], f"feat_{feat}[n_particles]/F"
            )

        # True MC PDG
        self.mc_pdg = np.zeros(self.max_particles, np.float32)
        self.tree.Branch("mcPDG", self.mc_pdg, "mcPDG[n_particles]/F")

    def reset_LCA(self):
        if self.LCAS:
            self.truth_dict["intermediate_skipped"][0] *= 0
            self.truth_dict["LCAS"][: self.max_particles**2] *= 0
        self.truth_dict["n_LCA_leaves"][0] *= 0
        self.truth_dict["LCA_leaves"][: self.max_particles] *= 0
        self.truth_dict["n_LCA"][0] *= 0
        self.truth_dict["LCA"][: self.max_particles**2] *= 0

    def event(self):
        """Run every event"""

        # Get the particle list (note this is a regular Particle list, not MCParticle)
        p_list = Belle2.PyStoreObj(self.mcparticle_list)

        # Event number from EventMetaData -loaded with PyStoreObj
        self.event_num[0] = self.eventinfo.getEvent()

        # Is background flag
        bkg_event = np.random.rand(1) > (1 - self.bkg_probability)

        # ### Create the LCA
        # IMPORTANT: The ArrayIndex is 0-based.
        # mcplist contains the root particles we are to create LCAs from
        # Reset the LCA list so if only one B is there it does now carry an older version over
        self.reset_LCA()

        if p_list.getListSize() == 1:
            part = p_list.obj()[0]

            # Get the corresponding MCParticle
            mcp = part.getMCParticle()

            # Get the B flag
            self.truth_dict["isUps"][0] = int(not bkg_event)

            # Call function to write history of leaves in the tree.
            # It internally calls function update_levels to find and save the level of each particle in the tree.
            # Necessary step to build the LCA.
            if self.LCAS:
                (
                    lcas_leaf_hist,
                    lcas_levels,
                    _, _, _, _, _, _, _,
                    intermediate_skipped,
                ) = write_hist(
                    mcp,
                    {}, {}, [], {}, {}, {}, {}, {},
                    False, False, False, True,
                    save_secondaries=self.save_secondaries,
                )
            leaf_hist, levels, _, _, _, _, _, _, _ = write_hist(
                mcp,
                {}, {}, [], {}, {}, {}, {}, {},
                False,
                False,
                save_secondaries=self.save_secondaries,
            )

            lca = np.zeros([len(leaf_hist), len(leaf_hist)])
            lcas = np.zeros([len(leaf_hist), len(leaf_hist)])

            for x, y in combinations(enumerate(leaf_hist), 2):
                intersection = [
                    i for i in leaf_hist[x[1]] if i in leaf_hist[y[1]]
                ]  # Such pythonic, much order
                lca[x[0], y[0]] = levels[intersection[-1]]
                lca[y[0], x[0]] = levels[intersection[-1]]

            if self.LCAS:
                for x, y in combinations(enumerate(lcas_leaf_hist), 2):
                    lcas_intersection = [
                        i for i in lcas_leaf_hist[x[1]] if i in lcas_leaf_hist[y[1]]
                    ]  # Such pythonic, much order
                    lcas[x[0], y[0]] = lcas_levels[lcas_intersection[-1]]
                    lcas[y[0], x[0]] = lcas_levels[lcas_intersection[-1]]

            if self.LCAS:
                self.truth_dict["intermediate_skipped"][
                    0
                ] = intermediate_skipped
                self.truth_dict["LCAS"][: lcas.size] = lcas.flatten()

            self.truth_dict["n_LCA_leaves"][0] = len(leaf_hist.keys())
            self.truth_dict["LCA_leaves"][
                : len(leaf_hist.keys())
            ] = list(leaf_hist.keys())

            self.truth_dict["n_LCA"][0] = lca.size
            self.truth_dict["LCA"][: lca.size] = lca.flatten()

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
                p_list = Belle2.PyStoreObj(p_list_name)

                for particle in p_list.obj():
                    # Get the B parent index, set to -1 if particle has no MC match
                    b_index = int(particle.getExtraInfo(self.b_parent_var))
                    # Need this to reorder LCA later, returns -1 if no MC match
                    if b_index >= 0:
                        p_index = particle.getMCParticle().getArrayIndex()
                        p_primary = particle.getMCParticle().isPrimaryParticle()
                        # Save particle's PDG code
                        mc_pdg = particle.getMCParticle().getPDG()
                        # Generate random b_index if event is background
                        if bkg_event:
                            b_index = (
                                int(1 if b_index == 2 else 2)
                                if np.random.rand(1) > 0.5
                                else b_index
                            )
                    else:
                        p_index = -1
                        p_primary = False
                        mc_pdg = 0
                        # Count the unmatched particle to both side (TODO: reasonable?)

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
        """Called once after all the processing is complete"""
        # self.h5_outfile.close()
        self.root_outfile.Write()
        self.root_outfile.Close()
