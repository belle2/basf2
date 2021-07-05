##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import numpy as np
import pandas as pd


def landau(x, a, mu, sigma):
    # A simple landau with thee capability to use lists as input
    if isinstance(x, np.ndarray):
        return np.array([landau(xi, a, mu, sigma) for xi in x])
    elif isinstance(x, pd.Series):
        return np.array([landau(xi, a, mu, sigma) for xi in x.values])
    else:
        from ROOT import TMath
        return a * TMath.Landau(x, mu, sigma, False)


def landau_with_gaus(x, a, mu, sigma, b, c, d):
    # A landau convoluted with a gauss. Needs a list as input
    if isinstance(x, np.ndarray):
        from ROOT import TMath
        return np.convolve([a * TMath.Landau(xi, mu, sigma, False)
                            for xi in x], [b * TMath.Gaus(xi, c, d, False) for xi in x], "same")
    else:
        raise ValueError()


def gumbel(x, mu, sigma, a):
    # A gumbel function
    z = (x - mu) / sigma
    return a * 1 / sigma * np.exp(- (z + np.exp(-z)))


def norm(x, a, mu, sigma):
    # A simple gauss
    return a * np.exp(-(x - mu) ** 2 / (2 * sigma ** 2))


def bifur_norm(x, a, mu, sigma_l, sigma_r):
    # A bifurcated gauss
    if isinstance(x, np.ndarray):
        return np.array([bifur_norm(xi, a, mu, sigma_l, sigma_r) for xi in x])
    else:
        arg = x - mu
        if arg < 0:
            return norm(x, a, mu, sigma_l)
        else:
            return norm(x, a, mu, sigma_r)


def double_norm(x, a, mu, sigma, b, mu2, sigma2):
    # A sum of two gauss
    return norm(x, a, mu, sigma) + norm(x, b, mu2, sigma2)


def norm_plus_lin(x, a, mu, sigma, b, c):
    # A gauss plus a linear function
    return norm(x, a, mu, sigma) + b * x + c


def inverse_squared(x, a, b, c, d):
    # A 1/x^2 function
    return a / (x - b) ** 2 + c + d * x


def inverse_sqrt(x, a, b, c, d):
    return a / (np.sqrt(x - b)) + c + d * x
