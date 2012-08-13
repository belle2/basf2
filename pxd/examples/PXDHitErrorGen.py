#!/usr/bin/env python
# -*- coding: utf-8 -*-

# -*- coding: utf-8 -*-

import sys
import math
from basf2 import *
logging.log_level = LogLevel.WARNING

# Some ROOT tools
import ROOT
from ROOT import Belle2


class PXDHitErrors(Module):

    """A simple module to check the reconstruction of PXDTrueHits."""

    def __init__(self):
        """Initialize the module"""

        super(PXDHitErrors, self).__init__()
        self.setName('PXDHitErrors')
        self.file = open('PXDHitErrorOutput.txt', 'w')
        self.vxdid_factors = (8192, 256, 32)  # For decoding VxdID's

    def beginRun(self):
        """ Do nothing """

    def event(self):
        """Find clusters with a truehit and print some stats."""

        truehits = Belle2.PyStoreArray('PXDTrueHits')
        nTruehits = truehits.getEntries()
        clusters = Belle2.PyStoreArray('PXDClusters')
        nClusters = clusters.getEntries()
        digits = Belle2.PyStoreArray('PXDDigits')
        nDigits = digits.getEntries()
        relClustersToTrueHits = \
            Belle2.PyRelationArray('PXDClustersToPXDTrueHits')
        nClusterRelations = relClustersToTrueHits.getEntries()
        relClustersToDigits = Belle2.PyRelationArray('PXDClustersToPXDDigits')
        nDigitRelations = relClustersToDigits.getEntries()

        # Write legend for file columns
        self.file.write('LEGEND TO COLUMNS: \n')
        self.file.write('SensorID Layer Ladder Sensor Truehit_index Cluster_index \n'
                        )
        self.file.write('TrueHit: u[cm], v[cm], charge[GeV], theta_u, theta_v \n'
                        )
        self.file.write('Cluster: u[cm], v[cm], charge[e-], size, size_u, size_v \n'
                        )
        self.file.write('Digits: n_digits {u[cm] v[cm] charge[e-]} \n')
        self.file.write('\n')

        # Start with the clusters and use the relation to get the corresponding digits and truehits.
        for cluster_index in range(nClusters):
            cluster = clusters[cluster_index]
            cluster_truehits = \
                relClustersToTrueHits.getToIndices(cluster_index)
            # FIXME: There is a problem with clusters having more than 1 TrueHit. Skipping for now.
            if len(cluster_truehits) > 1:
                continue

            for truehit_index in cluster_truehits:
                truehit = truehits[truehit_index]
                # Now let's store some data
                s = ''
                # Sesnor identification
                sensorID = truehit.getRawSensorID()
                [layer, ladder, sensor] = self.decode(sensorID)
                s_id = \
                    '{sID} {layer} {ladder} {sensor} {indexTH:4d} {indexCL:4d} '.format(
                    sID=sensorID,
                    layer=layer,
                    ladder=ladder,
                    sensor=sensor,
                    indexTH=truehit_index,
                    indexCL=cluster_index,
                    )
                s += s_id
                # TrueHit information
                thetaU = math.atan2(truehit.getExitU() - truehit.getEntryU(),
                                    0.0075)
                thetaV = math.atan2(truehit.getExitV() - truehit.getEntryV(),
                                    0.0075)
                s_th = \
                    '{uTH:10.5f} {vTH:10.5f} {eTH:10.7f} {thetaU:6.3f} {thetaV:6.3f} '.format(uTH=truehit.getU(),
                        vTH=truehit.getV(), eTH=truehit.getEnergyDep(),
                        thetaU=thetaU, thetaV=thetaV)
                s += s_th
                # Cluster information
                s_cl = \
                    '{uCL:10.5f} {vCL:10.5f} {eCL:10.1f} {eSeed:10.1f} {size:5d} {sizeU:5d} {sizeV:5d} '.format(
                    uCL=cluster.getU(),
                    vCL=cluster.getV(),
                    eCL=cluster.getCharge(),
                    eSeed=cluster.getSeedCharge(),
                    size=cluster.getSize(),
                    sizeU=cluster.getUSize(),
                    sizeV=cluster.getVSize(),
                    )
                s += s_cl
                # We can add some digits, too.
                digit_indices = relClustersToDigits.getToIndices(cluster_index)
                s_dig0 = '{nDigits:4d} '.format(nDigits=len(digit_indices))
                s += s_dig0
                for digit_index in digit_indices:
                    digit = digits[digit_index]
                    s_dig = \
                        '{u:10.5f} {v:10.5f} {e:10.1f} '.format(u=digit.getUCellPosition(),
                            v=digit.getVCellPosition(), e=digit.getCharge())
                    s += s_dig

                s += '\n'
                self.file.write(s)

    def terminate(self):
        """ Close the output file."""

        self.file.close()

    def decode(self, vxdid):
        """ Utility to decode sensor IDs """

        result = []
        for f in self.vxdid_factors:
            result.append(vxdid / f)
            vxdid = vxdid % f

        return result


# Particle gun module
particlegun = register_module('ParticleGun')
# Create Event information
evtmetagen = register_module('EvtMetaGen')
# Show progress of processing
progress = register_module('Progress')
# Load parameters
gearbox = register_module('Gearbox')
# Create geometry
geometry = register_module('Geometry')
# Run simulation
simulation = register_module('FullSim')
# PXD digitization module
pxddigi = register_module('PXDDigitizer')
# PXD clustering module
pxdclust = register_module('PXDClusterizer')
# Simpleoutput
output = register_module('SimpleOutput')

analyze = PXDHitErrors()

# Specify number of events to generate
evtmetagen.param({'EvtNumList': [10], 'RunList': [1]})

# Set parameters for particlegun
particlegun.param({  # Generate 5 tracks
                     # But vary the number of tracks according to Poisson distribution
                     # Generate pi+, pi-, e+ and e-
                     # with a normal distributed transversal momentum
                     # with a center of 5 GeV and a width of 1 GeV
                     # a normal distributed phi angle,
                     # center of 180 degree and a width of 30 degree
                     # Generate theta angles uniform in cos theta
                     # between 17 and 150 degree
                     # normal distributed vertex generation
                     # around the origin with a sigma of 2cm in the xy plane
                     # and no deviation in z
                     # all tracks sharing the same vertex per event
    'nTracks': 10,
    'varyNTracks': True,
    'pdgCodes': [211, -211, 11, -11],
    'momentumGeneration': 'normal',
    'momentumParams': [5, 0.2],
    'phiGeneration': 'uniform',
    'phiParams': [-5, 5],
    'thetaGeneration': 'uniformCosinus',
    'thetaParams': [0.05, 0.2],
    'vertexGeneration': 'normal',
    'xVertexParams': [1.5, 0.01],
    'yVertexParams': [0, 0.1],
    'zVertexParams': [-10, 0.1],
    'independentVertices': True,
    })

# Select subdetectors to be built
geometry.param('Components', ['PXD'])

pxddigi.param('statisticsFilename', 'digi.root')
pxddigi.param('ElectronicEffects', True)
pxddigi.param('SimpleDriftModel', False)

# create processing path
main = create_path()
main.add_module(evtmetagen)
main.add_module(progress)
main.add_module(particlegun)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(simulation)
main.add_module(pxddigi)
main.add_module(pxdclust)
main.add_module(analyze)
main.add_module(output)

# generate events
process(main)

# show call statistics
print statistics

