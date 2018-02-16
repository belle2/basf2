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


def importPXDPositionEstimator(fileName):
    """
    This function imports payloads for the PXD
    cluster position estimation.
    """

    # Load trained hit estimator
    hit_estimator = pickle.load(open(fileName, 'rb'))

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
        estimator_payload.addGrid(pixelkind, grid)

        for uBin in range(1, grid.GetXaxis().GetNbins() + 1):
            for vBin in range(1, grid.GetYaxis().GetNbins() + 1):

                classifier = hit_estimator.getShapeClassifier(uBin, vBin, pixelkind)
                classifier_payload = Belle2.PXDClusterShapeClassifierPar()

                for shape in classifier.getDigitalLabels():

                    shape_index = indexer_payload.getShapeIndex(shape)
                    classifier_payload.addShape(shape_index)

                    # Likelyhood to create shape
                    classifier_payload.addShapeLikelyhood(shape_index, classifier.getDigitalProb(shape))

                    percentiles = classifier.getPercentiles(shape)

                    for eta_index, hit in classifier.getHitMap(shape).items():

                        # Read hit ... and fill into offset payload
                        pos, cov, prob = hit
                        # FIXME: python hit estimator uses mm, but basf2 default unit is cm
                        offset_payload = Belle2.PXDClusterOffsetPar(
                            0.1 * pos[0], 0.1 * pos[1], 0.01 * cov[0, 0], 0.01 * cov[1, 1], 0.01 * cov[1, 0])
                        classifier_payload.addEtaOffset(shape_index, offset_payload)

                        # Fill percentile and eta likelyhood
                        classifier_payload.addEtaPercentile(shape_index, percentiles[eta_index - 1])
                        classifier_payload.addEtaLikelyhood(shape_index, prob)

                # and fill into position estimator payload
                estimator_payload.setShapeClassifier(classifier_payload, uBin, vBin, pixelkind)

    iov = Belle2.IntervalOfValidity(0, 0, -1, -1)
    Belle2.Database.Instance().storeData('PXDClusterPositionEstimatorPar', estimator_payload, iov)
    Belle2.Database.Instance().storeData('PXDClusterShapeIndexPar', indexer_payload, iov)


importPXDPositionEstimator(fileName='/home/benjamin/Desktop/cluster_shape_paper/examples/estimator_headtail_phase3.out')
