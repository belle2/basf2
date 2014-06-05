#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
logging.log_level = LogLevel.WARNING

# Load the required libraries
import ROOT
from ROOT import Belle2
ROOT.gSystem.Load('libvxd_dataobjects')


class Clusters2Truehits(Module):

    """A simple module to check the reconstruction of SVDTrueHits."""

    def __init__(self):
        """Initialize the module"""

        super(Clusters2Truehits, self).__init__()

    def beginRun(self):
        """ Set up the relation array. """

    def event(self):
        """Find clusters corresponding to simulated truehits."""

        truehits = Belle2.PyStoreArray('SVDTrueHits')
        nTruehits = truehits.getEntries()
        digits = Belle2.PyStoreArray('SVDDigits')
        nDigits = digits.getEntries()
        clusters = Belle2.PyStoreArray('SVDClusters')
        nClusters = clusters.getEntries()
        relClustersToTrueHits = \
            Belle2.PyRelationArray('SVDClustersToSVDTrueHits')
        nClusterRelations = relClustersToTrueHits.getEntries()
        print 'Found {nT} TrueHits and {nC} Clusters, with {nRel} relations.'\
            .format(nT=nTruehits, nC=nClusters, nRel=nClusterRelations)
        relDigitsToTrueHits = Belle2.PyRelationArray('SVDDigitsToSVDTrueHits')
        nDigitRelations = relDigitsToTrueHits.getEntries()
        print 'Found {nT} TrueHits and {nD} Digits, with {nRel} relations.'\
            .format(nT=nTruehits, nD=nDigits, nRel=nDigitRelations)
        relClustersToDigits = Belle2.PyRelationArray('SVDClustersToSVDDIgits')
        print 'Found {nR} relations between clusters and digits.'\
            .format(nR=relClustersToDigits.getEntries())
        # Define auxiliary print functions
        truehit_print = lambda index, truehit: \
            'TrueHit {index}: u {u}, v {v}, charge {charge}'\
            .format(index=index, u=truehit.getU(), v=truehit.getV(),
                    charge=truehit.getEnergyDep())
        cluster_print = lambda index, cluster: \
            ('Cluster {index}: u-side {side}, position {pos}, '
             + 'positionError {posError}, time {time}, signal {signal}'
             ).format(
                index=index,
                side=cluster.isUCluster(),
                pos=cluster.getPosition(),
                posError=cluster.getPositionSigma(),
                time=cluster.getClsTime(),
                signal=cluster.getCharge(),
            )
        digit_print = lambda index, digit: \
            ('Digit {index}: u-side {side}, strip {pos}, time {time},'
             + 'signal {signal}').format(
                index=index, side=digit.isUStrip(),
                pos=digit.getCellID(), time=digit.getIndex(),
                signal=digit.getCharge()
            )

        # Use the relation to get cluster's digits and truehits.
        truehits_unused_u = set(range(nTruehits))
        truehits_unused_v = set(range(nTruehits))

        for cluster_index in range(nClusters):
            cluster = clusters[cluster_index]
            uside = cluster.isUCluster()
            print cluster_print(cluster_index, cluster)
            for digit_index in relClustersToDigits.getToIndices(cluster_index):
                print digit_print(digit_index, digits[digit_index])

            print
            cluster_truehits = \
                relClustersToTrueHits.getToIndices(cluster_index)
            for truehit_index in cluster_truehits:
                print truehit_print(truehit_index, truehits[truehit_index])
                if uside:
                    truehits_unused_u.discard(truehit_index)
                else:
                    truehits_unused_v.discard(truehit_index)
                # Get digit indices for the current side
                truehit_digits = set()
                for digit_index in range(nDigits):
                    digit = digits[digit_index]
                    if digit.isUStrip() == uside:
                        for index in \
                                relDigitsToTrueHits.getToIndices(digit_index):
                            if index == truehit_index:
                                truehit_digits.add(digit_index)

                for index in truehit_digits:
                    print digit_print(index, digits[index])

                print

            print

        print
        print 'Truehits with no clusters in u: {n}'\
            .format(n=len(truehits_unused_u))
        print truehits_unused_u
        for truehit_index in truehits_unused_u:
            print truehit_print(truehit_index, truehits[truehit_index])
            truehit_digits = set()
            for digit_index in range(nDigits):
                digit = digits[digit_index]
                if digit.isUStrip():
                    for index in relDigitsToTrueHits.getToIndices(digit_index):
                        if index == truehit_index:
                            truehit_digits.add(digit_index)

            for index in truehit_digits:
                print digit_print(index, digits[index])

            print

        print
        print 'Truehits with no clusters in v: {n}'\
            .format(n=len(truehits_unused_v))
        print truehits_unused_v
        for truehit_index in truehits_unused_v:
            print truehit_print(truehit_index, truehits[truehit_index])
            truehit_digits = set()
            for digit_index in range(nDigitRelations):
                digit = digits[digit_index]
                if not digit.isUStrip():
                    for index in relDigitsToTrueHits.getToIndices(digit_index):
                        if index == truehit_index:
                            truehit_digits.add(digit_index)

            print truehit_digits
            for index in truehit_digits:
                print digit_print(index, digits[index])

            print

        print


# Particle gun module
particlegun = register_module('ParticleGun')
# Create Event information
eventinfosetter = register_module('EventInfoSetter')
# Show progress of processing
progress = register_module('Progress')
# Load parameters
gearbox = register_module('Gearbox')
# Create geometry
geometry = register_module('Geometry')
# Run simulation
simulation = register_module('FullSim')
# SVD digitization module
svddigi = register_module('SVDDigitizer')
# SVD clustering module
svdclust = register_module('SVDClusterizer')
# RootOutput
output = register_module('RootOutput')

analyze = Clusters2Truehits()

# Specify number of events to generate
eventinfosetter.param({'evtNumList': [10], 'runList': [1]})

# Set parameters for particlegun
particlegun.param({
    'nTracks': 1,
    'varyNTracks': True,
    'pdgCodes': [211, -211, 11, -11],
    'momentumGeneration': 'normalPt',
    'momentumParams': [5, 1],
    'phiGeneration': 'normal',
    'phiParams': [180, 30],
    'thetaGeneration': 'uniformCos',
    'thetaParams': [17, 150],
    'vertexGeneration': 'normal',
    'xVertexParams': [0, 0.2],
    'yVertexParams': [0, 0.2],
    'zVertexParams': [0, 0],
    'independentVertices': False,
})

# Select subdetectors to be built
geometry.param('components', ['MagneticField', 'PXD', 'SVD'])

svddigi.param('statisticsFilename', 'digi.root')
svddigi.param('ElectronicEffects', True)

# create processing path
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(particlegun)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(simulation)
main.add_module(svddigi)
main.add_module(svdclust)
main.add_module(analyze)
main.add_module(output)

# generate events
process(main)

# show call statistics
print statistics

# Wait for enter to be pressed
# sys.stdin.readline()
