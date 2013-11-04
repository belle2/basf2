#!/usr/bin/env python
# -*- coding: utf-8 -*-

# TestBeam 2009 Simulation & Analysis
# This is the second simulation scenario with 120 GeV/c pions and Belle 2
# 50x50 um DUT

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
        ## Output file object
        self.file = open('PXDHitErrorOutput.txt', 'w')
        ## Factors for decoding of VxdId's
        self.vxdid_factors = (8192, 256, 32)

    def beginRun(self):
        """ Write legend """

        self.file.write('LEGEND TO COLUMNS: \n')
        self.file.write('SensorID Layer Ladder Sensor Truehit_index '
                        + 'Cluster_index \n')
        self.file.write('TrueHit: u[cm], v[cm], charge[GeV], theta_u, '
                        + 'theta_v \n')
        self.file.write('Cluster: u[cm], v[cm], charge[e-], '
                        + 'seed charge[e-], size, size_u, size_v \n')
        self.file.write('Digits: n_digits {u[cm] v[cm] charge[e-]} \n')
        self.file.write('\n')

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

        # Start with clusters and use the relation to get the corresponding
        # digits and truehits.
        for cluster_index in range(nClusters):
            cluster = clusters[cluster_index]
            cluster_truehits = \
                relClustersToTrueHits.getToIndices(cluster_index)
            # FIXME: There is a problem with clusters having more than 1
            # TrueHit. Skipping for now.
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
                    '{sID} {layer} {ladder} {sensor} {indexT:4d} {indexC:4d} '\
                    .format(
                    sID=sensorID,
                    layer=layer,
                    ladder=ladder,
                    sensor=sensor,
                    indexT=truehit_index,
                    indexC=cluster_index,
                    )
                s += s_id
                # TrueHit information
                thetaU = math.atan2(truehit.getExitU() - truehit.getEntryU(),
                                    0.0075)
                thetaV = math.atan2(truehit.getExitV() - truehit.getEntryV(),
                                    0.0075)
                s_th = \
                    '{uTH:10.5f} {vTH:10.5f} {eTH:10.7f} '.format(
                        uTH=truehit.getU(),
                        vTH=truehit.getV(), eTH=truehit.getEnergyDep()) \
                    + '{thetaU:6.3f} {thetaV:6.3f} '.format(thetaU=thetaU,
                        thetaV=thetaV)
                s += s_th
                # Cluster information
                s_cl = \
                    '{uCL:10.5f} {vCL:10.5f} {eCL:10.1f} {eSeed:10.1f} '\
                    .format(uCL=cluster.getU(), vCL=cluster.getV(),
                        eCL=cluster.getCharge(),
                        eSeed=cluster.getSeedCharge()) \
                    + '{size:5d} {sizeU:5d} {sizeV:5d} '.format(
                        size=cluster.getSize(),
                        sizeU=cluster.getUSize(),
                        sizeV=cluster.getVSize())
                s += s_cl
                # We can add some digits, too.
                digit_indices = relClustersToDigits.getToIndices(cluster_index)
                s_dig0 = '{nDigits:4d} '.format(nDigits=len(digit_indices))
                s += s_dig0
                for digit_index in digit_indices:
                    digit = digits[digit_index]
                    s_dig = \
                        '{u:10.5f} {v:10.5f} {e:10.1f} '.format(
                            u=digit.getUCellPosition(),
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


# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)
# ParticleGun
particlegun = register_module('ParticleGun')
# pions:
particlegun.param('pdgCodes', [211, -211])
# number of primaries per event
particlegun.param('nTracks', 1)
# momentum 120 GeV/c
particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [120, 120])
# sensors in the geometry are placed into a beamline - at theta=0, phi=0-360
# in Belle2 coordinate system
particlegun.param('thetaGeneration', 'normal')
particlegun.param('thetaParams', [0.0, 0.1])
particlegun.param('phiGeneration', 'normal')
particlegun.param('phiParams', [0.0, 360])
# gun displacement according to the area of DUT (detector under test -
# sensor_id=3) times 0.8
particlegun.param('vertexGeneration', 'uniform')
particlegun.param('xVertexParams', [-0.256 * 0.8, 0.256 * 0.8])  # x OK
particlegun.param('yVertexParams', [-0.064 * 0.8, 0.064 * 0.8])  # y OK
particlegun.param('zVertexParams', [-1.0, 0.0])
particlegun.param('independentVertices', True)

# Create Event information
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [100], 'runList': [1]})

# Show progress of processing
progress = register_module('Progress')
# Load parameters from xml - load the geometry file which uses 50x50 um matrix
gearbox = register_module('Gearbox')
gearbox.param('fileName', 'pxd/testbeam/TBBelle2-Geometry.xml')

# Create geometry
geometry = register_module('Geometry')
# Run simulation
simulation = register_module('FullSim')
# Uncomment following lines to get particle tracks visualization
# simulation.param('EnableVisualization', True)
# simulation.param('UICommands', ['/vis/open VRML2FILE', '/vis/drawVolume',
#                 '/vis/scene/add/axes 0 0 0 100 mm',
#                 '/vis/scene/add/trajectories smooth',
#                 '/vis/modeling/trajectories/create/drawByCharge'])

# Add PXD Digitizer
digit = register_module('PXDDigitizer')
digit.param('SimpleDriftModel', False)
digit.param('statisticsFilename', 'TB2009PXDDigiStat.root')

# Add PXD Clusterizer
cluster = register_module('PXDClusterizer')
# Turn off Magnetic field effets assumption during clustering
cluster.param('TanLorentz', 0.0)
cluster.param('AssumeSorted', False)

# Save output of simulation
output = register_module('RootOutput')
output.param('outputFileName', 'TBSimulationOutput.root')

geosaver = register_module('ExportGeometry')
geosaver.param('Filename', 'TBGeometry.root')

analyze = PXDHitErrors()

# Path construction
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(geosaver)
main.add_module(particlegun)
main.add_module(simulation)
main.add_module(digit)
main.add_module(cluster)
main.add_module(analyze)
main.add_module(output)

# Process events
process(main)

# Print call statistics
print statistics
