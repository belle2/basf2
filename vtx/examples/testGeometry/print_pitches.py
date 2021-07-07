#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#############################################################
# Simple steering print pixel pitches of VTX sensors
#############################################################

import basf2 as b2
from simulation import add_simulation
from reconstruction import add_mc_reconstruction

import ROOT
from ROOT import Belle2

import argparse
ap = argparse.ArgumentParser()
ap.add_argument("--vtx", default="VTX-CMOS-7layer", help="VTX geometry variant from xml")
args = vars(ap.parse_args())


class VTXPrintPitches(b2.Module):
    """
    Simple module to print the pitch of the hit pixel in cm
    """

    def event(self):
        """Find clusters with a truehit and print some stats."""

        # Start with clusters and use the relation to get the corresponding
        # digits and truehits.
        for cluster in Belle2.PyStoreArray('VTXClusters'):

            # Now let's store some data
            sensor_info = Belle2.VXD.GeoCache.get(cluster.getSensorID())
            pitchU = sensor_info.getUPitch(cluster.getV())
            pitchV = sensor_info.getVPitch(cluster.getV())
            # Print the pitch obtained from the sensor info
            print('SensorLayer={} PitchU={:.5f}cm and PitchV={:.5f}cm'.format(
                str(cluster.getSensorID().getLayerNumber()), pitchU, pitchV))


# number of events to generate, can be overriden with -n
num_events = 100

# create path
main = b2.create_path()

main.add_module("EventInfoSetter", evtNumList=num_events)

main.add_module('ParticleGun', pdgCodes=[11],
                momentumGeneration="fixed", momentumParams=[5.2],
                thetaGeneration="fixed", thetaParams=[90.],
                phiGeneration="fixed", phiParams=[60.],
                # normal distributed vertex generation
                vertexGeneration='normal',
                xVertexParams=[-0.02, 0.0],
                yVertexParams=[0.28, 0.3],
                zVertexParams=[0.95, 0.3]
                )

main.add_module('Gearbox')
# remove PXD+SVD+MagneticField and add VTX
other_parts = [
    'COIL',
    'STR',
    'ServiceGapsMaterial',
    'BeamPipe',
    'Cryostat',
    'FarBeamLine',
    'HeavyMetalShield',
    'VXDService',
    'MagneticField']
main.add_module('Geometry', excludedComponents=['PXD', 'SVD', 'CDC', 'ECL', 'ARICH', 'TOP', 'KLM'] + other_parts,
                additionalComponents=[args['vtx']],
                useDB=False)

# G4 simulation
main.add_module('FullSim')

# Digitizer
main.add_module('VTXDigitizer')

# Clusturizer
main.add_module('VTXClusterizer')

# Print piches
main.add_module(VTXPrintPitches())

# process events and print call statistics
main.add_module('Progress')
b2.process(main)
print(b2.statistics)
