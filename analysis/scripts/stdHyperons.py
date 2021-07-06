#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import B2ERROR, B2WARNING
from b2bii import isB2BII
from modularAnalysis import cutAndCopyList, reconstructDecay, applyCuts
from vertex import treeFit, kFit

from stdCharged import stdPi, stdK
from stdV0s import stdLambdas
from stdPhotons import stdPhotons


def stdXi(fitter='TreeFit', path=None):
    r"""
    Reconstruct the standard :math:`\Xi^-` ``ParticleList`` named ``Xi-:std``.

    .. seealso:: `BELLE2-NOTE-PH-2019-011 <https://docs.belle2.org/record/BELLE2-NOTE-PH-2019-011.pdf>`_.

    Parameters:
        fitter (str): specify either ``KFit`` or ``TreeFit`` for the vertex reconstructions (default ``TreeFit``)
        path (basf2.Path): modules are added to this path building the ``Xi-:std`` list
    """

    if not isB2BII():
        stdLambdas(path=path)
        # 3.5 MeV Range around the nominal mass
        cutAndCopyList(
            'Lambda0:reco',
            'Lambda0:merged',
            '[ abs( dM ) < 0.0035 ] and \
            [ cosAngleBetweenMomentumAndVertexVector > 0.0 ] and \
            [ formula( [ dr^2 + dz^2 ]^[0.5] ) > 0.35 ] and \
            [ daughter(0,protonID) > 0.01 ] and \
            [ chiProb > 0.0 ]',
            True, path=path)
    else:
        stdPi('all', path=path)
        # Rough Lambda0 cuts from J. Yelton Observations of an Excited Omega- Baryon
        kFit('Lambda0:mdst', conf_level=0.0, path=path)  # Re-vertexing, recover vertex variables and error matrix
        cutAndCopyList(
            'Lambda0:reco',
            'Lambda0:mdst',
            '[ abs( dM ) < 0.0035 ] and \
            [ cosAngleBetweenMomentumAndVertexVector > 0.0 ] and \
            [ dr > 0.35 ] and \
            [ daughter(0,atcPIDBelle(4,3)) > 0.2 ] and \
            [ daughter(0,atcPIDBelle(4,2)) > 0.2 ] and \
            [ chiProb > 0.0 ]',
            True, path=path)

    # stdXi-
    if fitter == 'KFit':
        kFit('Lambda0:reco', 0.0, fit_type='massvertex', path=path)
        reconstructDecay('Xi-:reco -> Lambda0:reco pi-:all', '1.295 < M < 1.35', path=path)
        kFit('Xi-:reco', conf_level=0.0, path=path)
    elif fitter == 'TreeFit':
        reconstructDecay('Xi-:reco -> Lambda0:reco pi-:all', '1.295 < M < 1.35', path=path)
        treeFit('Xi-:reco', conf_level=0.0, massConstraint=[3122], path=path)
    else:
        B2ERROR(f"stdXi: invalid fitter ({fitter}). Choose from KFit or TreeFit")

    cutAndCopyList(
        'Xi-:std',
        'Xi-:reco',
        '[ cosAngleBetweenMomentumAndVertexVector > 0.0 ] and \
        [ formula( [ dr^2 + dz^2 ]^[0.5] ) > 0. and \
        formula([dr^2 + dz^2 ]^[0.5])<formula([daughter(0,dr)^2 + daughter(0,dz)^2]^[0.5])] and \
        [ chiProb > 0.0 ]',
        True,
        path=path)


