##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import logging
from tracking.validation.utilities import prob
from ROOT import Belle2  # make Belle2 namespace available
from ROOT import gSystem
import matplotlib.pyplot as plt
import numpy as np
import numpy.random

import matplotlib as mpl
mpl.use('Agg')


gSystem.Load('libframework')
gSystem.Load('libtracking')
gSystem.Load('libtracking_trackFindingCDC')


def get_logger():
    return logging.getLogger(__name__)


CONTACT = "oliver.frost@desy.de"

n_toys = 10000

radius = 1
center_x = 1
center_y = 1
center_phi = np.arctan2(center_y, center_x)

n_points = 50

# Position variance
pos_var = 0.0001

# Reconstruction variance
pos_var = 0.0001

chi2s = []
curvature_estimates = []
curvature_variances = []


def main():
    for i_toy in range(n_toys):
        chi_random = False
        if chi_random:
            chis = np.random.uniform(0, np.pi, n_points)
        else:
            # equally spaced points
            chis = np.linspace(0, np.pi, n_points)

        ls = np.random.normal(0, 10 * np.sqrt(pos_var), n_points)
        dls = np.random.normal(0, np.sqrt(pos_var), n_points)

        xs = (radius + ls + dls) * np.cos(chis + center_phi - np.pi) + center_x
        ys = (radius + ls + dls) * np.sin(chis + center_phi - np.pi) + center_y

        observations = Belle2.TrackFindingCDC.CDCObservations2D()

        for x, y, l in zip(xs, ys, ls):
            weight = 1 / pos_var
            observations.fill(x, y, l, weight)

        fitter = Belle2.TrackFindingCDC.CDCRiemannFitter.getFitter()
        trajectory = fitter.fit(observations)

        circle = trajectory.getGlobalCircle()
        if circle.curvature() < 0:
            circle.reverse()

        curvature_estimates.append(circle.curvature())
        curvature_variances.append(trajectory.getLocalVariance(0))
        chi2s.append(trajectory.getChi2())

        circle_points = [circle.atArcLength(chi * radius) for chi in np.linspace(0, np.pi, 50)]
        circle_xs = [p.x() for p in circle_points]
        circle_ys = [p.y() for p in circle_points]

        continue
        plt.plot(circle_xs, circle_ys)
        plt.scatter(xs, ys)

        plt.show()

    curvature_residuals = np.array(curvature_estimates) - 1 / radius

    plt.hist(curvature_residuals, bins=100)
    plt.title("Curvature residual")
    plt.show()

    plt.hist(curvature_residuals / np.sqrt(curvature_variances), bins=100, normed=True)

    normal_xs = np.linspace(-4, 4, 50)
    normal_ys = np.exp(-normal_xs * normal_xs / 2) / np.sqrt(2 * np.pi)
    plt.plot(normal_xs, normal_ys)
    plt.title("Curvature pull")
    plt.show()

    plt.hist(chi2s, bins=100)
    plt.show()

    ndfs = n_points - 3
    p_values = prob(chi2s, ndfs)
    plt.hist(p_values, bins=100, normed=True)
    plt.plot([0, 1], [1, 1])
    plt.show()


if __name__ == "__main__":
    main()
