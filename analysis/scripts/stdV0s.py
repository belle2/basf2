#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import b2bii
from basf2 import B2ERROR
import modularAnalysis as ma
from stdCharged import stdPi, stdPr
import vertex


def stdKshorts(
    prioritiseV0=True,
    fitter="TreeFit",
    path=None,
    updateAllDaughters=False,
    writeOut=False,
    addSuffix=False,
):
    """
    Load a combined list of the Kshorts list from V0 objects merged with
    a list of particles combined using the analysis ParticleCombiner module.

    The ParticleList is named ``K_S0:merged`` by default. If ``addSuffix`` is set to True,
    then a suffix of form ``_<fitter>`` is added depending on the chosen fitter.
    A vertex fit is performed and only candidates with an invariant mass in the
    range :math:`0.450 < M < 0.550~GeV` after the vertex fit,
    and for which the vertex fit did not fail, are kept.

    The vertex fitter can be selected among ``TreeFit``, ``KFit``, and ``Rave``.

    Parameters:
        prioritiseV0 (bool): should the V0 mdst objects be prioritised when merging?
        fitter (str): vertex fitter name, valid options are ``TreeFit``, ``KFit``, and ``Rave``.
        path (basf2.Path): the path to load the modules
        updateAllDaughters (bool): see the ``updateAllDaughters`` parameter of `vertex.treeFit`
            or the ``daughtersUpdate`` parameter of `vertex.kFit` / `vertex.raveFit`.

            .. warning:: The momenta of the daughters are updated only if ``updateAllDaughters`` is set
                to ``True`` (i.e. **not** by default). Some variables, e.g. `daughterAngle`, will only
                return meaningful results if the daughters momenta are updated.

                This happens because variables like `daughterAngle` assume the direction of the
                daughers momenta *at the Ks vertex* to be provided, while non-updated daughters will
                provide their momenta direction at the point-of-closest-approach (POCA) to the beam axis.

        writeOut (bool): whether RootOutput module should save the created ParticleList
        addSuffix (bool): whether to add a suffix of form ``_<fitter>`` to the ParticleList name
            depending on the chosen fitter
    """
    suffix = ""
    if addSuffix:
        suffix = f"_{fitter}"
    # Fill one list from V0
    ma.fillParticleList(
        f"K_S0:V0_ToFit{suffix} -> pi+ pi-", "", writeOut=writeOut, path=path
    )
    # Perform vertex fit and apply tighter mass window
    if fitter == "TreeFit":
        vertex.treeFit(
            f"K_S0:V0_ToFit{suffix}",
            conf_level=0.0,
            path=path,
            updateAllDaughters=updateAllDaughters,
        )
    elif fitter == "KFit":
        vertex.kFit(
            f"K_S0:V0_ToFit{suffix}",
            conf_level=0.0,
            path=path,
            daughtersUpdate=updateAllDaughters,
        )
    elif fitter == "Rave":
        vertex.raveFit(
            f"K_S0:V0_ToFit{suffix}",
            conf_level=0.0,
            path=path,
            silence_warning=True,
            daughtersUpdate=updateAllDaughters,
        )
    else:
        B2ERROR(
            "Valid fitter options for Kshorts are 'TreeFit', 'KFit', and 'Rave'. However, the latter is not recommended."
        )
    ma.applyCuts(f"K_S0:V0_ToFit{suffix}", "0.450 < M < 0.550", path=path)
    # Reconstruct a second list
    stdPi("all", path=path, writeOut=writeOut)  # no quality cuts
    ma.reconstructDecay(
        f"K_S0:RD{suffix} -> pi+:all pi-:all",
        "0.3 < M < 0.7",
        1,
        writeOut=writeOut,
        path=path,
    )
    # Again perform vertex fit and apply tighter mass window
    if fitter == "TreeFit":
        vertex.treeFit(
            f"K_S0:RD{suffix}",
            conf_level=0.0,
            path=path,
            updateAllDaughters=updateAllDaughters,
        )
    elif fitter == "KFit":
        vertex.kFit(
            f"K_S0:RD{suffix}",
            conf_level=0.0,
            path=path,
            daughtersUpdate=updateAllDaughters,
        )
    elif fitter == "Rave":
        vertex.raveFit(
            f"K_S0:RD{suffix}",
            conf_level=0.0,
            path=path,
            silence_warning=True,
            daughtersUpdate=updateAllDaughters,
        )
    ma.applyCuts(f"K_S0:RD{suffix}", "0.450 < M < 0.550", path=path)
    # Create merged list based on provided priority
    ma.mergeListsWithBestDuplicate(
        f"K_S0:merged{suffix}",
        [f"K_S0:V0_ToFit{suffix}", f"K_S0:RD{suffix}"],
        variable="particleSource",
        preferLowest=prioritiseV0,
        path=path,
    )