def stdXi0(gammatype='eff40', path=None):
    r"""
    Reconstruct the standard :math:`\Xi^0` ``ParticleList`` named ``Xi0:std``.

    .. seealso:: `BELLE2-NOTE-PH-2019-011 <https://docs.belle2.org/record/BELLE2-NOTE-PH-2019-011.pdf>`_.

    Parameters:
        gammatype (str): specify either ``eff60``, ``eff50``, ``eff40``, ``eff30``, or ``eff20``
                         to select the signal efficiency of the photons used in the pi0 reconstruction
                         (default ``eff40``)
        path (basf2.Path): modules are added to this path building the ``Xi0:std`` list
    """

    if not isB2BII():
        stdLambdas(path=path)
        # 3.5 MeV Range around nominal mass (~7*sigma_core)
        cutAndCopyList(
            'Lambda0:reco',
            'Lambda0:merged',
            '[ abs( dM ) < 0.0035 ] and \
            [ cosAngleBetweenMomentumAndVertexVector > 0.0 ] and \
            [ formula( [ dr^2 + dz^2 ]^[0.5] ) > 0.35 ] and \
            [ daughter(0,protonID) > 0.01 ] and \
            [ chiProb > 0.0 ]',
            True, path=path)
        # ~7*sigma Range around nominal mass
        stdPhotons(f'pi0{gammatype}_May2020', path=path)
        reconstructDecay(f'pi0:reco -> gamma:pi0{gammatype}_May2020 gamma:pi0{gammatype}_May2020',
                         'abs( dM ) < 0.0406',
                         True, path=path)

    else:
        # Rough pi0/Lambda0 cuts from J. Yelton Observations of an Excited Omega- Baryon
        cutAndCopyList(
            'pi0:reco',
            'pi0:mdst',
            '[ abs( dM ) < 0.0189 ] and \
        [ [ daughter(0,clusterReg) == 1 and daughter(0,E) > 0.05 ] or [ daughter(0,clusterReg) == 3 and daughter(0,E) > 0.05 ]  or \
        [ daughter(0,clusterReg) == 2 and  daughter(0,E) > 0.03 ] ] and \
        [ [ daughter(1,clusterReg) == 1 and daughter(1,E) > 0.05 ] or [ daughter(1,clusterReg) == 3 and daughter(1,E) > 0.05 ]  or \
        [ daughter(1,clusterReg) == 2 and  daughter(1,E) > 0.03 ] ]',
            path=path)
        kFit('Lambda0:mdst', conf_level=0.0, path=path)  # Re-vertexing, recover vertex variables and error matrix
        cutAndCopyList(
            'Lambda0:reco',
            'Lambda0:mdst',
            '[ abs( dM ) < 0.0035 ] and \
            [ cosAngleBetweenMomentumAndVertexVector > 0.0 ] and \
            [ dr > 0.35 ] and \
            [ daughter(0,atcPIDBelle(4,3)) > 0.2 ] and \
            [ daughter(0,atcPIDBelle(4,2)) > 0.2 ] and \
            [ chiProb > 0.0 ]',
            True, path=path)

    reconstructDecay(
        'Xi0:prelim -> Lambda0:reco pi0:reco',
        '1.225 < M < 1.405',
        path=path)
    treeFit('Xi0:prelim', conf_level=0.0, massConstraint=[3122], ipConstraint=True, updateAllDaughters=True, path=path)
    # Selecting ~4*sigma around the pi0 nominal mass
    # pi0 mass range is invariant for B2BII, tighter selection is required by user
    applyCuts('Xi0:prelim', '[ abs( daughter(1,dM) ) < 0.0232 ]', path=path)
    treeFit('Xi0:prelim', conf_level=0.0, massConstraint=[111, 3122], ipConstraint=True, updateAllDaughters=False, path=path)

    cutAndCopyList(
        'Xi0:std',
        'Xi0:prelim',
        '[ cosAngleBetweenMomentumAndVertexVector > 0.0 ] and \
         [ daughter(0,cosAngleBetweenMomentumAndVertexVectorInXYPlane) < cosAngleBetweenMomentumAndVertexVectorInXYPlane ] and \
         [ formula( [ dr^2 + dz^2 ]^[0.5] ) > 0.0 and \
         formula([dr^2 + dz^2]^[0.5])<formula([daughter(0,dr)^2 + daughter(0,dz)^2]^[0.5])] and \
         [ chiProb > 0.0 ]',
        True,
        path=path)


