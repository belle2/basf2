import basf2  # Import basf2 to make the Belle2 namespace available
import ROOT
from ROOT import Belle2

import numpy as np
import math
import contextlib


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


@contextlib.contextmanager
def root_cd(tdirectory):
    """Context manager that temporarily switches the current global ROOT directory while in the context.

    If a string as the name of a directory is given as the argument
    try to switch to the directory with that name in the current ROOT folder.

    If it is not present create it.

    Parameters
    ----------
    tdirectory : ROOT.TDirectory or str
        ROOT directory to switch to or name of a folder to switch.

    Returns
    -------
    ROOT.TDirectory
        The new current ROOT directory.
    """

    # Do not use ROOT.gDirectory here.
    # Since ROOT.gDirectory gets transported as a reference it changes on a call to cd() as well,
    # and can therefore not serve to save the former directory.
    save_tdirectory = ROOT.gROOT.CurrentDirectory()

    if isinstance(tdirectory, basestring):
        tdirectory_name = tdirectory

        # Look for the tdirectory with the name
        # before trying to create it
        tdirectory = save_tdirectory.GetDirectory(tdirectory_name)
        if not tdirectory:
            tdirectory = save_tdirectory.mkdir(tdirectory_name, tdirectory_name)
            if not tdirectory:
                raise RuntimeError("Could not create or find folder %s" % tdirectory_name)

            # If tdirectory_name is as hierachy like a/b/c make sure we select the most nested folder
            # (and not a as the documentation of TDirectory.mkdir suggests).
            tdirectory = save_tdirectory.GetDirectory(tdirectory_name)

    try:
        if tdirectory is not None:
            tdirectory.cd()
        yield tdirectory

    finally:
        save_tdirectory.cd()


def root_save_name(name):
    """Strips all meta characters that might be unsafe to use as a ROOT name"""

    deletechars = r"/$\#{}()"
    name = name.replace(' ', '_').replace('-', '_').translate(None, deletechars)
    return name


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


def getHelixFromMCParticle(mcParticle):
    position = mcParticle.getVertex()
    momentum = mcParticle.getMomentum()
    charge_sign = (-1 if mcParticle.getCharge() < 0 else 1)
    b_field = 1.5

    seed_helix = Belle2.Helix(position, momentum, charge_sign, b_field)
    return seed_helix


def getSeedTrackFitResult(trackCand):
    position = trackCand.getPosSeed()
    momentum = trackCand.getMomSeed()
    cartesian_covariance = trackCand.getCovSeed()
    charge_sign = (-1 if trackCand.getChargeSeed() < 0 else 1)
    particle_type = Belle2.Const.ParticleType(trackCand.getPdgCode())
    p_value = float('nan')
    b_field = 1.5
    cdc_hit_pattern = 0
    svd_hit_pattern = 0

    track_fit_result = Belle2.TrackFitResult(
        position,
        momentum,
        cartesian_covariance,
        charge_sign,
        particle_type,
        p_value,
        b_field,
        cdc_hit_pattern,
        svd_hit_pattern,
    )

    return track_fit_result