def goodBelleKshort(path):
    """
    Load the Belle goodKshort list. Creates a ParticleList named
    ``K_S0:legacyGoodKS``. A vertex fit is performed and only candidates that
    satisfy the :b2:var:`goodBelleKshort` criteria, with an invariant mass in the range
    :math:`0.468 < M < 0.528~GeV` after the vertex fit, and for which the vertex fit did not fail, are kept.

    Parameters:
        path (basf2.Path): the path to load the modules
    """
    if b2bii.isB2BII():
        ma.cutAndCopyList(
            "K_S0:legacyGoodKS", "K_S0:mdst", "0.3 < M < 0.7", writeOut=True, path=path
        )
    else:
        ma.fillParticleList(
            "K_S0:legacyGoodKS -> pi+ pi-", "0.3 < M < 0.7", writeOut=True, path=path
        )
    vertex.kFit("K_S0:legacyGoodKS", conf_level=0.0, path=path)
    ma.applyCuts(
        "K_S0:legacyGoodKS", "0.468 < M < 0.528 and goodBelleKshort==1", path=path
    )


def scaleErrorKshorts(
    prioritiseV0=True,
    fitter="TreeFit",
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
    addSuffix=False,
    path=None,
):
    """
    Reconstruct K_S0 applying helix error correction to K_S0 daughters given by ``modularAnalysis.scaleError``.
    The ParticleList is named ``K_S0:scaled`` by default. If ``addSuffix`` is set to True,
    then a suffix of form ``_<fitter>`` is added.

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
    @param addSuffix            Whether to add a suffix of form ``_<fitter>`` to the ParticleList name
                                depending on the chosen fitter

    """
    from basf2 import register_module

    suffix = ""
    if addSuffix:
        suffix = f"_{fitter}"

    # Load K_S0 from V0 and apply helix error correction to V0 daughters
    ma.fillParticleList(f"K_S0:V0{suffix} -> pi+ pi-", "", True, path=path)
    scaler_V0 = register_module("HelixErrorScaler")
    scaler_V0.set_name("ScaleError_" + f"K_S0:V0{suffix}")
    scaler_V0.param("inputListName", f"K_S0:V0{suffix}")
    scaler_V0.param("outputListName", f"K_S0:V0_scaled{suffix}")
    scaler_V0.param("scaleFactors_PXD", scaleFactors_V0)
    scaler_V0.param("scaleFactors_noPXD", scaleFactorsNoPXD_V0)
    scaler_V0.param("d0ResolutionParameters", d0Resolution_V0)
    scaler_V0.param("z0ResolutionParameters", z0Resolution_V0)
    scaler_V0.param("d0MomentumThreshold", d0MomThr_V0)
    scaler_V0.param("z0MomentumThreshold", z0MomThr_V0)
    path.add_module(scaler_V0)

    # Perform vertex fit and apply tighter mass window
    if fitter == "TreeFit":
        vertex.treeFit(f"K_S0:V0_scaled{suffix}", conf_level=0.0, path=path)
    elif fitter == "KFit":
        vertex.kFit(f"K_S0:V0_scaled{suffix}", conf_level=0.0, path=path)
    elif fitter == "Rave":
        vertex.raveFit(
            f"K_S0:V0_scaled{suffix}", conf_level=0.0, path=path, silence_warning=True
        )
    else:
        B2ERROR(
            "Valid fitter options for Kshorts are 'TreeFit', 'KFit', and 'Rave'. However, the latter is not recommended."
        )
    ma.applyCuts(f"K_S0:V0_scaled{suffix}", "0.450 < M < 0.550", path=path)

    # Reconstruct a second list
    stdPi("all", path=path)
    ma.scaleError(
        "pi+:scaled",
        "pi+:all",
        scaleFactors=scaleFactors_RD,
        scaleFactorsNoPXD=scaleFactorsNoPXD_RD,
        d0Resolution=d0Resolution_RD,
        z0Resolution=z0Resolution_RD,
        d0MomThr=d0MomThr_RD,
        z0MomThr=z0MomThr_RD,
        path=path,
    )

    ma.reconstructDecay(
        f"K_S0:RD_scaled{suffix} -> pi+:scaled pi-:scaled", "0.3 < M < 0.7", 1, True, path=path
    )
    # Again perform vertex fit and apply tighter mass window
    if fitter == "TreeFit":
        vertex.treeFit(f"K_S0:RD_scaled{suffix}", conf_level=0.0, path=path)
    elif fitter == "KFit":
        vertex.kFit(f"K_S0:RD_scaled{suffix}", conf_level=0.0, path=path)
    elif fitter == "Rave":
        vertex.raveFit(
            f"K_S0:RD_scaled{suffix}", conf_level=0.0, path=path, silence_warning=True
        )
    ma.applyCuts(f"K_S0:RD_scaled{suffix}", "0.450 < M < 0.550", path=path)
    # Create merged list based on provided priority
    ma.mergeListsWithBestDuplicate(
        f"K_S0:scaled{suffix}",
        [f"K_S0:V0_scaled{suffix}", f"K_S0:RD_scaled{suffix}"],
        variable="particleSource",
        preferLowest=prioritiseV0,
        path=path,
    )