def stdOmega(fitter='TreeFit', path=None):
    r"""
    Reconstruct the standard :math:`\Omega^-` ``ParticleList`` named ``Omega-:std``.

    .. seealso:: `BELLE2-NOTE-PH-2019-011 <https://docs.belle2.org/record/BELLE2-NOTE-PH-2019-011.pdf>`_.

    Parameters:
        fitter (str): specify either ``KFit`` or ``TreeFit`` for the vertex reconstructions (default ``TreeFit``)
        path (basf2.Path): modules are added to this path building the ``Omega-:std`` list
    """

    if not isB2BII():
        stdLambdas(path=path)
        # 3.5 MeV Range around the nominal mass
        cutAndCopyList(
            'Lambda0:reco',
            'Lambda0:merged',
            '[ abs( dM ) < 0.0035 ] and \
            [ cosAngleBetweenMomentumAndVertexVector > 0.0 ] and \
            [ formula( [ dr^2 + dz^2 ]^[0.5] ) > 0.35 ] and \
            [ daughter(0,protonID) > 0.01 ] and \
            [ chiProb > 0.0 ]',
            True, path=path)
    else:
        stdPi('all', path=path)
        # Rough Lambda0 cuts from J. Yelton Observations of an Excited Omega- Baryon
        kFit('Lambda0:mdst', conf_level=0.0, path=path)  # Re-vertexing, recover vertex variables and error matrix
        cutAndCopyList(
            'Lambda0:reco',
            'Lambda0:mdst',
            '[ abs( dM ) < 0.0035 ] and \
            [ cosAngleBetweenMomentumAndVertexVector > 0.0 ] and \
            [ dr > 0.35 ] and \
            [ daughter(0,atcPIDBelle(4,3)) > 0.2 ] and \
            [ daughter(0,atcPIDBelle(4,2)) > 0.2 ] and \
            [ chiProb > 0.0 ]',
            True, path=path)

    stdK('all', path=path)
    # stdOmega-
    if fitter == 'KFit':
        kFit('Lambda0:reco', 0.0, fit_type='massvertex', path=path)
        reconstructDecay('Omega-:reco -> Lambda0:reco K-:all', '1.622 < M < 1.722', path=path)
        kFit('Omega-:reco', conf_level=0.0, path=path)
    elif fitter == 'TreeFit':
        reconstructDecay('Omega-:reco -> Lambda0:reco K-:all', '1.622 < M < 1.722', path=path)
        treeFit('Omega-:reco', conf_level=0.0, massConstraint=[3122], path=path)
    else:
        B2ERROR(f"stdOmega: invalid fitter ({fitter}). Choose from KFit or TreeFit")

    if not isB2BII():
        cutAndCopyList(
            'Omega-:std',
            'Omega-:reco',
            '[ cosAngleBetweenMomentumAndVertexVector > 0.0] and \
            [ formula( [ dr^2 + dz^2 ]^[0.5] ) > 0. and \
            formula([dr^2 + dz^2]^[0.5])<formula([daughter(0,dr)^2 + daughter(0,dz)^2]^[0.5])] and \
            [ chiProb > 0.0 ] and \
            [ daughter(1,kaonID) > 0.01 ]',
            True,
            path=path)

    else:
        cutAndCopyList(
            'Omega-:std',
            'Omega-:reco',
            '[ cosAngleBetweenMomentumAndVertexVector > 0.0 ] and \
            [ formula( [ dr^2 + dz^2 ]^[0.5] ) > 0. and \
            formula([dr^2 + dz^2]^[0.5])<formula([daughter(0,dr)^2 + daughter(0,dz)^2 ]^[0.5])] and \
            [ chiProb > 0.0 ] and \
            [ daughter(1,atcPIDBelle(3,4)) > 0.2 and daughter(1,atcPIDBelle(3,2)) > 0.2 ]',
            True,
            path=path)


