#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Generate tau pair events using KKMC generator &
collects invariant mass distributions for all possible pairs of the visible final state particles.

Execute script with:
  $> basf2 printTauDecayMode.py -n 100
"""

import basf2 as b2
from beamparameters import add_beamparameters
import generators as ge
import ROOT
from ROOT import Belle2
from basf2 import Module
import numpy as np

# Configuration
# b2.set_random_seed(12345)
b2.set_log_level(b2.LogLevel.INFO)

# Create main processing path
main = b2.create_path()

# Event information: generate 1000 events in run 1
main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=1000)

# Beam parameters for Y4S resonance
beamparameters = add_beamparameters(main, "Y4S")

# Tau decay file for TAUOLA
decfile = b2.find_file('./generators/examples/TauolaBelle2/tauola_bbb.pigamma_112_261.dat')

# ROOT batch mode (suppress graphical output)
ROOT.gROOT.SetBatch(True)


class CalculateInvMassesPairwise(Module):
    """Module to calculate invariant mass distributions"""
    """This module assumes you'll use this script for up to 7 visible particles"""

    # 2-body combinations
    kPairs2Body = [
        (0, 1), (0, 2), (0, 3), (0, 4), (0, 5), (0, 6),
        (1, 2), (1, 3), (1, 4), (1, 5), (1, 6),
        (2, 3), (2, 4), (2, 5), (2, 6),
        (3, 4), (3, 5), (3, 6),
        (4, 5), (4, 6),
        (5, 6)
    ]

    # 3-body combinations
    kTriplets3Body = [
        (0, 1, 2), (0, 1, 3), (0, 1, 4), (0, 1, 5), (0, 1, 6),
        (0, 2, 3), (0, 2, 4), (0, 2, 5), (0, 2, 6),
        (0, 3, 4), (0, 3, 5), (0, 3, 6),
        (0, 4, 5), (0, 4, 6), (0, 5, 6),
        (1, 2, 3), (1, 2, 4), (1, 2, 5), (1, 2, 6),
        (1, 3, 4), (1, 3, 5), (1, 3, 6),
        (1, 4, 5), (1, 4, 6), (1, 5, 6),
        (2, 3, 4), (2, 3, 5), (2, 3, 6),
        (2, 4, 5), (2, 4, 6), (2, 5, 6),
        (3, 4, 5), (3, 4, 6), (3, 5, 6),
        (4, 5, 6)
    ]

    # 4-body combinations
    kQuads4Body = [
        (0, 1, 2, 3), (0, 1, 2, 4), (0, 1, 2, 5), (0, 1, 2, 6),
        (0, 1, 3, 4), (0, 1, 3, 5), (0, 1, 3, 6),
        (0, 1, 4, 5), (0, 1, 4, 6), (0, 1, 5, 6),
        (0, 2, 3, 4), (0, 2, 3, 5), (0, 2, 3, 6),
        (0, 2, 4, 5), (0, 2, 4, 6), (0, 2, 5, 6),
        (0, 3, 4, 5), (0, 3, 4, 6), (0, 3, 5, 6),
        (0, 4, 5, 6),
        (1, 2, 3, 4), (1, 2, 3, 5), (1, 2, 3, 6),
        (1, 2, 4, 5), (1, 2, 4, 6), (1, 2, 5, 6),
        (1, 3, 4, 5), (1, 3, 4, 6), (1, 3, 5, 6),
        (1, 4, 5, 6),
        (2, 3, 4, 5), (2, 3, 4, 6), (2, 3, 5, 6),
        (2, 4, 5, 6),
        (3, 4, 5, 6)
    ]

    num_events = 0
    deb_events = 0

    def __init__(self, output_file='taupair_invariant_masses.root'):
        super().__init__()
        self.m_compute2Body = True
        self.m_compute3Body = True
        self.m_compute4Body = True
        self.output_file = output_file
        self._root_file = None
        self._tree = None
        self._branches = {}

    def initialize(self):
        self._root_file = ROOT.TFile(self.output_file, 'RECREATE')
        self._tree = ROOT.TTree('TauPairMasses', 'Tau pair invariant masses and PDGs')

        def book_float(name):
            self._branches[name] = np.zeros(1, dtype=np.float64)
            self._tree.Branch(name, self._branches[name], f'{name}/D')

        def book_int(name):
            self._branches[name] = np.zeros(1, dtype=np.int32)
            self._tree.Branch(name, self._branches[name], f'{name}/I')

        for prefix in ('minus', 'plus'):
            book_int(f'{prefix}_mc_mode')
            for i in range(len(self.kPairs2Body)):
                book_float(f'{prefix}_mass2_{i + 1}')
                book_int(f'{prefix}_mass2_{i + 1}_pdg1')
                book_int(f'{prefix}_mass2_{i + 1}_pdg2')
            for i in range(len(self.kTriplets3Body)):
                book_float(f'{prefix}_mass3_{i + 1}')
                book_int(f'{prefix}_mass3_{i + 1}_pdg1')
                book_int(f'{prefix}_mass3_{i + 1}_pdg2')
                book_int(f'{prefix}_mass3_{i + 1}_pdg3')
            for i in range(len(self.kQuads4Body)):
                book_float(f'{prefix}_mass4_{i + 1}')
                book_int(f'{prefix}_mass4_{i + 1}_pdg1')
                book_int(f'{prefix}_mass4_{i + 1}_pdg2')
                book_int(f'{prefix}_mass4_{i + 1}_pdg3')
                book_int(f'{prefix}_mass4_{i + 1}_pdg4')

    def event(self):
        self.num_events += 1
        """Process each event to extract relevant information"""
        taupairdecay = Belle2.PyStoreObj("TauPairDecay")
        mcparticles = Belle2.PyStoreArray('MCParticles')

        tauMinusMCMode = taupairdecay.getTauMinusIdMode()
        tauPlusMCMode = taupairdecay.getTauPlusIdMode()
        self._set_branch('minus_mc_mode', tauMinusMCMode)
        self._set_branch('plus_mc_mode', tauPlusMCMode)

        dau_tauMinus = taupairdecay.getTauMinusDaughters()
        dau_tauPlus = taupairdecay.getTauPlusDaughters()
        # compute visible (non-neutrino) daughter indices for mass calculations
        vis_minus = self.visible_for_masses(dau_tauMinus, mcparticles)
        vis_plus = self.visible_for_masses(dau_tauPlus, mcparticles)

        if self.m_compute2Body or self.m_compute3Body or self.m_compute4Body:
            self.compute_and_store_invariant_masses(vis_minus, mcparticles, True)
            self.compute_and_store_invariant_masses(vis_plus, mcparticles, False)
        else:
            self.store_no_invariant_masses()

        self.fill_tree(vis_minus, vis_plus, mcparticles)

    def terminate(self):
        if self._root_file is not None:
            self._root_file.cd()
            if self._tree is not None:
                self._tree.Write()
            self._root_file.Close()

    def _set_branch(self, name, value):
        self._branches[name][0] = value

    def _pdgs_for_combo(self, visible_particles, mcparticles, combo):
        pdgs = []
        for i in combo:
            try:
                pdgs.append(int(mcparticles[visible_particles[i]].getPDG()))
            except Exception:
                pdgs.append(0)
        return pdgs

    def fill_tree(self, vis_minus, vis_plus, mcparticles):
        for prefix, visible in (('minus', vis_minus), ('plus', vis_plus)):
            for i, combo in enumerate(self.kPairs2Body):
                name = f'{prefix}_mass2_{i + 1}'
                mass = -1.0
                pdgs = [-1, -1]
                if self.m_compute2Body and len(visible) >= 2 and max(combo) < len(visible):
                    mass = self.compute_n_body_masses(visible, mcparticles, [combo])[0]
                    pdgs = self._pdgs_for_combo(visible, mcparticles, combo)
                self._set_branch(name, mass)
                self._set_branch(f'{name}_pdg1', pdgs[0])
                self._set_branch(f'{name}_pdg2', pdgs[1])

            for i, combo in enumerate(self.kTriplets3Body):
                name = f'{prefix}_mass3_{i + 1}'
                mass = -1.0
                pdgs = [-1, -1, -1]
                if self.m_compute3Body and len(visible) >= 3 and max(combo) < len(visible):
                    mass = self.compute_n_body_masses(visible, mcparticles, [combo])[0]
                    pdgs = self._pdgs_for_combo(visible, mcparticles, combo)
                self._set_branch(name, mass)
                self._set_branch(f'{name}_pdg1', pdgs[0])
                self._set_branch(f'{name}_pdg2', pdgs[1])
                self._set_branch(f'{name}_pdg3', pdgs[2])

            for i, combo in enumerate(self.kQuads4Body):
                name = f'{prefix}_mass4_{i + 1}'
                mass = -1.0
                pdgs = [-1, -1, -1, -1]
                if self.m_compute4Body and len(visible) >= 4 and max(combo) < len(visible):
                    mass = self.compute_n_body_masses(visible, mcparticles, [combo])[0]
                    pdgs = self._pdgs_for_combo(visible, mcparticles, combo)
                self._set_branch(name, mass)
                self._set_branch(f'{name}_pdg1', pdgs[0])
                self._set_branch(f'{name}_pdg2', pdgs[1])
                self._set_branch(f'{name}_pdg3', pdgs[2])
                self._set_branch(f'{name}_pdg4', pdgs[3])

        if self._tree is not None:
            self._tree.Fill()

    def visible_for_masses(self, tauDaughters, mcparticles):
        """Return list of daughter indices excluding neutrinos (pdg 12,14,16).

        tauDaughters: iterable of indices
        mcparticles: Belle2.PyStoreArray of MCParticles
        """
        visibleForMasses = []
        for idx in tauDaughters:
            try:
                pdgid = abs(mcparticles[idx].getPDG())
            except Exception:
                # if access fails, skip
                continue
            if pdgid in (12, 14, 16):
                continue
            visibleForMasses.append(idx)
        # print detailed PDG information for the selected visible daughters
        try:
            if len(visibleForMasses) == 0:
                print('TauDecayMode::   particle PDG: (none)')
            else:
                parts = []
                for i in range(len(visibleForMasses)):
                    try:
                        pdg = mcparticles[visibleForMasses[i]].getPDG()
                    except Exception:
                        pdg = 'err'
                    parts.append(f'[{i}]={pdg}')
                print('TauDecayMode::   particle PDG: ' + ', '.join(parts))
        except Exception:
            # be defensive: do not break processing if printing fails
            pass

        return visibleForMasses

    def calculate_invariant_mass(self, E, px, py, pz):
        m2 = E * E - (px * px + py * py + pz * pz)
        return float(np.sqrt(max(m2, 0.0)))

    def compute_n_body_masses(self, visible_particles, mcparticles, combinations):
        masses = [0.0] * len(combinations)
        n = len(visible_particles)
        for slot, combo in enumerate(combinations):
            if any(i >= n for i in combo):
                continue

            E = 0.0
            px = 0.0
            py = 0.0
            pz = 0.0
            valid = True
            for i in combo:
                try:
                    p = mcparticles[visible_particles[i]]
                    mom = p.getMomentum()
                    E += p.getEnergy()
                    px += mom.X()
                    py += mom.Y()
                    pz += mom.Z()
                except Exception:
                    valid = False
                    break

            if valid:
                masses[slot] = self.calculate_invariant_mass(E, px, py, pz)

        return masses

    def compute2BodyMasses(self, visible_particles, mcparticles):
        if len(visible_particles) < 2:
            return [0.0] * len(self.kPairs2Body)
        return self.compute_n_body_masses(visible_particles, mcparticles, self.kPairs2Body)

    def compute3BodyMasses(self, visible_particles, mcparticles):
        if len(visible_particles) < 3:
            return [0.0] * len(self.kTriplets3Body)
        return self.compute_n_body_masses(visible_particles, mcparticles, self.kTriplets3Body)

    def compute4BodyMasses(self, visible_particles, mcparticles):
        if len(visible_particles) < 4:
            return [0.0] * len(self.kQuads4Body)
        return self.compute_n_body_masses(visible_particles, mcparticles, self.kQuads4Body)

    def compute_and_store_invariant_masses(self, visible_particles, mcparticles, is_minus):
        out = {}

        if self.m_compute2Body:
            masses2 = self.compute2BodyMasses(visible_particles, mcparticles)
            out['2body'] = masses2

        if self.m_compute3Body:
            masses3 = self.compute3BodyMasses(visible_particles, mcparticles)
            out['3body'] = masses3

        if self.m_compute4Body:
            masses4 = self.compute4BodyMasses(visible_particles, mcparticles)
            out['4body'] = masses4

        return out

    def store_no_invariant_masses(self):
        return None


# run
main.add_module("Progress")
# use KKMC to generate taupair events
ge.add_kkmc_generator(path=main, finalstate='tau-tau+', signalconfigfile=decfile)

# main.add_module("PrintMCParticles", logLevel=b2.LogLevel.INFO, onlyPrimaries=False, showMomenta=True, showStatus=True)
main.add_module("TauDecayMode", file_minus=b2.find_file('data/analysis/modules/TauDecayMode/map_tauminus.txt'),
                file_plus=b2.find_file('data/analysis/modules/TauDecayMode/map_tauplus.txt'), printmode="all")
TauInvMassModule = CalculateInvMassesPairwise(output_file='kkmc_tautau_invariant_masses_example.root')
main.add_module(TauInvMassModule)

main.add_module("RootOutput", outputFileName='kkmc_tautau_bbb_example.root')
# main.add_module("HepMCOutput", OutputFilename=f'kkmc_tautau_bbb_{tag}.hepmc', StoreVirtualParticles=True)

# generate events
b2.process(main)

# show call statistics
print(b2.statistics)
