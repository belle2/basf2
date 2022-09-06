#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import B2ERROR
import modularAnalysis as ma
from stdCharged import stdPi, stdPr
import vertex


def stdKshorts(prioritiseV0=True, fitter='TreeFit', path=None):
    """
    Load a combined list of the Kshorts list from V0 objects merged with
    a list of particles combined using the analysis ParticleCombiner module.

    The ParticleList is named ``K_S0:merged``. A vertex fit is performed and only
    candidates with an invariant mass in the range :math:`0.450 < M < 0.550~GeV`,
    and for which the vertex fit did not fail, are kept.

    The vertex fitter can be selected among ``TreeFit``, ``KFit``, and ``Rave``.

    Parameters:
        prioritiseV0 (bool): should the V0 mdst objects be prioritised when merging?
        fitter (str): vertex fitter name, valid options are ``TreeFit``, ``KFit``, and ``Rave``.
        path (basf2.Path): the path to load the modules
    """
    # Fill one list from V0
    ma.fillParticleList('K_S0:V0 -> pi+ pi-', '', True, path=path)
    ma.cutAndCopyList('K_S0:V0_MassWindow', 'K_S0:V0', '0.3 < M < 0.7', path=path)
    # Perform vertex fit and apply tighter mass window
    if fitter == 'TreeFit':
        vertex.treeFit('K_S0:V0_MassWindow', conf_level=0.0, path=path)
    elif fitter == 'KFit':
        vertex.kFit('K_S0:V0_MassWindow', conf_level=0.0, path=path)
    elif fitter == 'Rave':
        vertex.raveFit('K_S0:V0_MassWindow', conf_level=0.0, path=path, silence_warning=True)
    else:
        B2ERROR("Valid fitter options for Kshorts are 'TreeFit', 'KFit', and 'Rave'. However, the latter is not recommended.")
    ma.applyCuts('K_S0:V0_MassWindow', '0.450 < M < 0.550', path=path)
    # Reconstruct a second list
    stdPi('all', path=path)  # no quality cuts
    ma.reconstructDecay('K_S0:RD -> pi+:all pi-:all', '0.3 < M < 0.7', 1, True, path=path)
    # Again perform vertex fit and apply tighter mass window
    if fitter == 'TreeFit':
        vertex.treeFit('K_S0:RD', conf_level=0.0, path=path)
    elif fitter == 'KFit':
        vertex.kFit('K_S0:RD', conf_level=0.0, path=path)
    elif fitter == 'Rave':
        vertex.raveFit('K_S0:RD', conf_level=0.0, path=path, silence_warning=True)
    ma.applyCuts('K_S0:RD', '0.450 < M < 0.550', path=path)
    # Create merged list based on provided priority
    ma.mergeListsWithBestDuplicate('K_S0:merged', ['K_S0:V0_MassWindow', 'K_S0:RD'],
                                   variable='particleSource', preferLowest=prioritiseV0, path=path)


def goodBelleKshort(path):
    """
    Load the Belle goodKshort list. Creates a ParticleList named
    ``K_S0:legacyGoodKS``. A vertex fit is performed and only candidates that
    satisfy the :b2:var:`goodBelleKshort` criteria, with an invariant mass in the range
    :math:`0.468 < M < 0.528~GeV`, and for which the vertex fit did not fail, are kept

    Parameters:
        path (basf2.Path): the path to load the modules
    """
    ma.fillParticleList('K_S0:legacyGoodKS -> pi+ pi-', '0.3 < M < 0.7', True, path=path)
    vertex.kFit('K_S0:legacyGoodKS', conf_level=0.0, path=path)
    ma.applyCuts('K_S0:legacyGoodKS', '0.468 < M < 0.528 and goodBelleKshort==1', path=path)