def goodXi(xitype='loose', path=None):
    r"""
    Select the standard good :math:`\Xi^-` ``ParticleList`` named ``Xi-:veryloose``, ``Xi-:loose``, or ``Xi-:tight``
    from the reconstructed ``Xi-:std``.

    .. seealso:: `BELLE2-NOTE-PH-2019-011 <https://docs.belle2.org/record/BELLE2-NOTE-PH-2019-011.pdf>`_.

    Parameters:
        xitype (str): specify either ``veryloose``, ``loose``,  or ``tight`` for good ``ParticleList`` selection (default ``loose``)
        path (basf2.Path): modules are added to this path building the ``Xi-:veryloose``, ``Xi-:loose``, or ``Xi-:tight``, list
    """

    if not _std_hyperon_is_in_path("Xi-", path):
        B2WARNING("Could not find standard Xi particle list! Creating it with default options.")
        stdXi(path=path)
        assert _std_hyperon_is_in_path("Xi-", path)

    if xitype == 'veryloose':
        cutAndCopyList(
            'Xi-:veryloose',
            'Xi-:std',
            '[ daughter(1,pt) > 0.05 and \
            formula( [ dr^2 + dz^2 ]^[0.5] ) > 0.1 ]',
            True,
            path=path)

    elif xitype == 'loose':
        cutAndCopyList(
            'Xi-:loose',
            'Xi-:std',
            '[ daughter(1,pt) > 0.05 and \
            formula( [ dr^2 + dz^2 ]^[0.5] ) > 0.1 and \
            formula([daughter(0,cosAngleBetweenMomentumAndVertexVectorInXYPlane)/cosAngleBetweenMomentumAndVertexVectorInXYPlane])\
<1.006 ]',
            True,
            path=path)

    elif xitype == 'tight':
        cutAndCopyList(
            'Xi-:tight',
            'Xi-:std',
            '[ daughter(1,pt) > 0.1 and \
            formula( [ dr^2 + dz^2 ]^[0.5] ) > 0.15 and \
            formula([daughter(0,cosAngleBetweenMomentumAndVertexVectorInXYPlane)/cosAngleBetweenMomentumAndVertexVectorInXYPlane])\
<1.001 ]',
            True,
            path=path)
    else:
        raise ValueError(f"\"{xitype}\" is none of the allowed Xi- list types!")


def goodXi0(xitype='loose', path=None):
    r"""
    Select the standard good :math:`\Xi^0` ``ParticleList`` named ``Xi0:veryloose``, ``Xi0:loose``, or ``Xi0:tight``
    from the reconstructed ``Xi0:std``.

    .. seealso:: `BELLE2-NOTE-PH-2019-011 <https://docs.belle2.org/record/BELLE2-NOTE-PH-2019-011.pdf>`_.

    Parameters:
        xitype (str): specify either ``veryloose``, ``loose``,  or ``tight`` for good ``ParticleList`` selection (default ``loose``)
        path (basf2.Path): modules are added to this path building the ``Xi0:veryloose``, ``Xi0:loose``, or ``Xi0:tight``, list
    """

    if not _std_hyperon_is_in_path("Xi0", path):
        B2WARNING("Could not find standard Xi0 particle list! Creating it with default options.")
        stdXi0(path=path)
        assert _std_hyperon_is_in_path("Xi0", path)

    if xitype == 'veryloose':
        # Select pi0 at 3*sigma around the nominal mass
        cutAndCopyList(
            'Xi0:veryloose',
            'Xi0:std',
            '[ formula( [ dr^2 + dz^2 ]^[0.5] ) > 0.25 and \
            daughter(1,p) > 0.1 and \
            abs( daughter(1,dM) ) < 0.0174 ]',
            True,
            path=path)

    elif xitype == 'loose':
        # Select pi0 at 3*sigma around the nominal mass
        cutAndCopyList(
            'Xi0:loose',
            'Xi0:std',
            '[ formula( [ dr^2 + dz^2 ]^[0.5] ) > 0.5 and \
            daughter(1,p) > 0.15 and \
            abs( daughter(1,dM) ) < 0.0174 ]',
            True,
            path=path)

    elif xitype == 'tight':
        # Select pi0 at 2*sigma around the nominal mass
        cutAndCopyList(
            'Xi0:tight',
            'Xi0:std',
            '[ formula( [ dr^2 + dz^2 ]^[0.5] ) > 1.4 and \
            daughter(1,p) > 0.25 and \
            abs( daughter(1,dM) ) < 0.0116 ]',
            True,
            path=path)
    else:
        raise ValueError(f"\"{xitype}\" is none of the allowed Xi0 list types!")


