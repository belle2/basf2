#!/usr/bin/env python3
# requires `pip3 install --user particle`

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
from particle import Particle, ParticleNotFound
from particle.particle.particle import InvalidParticle
import pdg
from math import isclose
import basf2

pdg.load(basf2.find_file("data/framework/particledb/evt.pdl"))

not_found = []
all_pdgcodes = set()

for p in pdg.search():
    try:
        pdg = Particle.from_pdgid(p.PdgCode())
        names = ["mass", "width", "lifetime", "charge"]
        current = [p.Mass(), p.Width(), p.Lifetime(), p.Charge()/3]
        nominal = [pdg.mass/1e3 if pdg.mass is not None else 0,
                   pdg.width/1e3 if pdg.width is not None else 0,
                   pdg.lifetime/1e9 if pdg.lifetime is not None and pdg.width > 0 else 0,
                   pdg.charge]
        for n, a, b in zip(names, current, nominal):
            if not isclose(a, b, rel_tol=0.05, abs_tol=1e-12):
                print(f"{p.GetName()} difference in {n}: {a} != {b}")
    except (ParticleNotFound, InvalidParticle):
        not_found.append(p)

    all_pdgcodes.add(p.PdgCode())

print("Not found in PDG table:", ", ".join(p.GetName() for p in not_found))
print("Found in PDG table but not evt.pdl:", ", ".join(p.name for p in Particle.findall(lambda p: p.pdgid not in all_pdgcodes)))