def scaleErrorKshorts(prioritiseV0=True, fitter='TreeFit',
                      scaleFactors_V0=[1.125927, 1.058803, 1.205928, 1.066734, 1.047513],
                      scaleFactorsNoPXD_V0=[1.125927, 1.058803, 1.205928, 1.066734, 1.047513],
                      d0Resolution_V0=[0.001174, 0.000779],
                      z0Resolution_V0=[0.001350, 0.000583],
                      d0MomThr_V0=0.500000,
                      z0MomThr_V0=0.00000,
                      scaleFactors_RD=[1.149631, 1.085547, 1.151704, 1.096434, 1.086659],
                      scaleFactorsNoPXD_RD=[1.149631, 1.085547, 1.151704, 1.096434, 1.086659],
                      d0Resolution_RD=[0.00115328, 0.00134704],
                      z0Resolution_RD=[0.00124327, 0.0013272],
                      d0MomThr_RD=0.500000,
                      z0MomThr_RD=0.500000,
                      path=None):
    '''
    Reconstruct K_S0 applying helix error correction to K_S0 daughters given by ``modularAnalysis.scaleError``.
    The ParticleList is named ``K_S0:scaled``

    Considering the difference of multiple scattering through the beam pipe,
    different parameter sets are used for K_S0 decaying outside/inside the beam pipe (``K_S0:V0/RD``).

    Only for TDCPV analysis.

    @param prioritiseV0         If True K_S0 from V0 object is prioritised over RD when merged.
    @param fitter               Vertex fitter option. Choose from ``TreeFit``, ``KFit`` and ``Rave``.
    @param scaleFactors_V0      List of five constants to be multiplied to each of helix errors (for tracks with a PXD hit)
    @param scaleFactorsNoPXD_V0 List of five constants to be multiplied to each of helix errors (for tracks without a PXD hit)
    @param d0Resolution_V0      List of two parameters, (a [cm], b [cm/(GeV/c)]),
                                defining d0 best resolution as sqrt{ a**2 + (b / (p*beta*sinTheta**1.5))**2 }
    @param z0Resolution_V0      List of two parameters, (a [cm], b [cm/(GeV/c)]),
                                defining z0 best resolution as sqrt{ a**2 + (b / (p*beta*sinTheta**2.5))**2 }
    @param d0MomThr_V0          d0 best resolution is kept constant below this momentum
    @param z0MomThr_V0          z0 best resolution is kept constant below this momentum
    @param scaleFactors_RD      List of five constants to be multiplied to each of helix errors (for tracks with a PXD hit)
    @param scaleFactorsNoPXD_RD List of five constants to be multiplied to each of helix errors (for tracks without a PXD hit)
    @param d0Resolution_RD      List of two parameters, (a [cm], b [cm/(GeV/c)]),
                                defining d0 best resolution as sqrt{ a**2 + (b / (p*beta*sinTheta**1.5))**2 }
    @param z0Resolution_RD      List of two parameters, (a [cm], b [cm/(GeV/c)]),
                                defining z0 best resolution as sqrt{ a**2 + (b / (p*beta*sinTheta**2.5))**2 }
    @param d0MomThr_RD          d0 best resolution is kept constant below this momentum
    @param z0MomThr_RD          z0 best resolution is kept constant below this momentum

    '''
    from basf2 import register_module
    # Load K_S0 from V0 and apply helix error correction to V0 daughters
    ma.fillParticleList('K_S0:V0 -> pi+ pi-', '', True, path=path)
    scaler_V0 = register_module("HelixErrorScaler")
    scaler_V0.set_name('ScaleError_' + 'K_S0:V0')
    scaler_V0.param('inputListName', 'K_S0:V0')
    scaler_V0.param('outputListName', 'K_S0:V0_scaled')
    scaler_V0.param('scaleFactors_PXD', scaleFactors_V0)
    scaler_V0.param('scaleFactors_noPXD', scaleFactorsNoPXD_V0)
    scaler_V0.param('d0ResolutionParameters', d0Resolution_V0)
    scaler_V0.param('z0ResolutionParameters', z0Resolution_V0)
    scaler_V0.param('d0MomentumThreshold', d0MomThr_V0)
    scaler_V0.param('z0MomentumThreshold', z0MomThr_V0)
    path.add_module(scaler_V0)

    ma.applyCuts('K_S0:V0_scaled', '0.3 < M < 0.7', path=path)
    # Perform vertex fit and apply tighter mass window
    if fitter == 'TreeFit':
        vertex.treeFit('K_S0:V0_scaled', conf_level=0.0, path=path)
    elif fitter == 'KFit':
        vertex.kFit('K_S0:V0_scaled', conf_level=0.0, path=path)
    elif fitter == 'Rave':
        vertex.raveFit('K_S0:V0_scaled', conf_level=0.0, path=path, silence_warning=True)
    else:
        B2ERROR("Valid fitter options for Kshorts are 'TreeFit', 'KFit', and 'Rave'. However, the latter is not recommended.")
    ma.applyCuts('K_S0:V0_scaled', '0.450 < M < 0.550', path=path)

    # Reconstruct a second list
    stdPi('all', path=path)
    ma.scaleError('pi+:scaled', 'pi+:all',
                  scaleFactors=scaleFactors_RD,
                  scaleFactorsNoPXD=scaleFactorsNoPXD_RD,
                  d0Resolution=d0Resolution_RD,
                  z0Resolution=z0Resolution_RD,
                  d0MomThr=d0MomThr_RD,
                  z0MomThr=z0MomThr_RD,
                  path=path)

    ma.reconstructDecay('K_S0:RD_scaled -> pi+:scaled pi-:scaled', '0.3 < M < 0.7', 1, True, path=path)
    # Again perform vertex fit and apply tighter mass window
    if fitter == 'TreeFit':
        vertex.treeFit('K_S0:RD_scaled', conf_level=0.0, path=path)
    elif fitter == 'KFit':
        vertex.kFit('K_S0:RD_scaled', conf_level=0.0, path=path)
    elif fitter == 'Rave':
        vertex.raveFit('K_S0:RD_scaled', conf_level=0.0, path=path, silence_warning=True)
    ma.applyCuts('K_S0:RD_scaled', '0.450 < M < 0.550', path=path)
    # Create merged list based on provided priority
    ma.mergeListsWithBestDuplicate('K_S0:scaled', ['K_S0:V0_scaled', 'K_S0:RD_scaled'],
                                   variable='particleSource', preferLowest=prioritiseV0, path=path)


