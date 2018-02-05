#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>benni</contact>
  <description>
    Histogram the difference between the estimated cluster position and the
    position of the related truehit for all layers of the PXD.
  </description>
</header>
"""

import sys
import math
import numpy as np
from basf2 import *
from pxd import *
import ROOT
from ROOT import Belle2

# set_log_level(LogLevel.ERROR)
# set some random seed
set_random_seed(10346)
# momenta to generate the plots for
momenta = [3.0]
# theta parameters
theta_params = [90, 0.1]


# FIXME: The following functions are helpers. Find a more reasonable implementation for them.
import re


def computeFeature(shape, thetaU, thetaV):
    """Returns feature scalar """
    feature = 0.0
    entry, exit = get_entry_exit_index(shape, thetaU, thetaV)

    if not entry == exit:
        # Feature is the eta value from entry/exit signals
        picked_signals = pick_signals(shape, indexset=[entry, exit])
        feature = picked_signals[0] / (picked_signals[0] + picked_signals[1])
    else:
        # Single digit shape. Feature is the digit charge
        picked_signals = pick_signals(shape, indexset=[entry])
        feature = picked_signals[0]

    return feature


def get_entry_exit_index(shape, thetaU=1, thetaV=1):
    "Get indices for entry and exit pixel of track depending on slopes into sensor"
    vcells = get_vcells(shape)
    size = vcells.shape[0]
    vmax = np.max(vcells)

    if thetaV >= 0:
        if thetaU >= 0:
            return 0, size - 1
        else:
            return get_indices(shape, 0)[-1], get_indices(shape, vmax)[0]
    else:
        if thetaU >= 0:
            return get_indices(shape, vmax)[0], get_indices(shape, 0)[-1]
        else:
            return get_indices(shape, vmax)[-1], get_indices(shape, 0)[0]


def get_short_digital_label(shape, thetaU, thetaV):
    """Short (digital) label contains only entry&exit pixels hit by track"""
    label = "S"
    digits = re.split('D', shape)
    entry, exit = get_entry_exit_index(shape, thetaU, thetaV)

    # Add entry pixel
    data = re.split('\.', digits[entry + 1])
    label += "D" + data[0] + '.' + data[1]

    if not entry == exit:
        # Add exit pixel
        data = re.split('\.', digits[exit + 1])
        label += "D" + data[0] + '.' + data[1]

    return label


def get_size(shape):
    """Number of digits in shape"""
    return len(re.split('D', shape)) - 1


def get_signals(shape):
    """Get array of signals from shape"""
    size = get_size(shape)
    signals = np.zeros((size,), dtype=np.int)
    digits = re.split('D', shape)
    for i in range(size):
        digit = digits[i + 1]
        signals[i] = int(re.split('\.', digit)[2])
    return signals


def pick_signals(shape, indexset):
    """Get array of picked signals from shape. Picked signals are given in indexset."""
    dim = len(indexset)
    signals = np.zeros((dim,), dtype=np.int)
    digits = re.split('D', shape)
    for i, index in enumerate(indexset):
        digit = digits[index + 1]
        signals[i] = int(re.split('\.', digit)[2])
    return signals


def get_vcells(shape):
    """Get array of vcells from shape"""
    size = get_size(shape)
    vcells = np.zeros((size,), dtype=np.int)
    digits = re.split('D', shape)
    for i in range(size):
        digit = digits[i + 1]
        vcells[i] = int(re.split('\.', digit)[0])
    return vcells


def get_indices(shape, v):
    """Get list of digit indices for vcell=v"""
    indexlist = []
    for i, vcell in enumerate(get_vcells(shape)):
        if vcell == v:
            indexlist.append(i)
    return indexlist


class PXDPositionEstimation(Module):
    """
    Histogram the difference between the estimated cluster position and the position of the related truehit.
    """

    def __init__(self):
        """Create a member to access cluster shape position estimator"""
        super().__init__()  # don't forget to call parent constructor
        self.position_estimator = Belle2.PyDBObj('PXDClusterPositionEstimatorPar')
        self.shape_indexer = Belle2.PyDBObj('PXDClusterShapeIndexPar')
        self.eventinfo = Belle2.PyStoreObj('EventMetaData')

    def compute_shape(self, cluster):
        """ Returns shape for pxd cluster"""

        # get sorted list of digits caused by truehits
        digits = cluster.getRelationsTo("PXDDigits")

        # compute the shape string
        minu = min([digit.getUCellID() for digit in digits])
        minv = min([digit.getVCellID() for digit in digits])
        shape = str(len(digits))
        sensor_info = Belle2.VXD.GeoCache.get(cluster.getSensorID())
        reject = False
        kinds = set()

        for i, digit in enumerate(digits):
            charge = int(digit.getCharge())
            shape += 'D' + str(digit.getVCellID() - minv) + '.' + str(digit.getUCellID() - minu) + '.' + str(charge)
            kind = self.getPixelKind(sensor_info.getVPitch(sensor_info.getVCellPosition(digit.getVCellID())))
            kinds.add(kind)

            # Cluster at sensor edge
            if digit.getVCellID() <= 0 or digit.getVCellID() >= 767:
                reject = True
            # Cluster at sensor edge
            if digit.getUCellID() <= 0 or digit.getUCellID() >= 249:
                reject = True

        # Check cluster has not digits with different pixel kind
        if not len(kinds) == 1:
            reject = True

        return shape, minu, minv, reject, kinds.pop()

    def getPixelKind(self, VPitch):
        if abs(VPitch - 0.0055) < 0.0001:
            return 0
        elif abs(VPitch - 0.0060) < 0.0001:
            return 1
        elif abs(VPitch - 0.0070) < 0.0001:
            return 2
        elif abs(VPitch - 0.0085) < 0.0001:
            return 3
        else:
            return -1

    def event(self):
        """Fill the residual and pull histograms"""

        # Get truehits
        truehits = Belle2.PyStoreArray("PXDTrueHits")

        for truehit in truehits:
            if isinstance(truehit, Belle2.PXDTrueHit):
                clusters = truehit.getRelationsFrom("PXDClusters")

                # now check if we find a cluster
                for j, cls in enumerate(clusters):
                    # we ignore all clusters where less then 100 electrons come from
                    # our truehit
                    if clusters.weight(j) < 100:
                        continue

                    shape, minu, minv, reject, pixelkind = self.compute_shape(cls)
                    mom = truehit.getMomentum()
                    thetaV = math.atan(mom[1] / mom[2]) * 180 / math.pi
                    thetaU = math.atan(mom[0] / mom[2]) * 180 / math.pi
                    print("Found truehit - cluster match on sensor " +
                          str(truehit.getSensorID()) + " ({:d})".format(int(truehit.getSensorID())))
                    print("    Matched truehit has angles thetaU/thetaV: {:.1f}/{:.1f} ".format(thetaU, thetaV))
                    print("    Matched cluster has shape " + shape + " location at ui/vi: {:d}/{:d}".format(minu, minv))

                    eta = computeFeature(shape, thetaU, thetaV)
                    dshape = get_short_digital_label(shape, thetaU, thetaV)
                    shape_index = self.shape_indexer.getShapeIndex(dshape)

                    print("    shape {} -> index {}".format(shape, shape_index))
                    print("    eta={:.5f}".format(eta))

                    # FIXME: how to get the eta index
                    if get_size(shape) == 1:
                        print('skipping single pixel cluster')
                        continue

                    eta_index = 0
                    if not reject and self.position_estimator.hasOffset(shape_index, eta_index, thetaU, thetaV, pixelkind):
                        print("FOUND CORRECTION")

                    #  # Now, we can safely querry the hit
                    #  offsets, cov, prob = self.estimator.getHit(shape, thetaU, thetaV, pixelkind)
                    #  print("    Matching likelyhood is {:.3f}".format(prob) )
                    #
                    #  #for pixelkind in self.position_estimator.getPixelkinds():
                    #  bool hasOffset(int shape_index, int feature_index, double thetaU, double thetaV, int pixelkind)


# Now let's create a path to simulate our events. We need a bit of statistics but
# that's not too bad since we only simulate single muons
main = create_path()
main.add_module("EventInfoSetter", evtNumList=[10000])
main.add_module("Gearbox")
# we only need the pxd for this
main.add_module("Geometry", components=['MagneticFieldConstant4LimitedRSVD',
                                        'BeamPipe', 'PXD'])
particlegun = main.add_module("ParticleGun")
particlegun.param({
    "nTracks": 1,
    "pdgCodes": [13, -13],
    # generate discrete momenta with equal weights
    "momentumGeneration": 'discrete',
    "momentumParams": momenta + [1] * len(momenta),
    "thetaGeneration": 'normal',
    "thetaParams": theta_params,
})
main.add_module("FullSim")
add_pxd_simulation(main)
add_pxd_reconstruction(main)

positionestimation = PXDPositionEstimation()
main.add_module(positionestimation)
main.add_module("Progress")

process(main)
print(statistics)