def stdLambdas(
    prioritiseV0=True,
    fitter="TreeFit",
    path=None,
    updateAllDaughters=False,
    writeOut=False,
    addSuffix=False,
):
    """
    Load a combined list of the Lambda list from V0 objects merged with
    a list of particles combined using the analysis ParticleCombiner module.

    The ParticleList is named ``Lambda0:merged`` by default. If ``addSuffix`` is set to True,
    then a suffix of form ``_<fitter>`` is added depending on the chosen fitter.
    A vertex fit is performed and only candidates with an invariant mass in the
    range :math:`1.10 < M < 1.13~GeV` after the vertex fit,
    and for which the vertex fit did not fail, are kept.

    The vertex fitter can be selected among ``TreeFit``, ``KFit``, and ``Rave``.

    Parameters:
        prioritiseV0 (bool): should the V0 mdst objects be prioritised when merging?
        fitter (str): vertex fitter name, valid options are ``TreeFit``, ``KFit``, and ``Rave``.
        path (basf2.Path): the path to load the modules
        updateAllDaughters (bool): see the ``updateAllDaughters`` parameter of `vertex.treeFit`
            or the ``daughtersUpdate`` parameter of `vertex.kFit` / `vertex.raveFit`.

            .. warning:: The momenta of the daughters are updated only if ``updateAllDaughters`` is set
                to ``True`` (i.e. **not** by default). Some variables, e.g. `daughterAngle`, will only
                return meaningful results if the daughters momenta are updated.

                This happens because variables like `daughterAngle` assume the direction of the
                daughers momenta *at the Lambda vertex* to be provided, while non-updated daughters
                will provide their momenta direction at the point-of-closest-approach (POCA) to the
                beam axis.

        writeOut (bool): whether RootOutput module should save the created ParticleList
        addSuffix (bool): whether to add a suffix of form ``_<fitter>`` to the ParticleList name
            depending on the chosen fitter
    """
    suffix = ""
    if addSuffix:
        suffix = f"_{fitter}"
    # Fill one list from V0
    ma.fillParticleList(
        f"Lambda0:V0_ToFit{suffix} -> p+ pi-", "", writeOut=writeOut, path=path
    )
    # Perform vertex fit and apply tighter mass window
    if fitter == "TreeFit":
        vertex.treeFit(
            f"Lambda0:V0_ToFit{suffix}",
            conf_level=0.0,
            path=path,
            updateAllDaughters=updateAllDaughters,
        )
    elif fitter == "KFit":
        vertex.kFit(
            f"Lambda0:V0_ToFit{suffix}",
            conf_level=0.0,
            path=path,
            daughtersUpdate=updateAllDaughters,
        )
    elif fitter == "Rave":
        vertex.raveFit(
            f"Lambda0:V0_ToFit{suffix}",
            conf_level=0.0,
            path=path,
            silence_warning=True,
            daughtersUpdate=updateAllDaughters,
        )
    else:
        B2ERROR(
            "Valid fitter options for Lambdas are 'TreeFit', 'KFit', and 'Rave'. However, the latter is not recommended."
        )
    ma.applyCuts(f"Lambda0:V0_ToFit{suffix}", "1.10 < M < 1.13", path=path)
    # Find V0 duplicate with better vertex fit quality
    ma.markDuplicate(f"Lambda0:V0_ToFit{suffix}", False, path=path)
    ma.applyCuts(f"Lambda0:V0_ToFit{suffix}", "extraInfo(highQualityVertex)", path=path)
    # Reconstruct a second list
    stdPi("all", path=path, writeOut=writeOut)  # no quality cuts
    stdPr("all", path=path, writeOut=writeOut)  # no quality cuts
    ma.reconstructDecay(
        f"Lambda0:RD{suffix} -> p+:all pi-:all",
        "0.9 < M < 1.3",
        1,
        writeOut=writeOut,
        path=path,
    )
    # Again perform vertex fit and apply tighter mass window
    if fitter == "TreeFit":
        vertex.treeFit(
            f"Lambda0:RD{suffix}",
            conf_level=0.0,
            path=path,
            updateAllDaughters=updateAllDaughters,
        )
    elif fitter == "KFit":
        vertex.kFit(
            f"Lambda0:RD{suffix}",
            conf_level=0.0,
            path=path,
            daughtersUpdate=updateAllDaughters,
        )
    elif fitter == "Rave":
        vertex.raveFit(
            f"Lambda0:RD{suffix}",
            conf_level=0.0,
            path=path,
            silence_warning=True,
            daughtersUpdate=updateAllDaughters,
        )
    ma.applyCuts(f"Lambda0:RD{suffix}", "1.10 < M < 1.13", path=path)
    # Find RD duplicate with better vertex fit quality
    ma.markDuplicate(f"Lambda0:RD{suffix}", False, path=path)
    ma.applyCuts(f"Lambda0:RD{suffix}", "extraInfo(highQualityVertex)", path=path)
    ma.mergeListsWithBestDuplicate(
        f"Lambda0:merged{suffix}",
        [f"Lambda0:V0_ToFit{suffix}", f"Lambda0:RD{suffix}"],
        variable="particleSource",
        preferLowest=prioritiseV0,
        path=path,
    )