def stdLambdas(prioritiseV0=True, fitter='TreeFit', path=None):
    """
    Load a combined list of the Lambda list from V0 objects merged with
    a list of particles combined using the analysis ParticleCombiner module.

    The ParticleList is named ``Lambda0:merged``. A vertex fit is performed and only
    candidates with an invariant mass in the range :math:`1.10 < M < 1.13~GeV`,
    and for which the vertex fit did not fail, are kept.

    The vertex fitter can be selected among ``TreeFit``, ``KFit``, and ``Rave``.

    Parameters:
        prioritiseV0 (bool): should the V0 mdst objects be prioritised when merging?
        fitter (str): vertex fitter name, valid options are ``TreeFit``, ``KFit``, and ``Rave``.
        path (basf2.Path): the path to load the modules
    """
    # Fill one list from V0
    ma.fillParticleList('Lambda0:V0 -> p+ pi-', '', True, path=path)
    ma.cutAndCopyList('Lambda0:V0_MassWindow', 'Lambda0:V0', '0.9 < M < 1.3', path=path)
    # Perform vertex fit and apply tighter mass window
    if fitter == 'TreeFit':
        vertex.treeFit('Lambda0:V0_MassWindow', conf_level=0.0, path=path)
    elif fitter == 'KFit':
        vertex.kFit('Lambda0:V0_MassWindow', conf_level=0.0, path=path)
    elif fitter == 'Rave':
        vertex.raveFit('Lambda0:V0_MassWindow', conf_level=0.0, path=path, silence_warning=True)
    else:
        B2ERROR("Valid fitter options for Lambdas are 'TreeFit', 'KFit', and 'Rave'. However, the latter is not recommended.")
    ma.applyCuts('Lambda0:V0_MassWindow', '1.10 < M < 1.13', path=path)
    # Find V0 duplicate with better vertex fit quality
    ma.markDuplicate('Lambda0:V0_MassWindow', False, path=path)
    ma.applyCuts('Lambda0:V0_MassWindow', 'extraInfo(highQualityVertex)', path=path)
    # Reconstruct a second list
    stdPi('all', path=path)  # no quality cuts
    stdPr('all', path=path)  # no quality cuts
    ma.reconstructDecay('Lambda0:RD -> p+:all pi-:all', '0.9 < M < 1.3', 1, True, path=path)
    # Again perform vertex fit and apply tighter mass window
    if fitter == 'TreeFit':
        vertex.treeFit('Lambda0:RD', conf_level=0.0, path=path)
    elif fitter == 'KFit':
        vertex.kFit('Lambda0:RD', conf_level=0.0, path=path)
    elif fitter == 'Rave':
        vertex.raveFit('Lambda0:RD', conf_level=0.0, path=path, silence_warning=True)
    ma.applyCuts('Lambda0:RD', '1.10 < M < 1.13', path=path)
    # Find RD duplicate with better vertex fit quality
    ma.markDuplicate('Lambda0:RD', False, path=path)
    ma.applyCuts('Lambda0:RD', 'extraInfo(highQualityVertex)', path=path)
    ma.mergeListsWithBestDuplicate('Lambda0:merged', ['Lambda0:V0_MassWindow', 'Lambda0:RD'],
                                   variable='particleSource', preferLowest=prioritiseV0, path=path)
