#!/usr/bin/env python3
# -*- coding: utf-8 -*-


##############################################################################
#
# Imports PXD calibration payloads
# Some PXD calibration constants are currently stored in numpy files
# or binary files with pickled python objects.
#
# This example reads these constants, fills dbobjects and imports them
# into the conditions db.
#
##############################################################################

from basf2 import *
from ROOT import Belle2
import ROOT

import pickle

fname = '/home/benjamin/Desktop/cluster_shape_paper/examples/estimator_headtail_half.out'


# Load trained hit estimator
hit_estimator = pickle.load(open(fname, 'rb'))

# Create list of unique shapes
unique_shapes = []

# and fill it
for pixelkind in hit_estimator.getPixelkinds():
    for uBin in range(1, hit_estimator.getGrid(pixelkind).getUBins() + 1):
        for vBin in range(1, hit_estimator.getGrid(pixelkind).getVBins() + 1):
            shape_classifier = hit_estimator.getShapeClassifier(uBin, vBin, pixelkind)
            unique_shapes = unique_shapes + list(shape_classifier.getDigitalLabels())

# Remove all duplicates
unique_shapes = list(set(unique_shapes))
print('number of unique shapes is ', len(unique_shapes))


# Create index table
indexer_payload = Belle2.PXDClusterShapeIndexPar()

for index, shape in enumerate(unique_shapes):
    indexer_payload.addShape(str(shape), index)

# Create position estimator
estimator_payload = Belle2.PXDClusterPositionEstimatorPar()

# ... and fill it
for pixelkind in hit_estimator.getPixelkinds():

    grid = ROOT.TH2F("grid_{:d}".format(pixelkind), "grid_{:d}".format(pixelkind), 18, -90.0, +90.0, 18, -90.0, +90.0)
    estimator_payload.addPixelkind(pixelkind, grid)

    print('pixelkind: ', pixelkind)

    for uBin in range(1, hit_estimator.getGrid(pixelkind).getUBins() + 1):
        for vBin in range(1, hit_estimator.getGrid(pixelkind).getVBins() + 1):

            classifier = hit_estimator.getShapeClassifier(uBin, vBin, pixelkind)
            classifier_payload = Belle2.PXDClusterShapeClassifierPar()

            print('uBin/vBin bin is ({:d}/{:d})'.format(uBin, vBin))

            for shape in classifier.getDigitalLabels():

                shape_index = indexer_payload.getShapeIndex(shape)
                classifier_payload.addShape(shape_index)

                print("shape " + shape + " has index {:d}".format(shape_index))

                # Likelyhood to create shape
                shape_likelyhood = classifier.getDigitalProb(shape)
                classifier_payload.addShapeLikelyhood(shape_index, shape_likelyhood)

                percentiles = classifier.getPercentiles(shape)

                for eta_index, hit in classifier.getHitMap(shape).items():

                    # Read hit
                    offsets = hit[0]
                    cov = hit[1]

                    print("  OFFSET  U={:.5f}, V={:.5f}, USigma2={:.5f}, VSigma2={:.5f}, Cov={:.5f}".format(
                        offsets[0], offsets[1], cov[0, 0], cov[1, 1], cov[0, 1]))

                    # ... and fill into offset payload
                    offset_payload = Belle2.PXDClusterOffsetPar()
                    offset_payload.setU(offsets[0])
                    offset_payload.setV(offsets[1])
                    offset_payload.setUSigma2(cov[0, 0])
                    offset_payload.setVSigma2(cov[1, 1])
                    offset_payload.setUVCovariance(cov[1, 0])

                    # ... and fill offset into shape classifier payload
                    classifier_payload.addOffset(shape_index, offset_payload)

                    # Read percentile (FIXME: the index shift is not nice)
                    percentile = percentiles[eta_index - 1]

                    # ... and fill it
                    classifier_payload.addPercentile(shape_index, percentile)

                    # Read likelyhood to create shape and eta index
                    likelyhood = hit[2]
                    # ... and fill it
                    classifier_payload.addLikelyhood(shape_index, likelyhood)

                    print("  OFFSET  likelyhood {:.5f}".format(likelyhood))
                    print("  OFFSET  percentile {:.5f}".format(percentile))

            # and fill into position estimator payload
            estimator_payload.setShapeClassifier(classifier_payload, uBin, vBin, pixelkind)

iov = Belle2.IntervalOfValidity(0, 0, -1, -1)
Belle2.Database.Instance().storeData('PXDClusterPositionEstimatorPar', estimator_payload, iov)
Belle2.Database.Instance().storeData('PXDClusterShapeIndexPar', indexer_payload, iov)
