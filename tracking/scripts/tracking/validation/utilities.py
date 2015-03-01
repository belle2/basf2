import ROOT
from ROOT import Belle2
import numpy as np
import math

# Vectorised version of the error function for numpy arrays
try:
    from scipy.special import erf
except ImportError:
    # Minimal workaround that only relies on numpy and python 2.7
    # erf as a vectorized function
    # Will convert the incoming nparray to dtype 'object', if nan
    # values are contained
    # use numpy.asfarray(...) to convert back
    erf_ufunc = np.frompyfunc(math.erf, 1, 1)

    def erf(*args):
        result = erf_ufunc(*args)
        return np.asfarray(result)


# Vectorised version of the Prob function as known from TMath for numpy arrays
try:
    from scipy.stats import chi2
    # Prob(chi2, ndf) is defined as 1 - cdf (cumulative density function of the chi2 distribution with ndf)
    prob = chi2.sf
    del chi2

except ImportError:
    # Minimal workaround that only relies on numpy and python 2.7
    # prob as a vectorized function
    prop = np.frompyfunc(ROOT.TMath.Prob, 2, 1)


def is_primary(mc_particle):
    """Indicates if the given MCParticle is primary.

    Parameters
    ----------
    mc_particle : Belle2.MCParticle
        MCParticle to be checked"""

    return mc_particle.hasStatus(Belle2.MCParticle.c_PrimaryParticle)


def is_stable_in_generator(mc_particle):
    """Indicates if the given MCParticle is stable in the generator

    Parameters
    ----------
    mc_particle : Belle2.MCParticle
        MCParticle to be checked"""

    return mc_particle.hasStatus(Belle2.MCParticle.c_StableInGenerator)
