#!/usr/bin/env python
# -*- coding: utf-8 -*-
from basf2 import *
import pdg
import os

# generate continuum in CMS with a specified center of mass energy

# specify cms energy in GeV
cms_energy = 10.58
# to get a specific center of mass we can use pdg values from the framework,
# e.g. to generate at the Z we can use
# cms_energy = pdg.get("Z0").Mass()

# specify cms energy spread. Resulting events should have a normal distributed
# CMS energy with sigma equal to this spread. This also means that the primary
# particle is not totally at rest because both beams are smeared independently.
# So for true CMS set this value to zero
cms_energy_spread = 0.00

# calculate values for each beam to get correct cms
beam_energy = str(cms_energy / 2.)
beam_error = str(cms_energy_spread / 2 ** .5)

# suppress messages and during processing:
set_log_level(LogLevel.WARNING)

# create path
main = create_path()

# add event info for generated events, we want to generate 3 events
main.add_module("EventInfoSetter", evtNumList=[3])

# add gearbox to load the beam parameters but override the settings
main.add_module("Gearbox", override=[
    # set the beam energy for both beams
    ("//SuperKEKB/HER/energy", beam_energy, "GeV"),
    ("//SuperKEKB/LER/energy", beam_energy, "GeV"),
    # set the beam energy spread for both beams
    ("//SuperKEKB/HER/energyError", beam_error, "GeV"),
    ("//SuperKEKB/LER/energyError", beam_error, "GeV"),
    # remove the crossing angle
    ("//SuperKEKB/HER/angle", "0", "mrad"),
    ("//SuperKEKB/LER/angle", "0", "mrad"),
])

# add EvtGen and set parent particle to virtual photon and the given decay file
userDECFile = os.path.join(os.environ['BELLE2_LOCAL_DIR'], 'generators/evtgen/decayfiles/ccbar+Dst.dec')
# userDECFile = os.path.join(os.environ['BELLE2_LOCAL_DIR'], 'generators/evtgen/decayfiles/uubar.dec')
main.add_module("EvtGenInput", ParentParticle="vpho", userDECFile=userDECFile)

# print generated particles and set it's log level to info so that we see the
# generated mc particle list
main.add_module('PrintMCParticles', logLevel=LogLevel.INFO)

# process the events
process(main)
