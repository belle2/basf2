#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>PhokharaEvtgenData.root</input>
    <output>PhokharaEvtgenAnalysis.root</output>
    <contact>Kirill Chilikin (chilikin@lebedev.ru)</contact>
    <description>Analysis of e+ e- -> J/psi eta_c events.</description>
</header>
"""

from basf2 import *
import ROOT
import numpy
from ROOT import Belle2


class PhokharaEvtgenAnalysisModule(Module):
    """ Analysis module for PhokharaEvtgen. """

    def __init__(self):
        """Initialization."""
        super(PhokharaEvtgenAnalysisModule, self).__init__()
        self.output_file = ROOT.TFile('PhokharaEvtgenAnalysis.root', 'recreate')
        self.tree = ROOT.TTree('tree', '')
        self.ecms = numpy.zeros(1, dtype=numpy.float32)
        self.gamma_e = numpy.zeros(1, dtype=numpy.float32)
        self.gamma_px = numpy.zeros(1, dtype=numpy.float32)
        self.gamma_py = numpy.zeros(1, dtype=numpy.float32)
        self.gamma_pz = numpy.zeros(1, dtype=numpy.float32)
        self.jpsi_e = numpy.zeros(1, dtype=numpy.float32)
        self.jpsi_px = numpy.zeros(1, dtype=numpy.float32)
        self.jpsi_py = numpy.zeros(1, dtype=numpy.float32)
        self.jpsi_pz = numpy.zeros(1, dtype=numpy.float32)
        self.lepton_e = numpy.zeros(1, dtype=numpy.float32)
        self.lepton_px = numpy.zeros(1, dtype=numpy.float32)
        self.lepton_py = numpy.zeros(1, dtype=numpy.float32)
        self.lepton_pz = numpy.zeros(1, dtype=numpy.float32)
        self.tree.Branch('ecms', self.ecms, 'ecms/F')
        self.tree.Branch('gamma_e', self.gamma_e, 'gamma_e/F')
        self.tree.Branch('gamma_px', self.gamma_px, 'gamma_px/F')
        self.tree.Branch('gamma_py', self.gamma_py, 'gamma_py/F')
        self.tree.Branch('gamma_pz', self.gamma_pz, 'gamma_pz/F')
        self.tree.Branch('jpsi_e', self.jpsi_e, 'jpsi_e/F')
        self.tree.Branch('jpsi_px', self.jpsi_px, 'jpsi_px/F')
        self.tree.Branch('jpsi_py', self.jpsi_py, 'jpsi_py/F')
        self.tree.Branch('jpsi_pz', self.jpsi_pz, 'jpsi_pz/F')
        self.tree.Branch('lepton_e', self.lepton_e, 'lepton_e/F')
        self.tree.Branch('lepton_px', self.lepton_px, 'lepton_px/F')
        self.tree.Branch('lepton_py', self.lepton_py, 'lepton_py/F')
        self.tree.Branch('lepton_pz', self.lepton_pz, 'lepton_pz/F')

    def event(self):
        """ Event function. """
        mc_initial_particles = Belle2.PyStoreObj('MCInitialParticles')
        self.ecms[0] = mc_initial_particles.getMass()
        mc_particles = Belle2.PyStoreArray('MCParticles')
        for mc_particle in mc_particles:
            # Select virtual photons.
            if (mc_particle.getPDG() != 10022):
                continue
            p = mc_particle.getMomentum()
            self.gamma_e[0] = mc_particle.getEnergy()
            self.gamma_px[0] = p.X()
            self.gamma_py[0] = p.Y()
            self.gamma_pz[0] = p.Z()
            jpsi = mc_particle.getDaughters()[0]
            p = jpsi.getMomentum()
            self.jpsi_e[0] = jpsi.getEnergy()
            self.jpsi_px[0] = p.X()
            self.jpsi_py[0] = p.Y()
            self.jpsi_pz[0] = p.Z()
            lepton = jpsi.getDaughters()[0]
            p = lepton.getMomentum()
            self.lepton_e[0] = lepton.getEnergy()
            self.lepton_px[0] = p.X()
            self.lepton_py[0] = p.Y()
            self.lepton_pz[0] = p.Z()
            self.tree.Fill()

    def terminate(self):
        """ Termination function. """
        self.output_file.cd()
        self.tree.Write()
        self.output_file.Close()


# Input.
root_input = register_module('RootInput')
root_input.param('inputFileName', 'PhokharaEvtgenData.root')

# Analysis.
phokhara_evtgen = PhokharaEvtgenAnalysisModule()

# Create main path.
main = create_path()

# Add modules to main path
main.add_module(root_input)
main.add_module(phokhara_evtgen)

# Run.
process(main)

print(statistics)