def goodOmega(omegatype='loose', path=None):
    r"""
    Select the standard good :math:`\Omega^-` ``ParticleList`` named ``Omega-:veryloose``, ``Omega-:loose``,
    or ``Omega-:tight`` from the reconstructed ``Omega-:std``.

    .. seealso:: `BELLE2-NOTE-PH-2019-011 <https://docs.belle2.org/record/BELLE2-NOTE-PH-2019-011.pdf>`_.

    Parameters:
        omegatype (str): specify either ``veryloose``, ``loose``,  or ``tight`` for good ``ParticleList`` selection
                         (default ``veryloose``)
        path (basf2.Path): modules are added to this path building the ``Omega-:veryloose``, ``Omega-:loose``,
                           or ``Omega-:tight``, list
    """

    if not _std_hyperon_is_in_path("Omega-", path):
        B2WARNING("Could not find standard Omega particle list! Creating it with default options.")
        stdOmega(path=path)
        assert _std_hyperon_is_in_path("Omega-", path)

    if omegatype == 'veryloose':
        cutAndCopyList(
            'Omega-:veryloose',
            'Omega-:std',
            '[ daughter(1,pt) > 0.15 and \
            formula( [ dr^2 + dz^2 ]^[0.5] ) > 0.05 ]',
            True,
            path=path)

    elif omegatype == 'loose':
        cutAndCopyList(
            'Omega-:loose',
            'Omega-:std',
            '[ daughter(1,pt) > 0.15 and \
            formula( [ dr^2 + dz^2 ]^[0.5] ) > 0.15 and \
            formula([daughter(0,cosAngleBetweenMomentumAndVertexVectorInXYPlane)/cosAngleBetweenMomentumAndVertexVectorInXYPlane])\
<1.0015 ]',
            True,
            path=path)

    elif omegatype == 'tight':
        cutAndCopyList(
            'Omega-:tight',
            'Omega-:std',
            '[ daughter(1,pt) > 0.3 and \
            formula( [ dr^2 + dz^2 ]^[0.5] ) > 0.15 and \
            formula([daughter(0,cosAngleBetweenMomentumAndVertexVectorInXYPlane)/cosAngleBetweenMomentumAndVertexVectorInXYPlane])\
<1.0005 ]',
            True,
            path=path)
    else:
        raise ValueError(f"\"{omegatype}\" is none of the allowed Omega list types!")


def _std_hyperon_is_in_path(hyperon, path):
    """
    Helper function to check if the std hyperon is already in the reconstruction path.

    Checks whether there is a ``PListCutAndCopy`` module with the
    ``outputListName``: ``<hyperon>:std``.
    :param hyperon: One of ["Xi-", "Xi0", "Omega-"]
    :param path: Instance of basf2.Path
    :returns: Boolean, whether  ``PListCutAndCopy`` with ``outputListName`` ``<hyperon>:std`` was found in path.
    """
    # maybe this function could be generalized for other standard particles, but
    # so far it has only been tested for standard hyperons:
    allowed_hyperons = {"Xi-", "Xi0", "Omega-"}
    if hyperon not in allowed_hyperons:
        raise ValueError(
            f"\"{hyperon}\" is not in list of hyperons that this function has been tested for ({allowed_hyperons})."
        )
    for module in path.modules():
        if (module.name() == f"PListCutAndCopy_{hyperon}:std" or
                module.name().split(" -> ")[0] == f"ParticleCombiner_{hyperon}:std"):
            return True
    return False
