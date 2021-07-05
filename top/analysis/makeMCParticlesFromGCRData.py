#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# --------------------------------------------------------------------------------
# Make MCParticles from reconstructed cosmic tracks and write to basf2 root file
# This file can be used as input to simulation, see examples/simGCRData.py
# --------------------------------------------------------------------------------

import basf2 as b2
from rawdata import add_unpackers
from reconstruction import add_cosmics_reconstruction
from ROOT import Belle2
import math


class MakeMCParticles(b2.Module):
    ''' make MCParticles from reconstructed cosmic tracks '''

    def initialize(self):
        ''' initialization '''

        mcParticles = Belle2.PyStoreArray('MCParticles')
        mcParticles.registerInDataStore()

    def event(self):
        ''' event processing '''

        mcParticles = Belle2.PyStoreArray('MCParticles')
        tracks = Belle2.PyStoreArray('Tracks')
        absPDG = abs(Belle2.Const.muon.getPDGCode())
        mass = Belle2.Const.muon.getMass()
        for track in tracks:
            extHits = track.getRelationsWith('ExtHits')
            selExtHits = []
            for extHit in extHits:
                if extHit.getDetectorID() != Belle2.Const.TOP:
                    continue
                if abs(extHit.getPdgCode()) != absPDG:
                    continue
                if extHit.getPosition().Perp() < 123.5:
                    continue
                if extHit.getPosition() * extHit.getMomentum() > 0:
                    continue
                selExtHits.append(extHit)
            if len(selExtHits) == 0:
                continue
            sortedExtHits = sorted(selExtHits, key=lambda x: (x.getTOF()))
            extHit = sortedExtHits[0]
            pos = extHit.getPosition()
            mom = extHit.getMomentum()
            tof = extHit.getTOF()
            part = mcParticles.appendNew()
            part.setPDG(extHit.getPdgCode())
            part.setMass(mass)
            part.setProductionVertex(pos)
            part.setProductionTime(tof)
            part.setMomentum(mom)
            part.setEnergy(math.sqrt(mom.Mag2() + mass**2))
            part.setValidVertex(True)
            part.setStatus(Belle2.MCParticle.c_PrimaryParticle)
            part.addStatus(Belle2.MCParticle.c_StableInGenerator)

        if mcParticles.getEntries() > 0:
            self.return_value(1)
        else:
            self.return_value(0)


# Create path
main = b2.create_path()
emptypath = b2.create_path()

# input
roinput = b2.register_module('RootInput')
# roinput = register_module('SeqRootInput')
main.add_module(roinput)

# geometry parameters
gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)

# Geometry
geometry = b2.register_module('Geometry')
main.add_module(geometry)

# unpackers
add_unpackers(main, components=['CDC'])

# reconstruction
add_cosmics_reconstruction(main, components=['CDC'], merge_tracks=True)

# make MCParticles from reconstructed tracks
maker = MakeMCParticles()
main.add_module(maker)
maker.if_false(emptypath)

# output
output = b2.register_module('RootOutput')
output.param('branchNames', ['MCParticles'])
main.add_module(output)

# Print progress
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print statistics
print(b2.statistics)
