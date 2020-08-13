#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from basf2 import B2ERROR
from modularAnalysis import cutAndCopyList, reconstructDecay, applyCuts
from vertex import treeFit, kFit

from stdCharged import stdPi, stdK
from stdV0s import stdLambdas
from stdPhotons import stdPhotons
from stdPi0s import stdPi0s


def stdXi(fitter='TreeFit', b2bii=False, path=None):
    r"""
    Reconstruct the standard :math:`\Xi^-` ``ParticleList`` named ``Xi-:std``.

    .. seealso:: `BELLE2-NOTE-PH-2019-011 <https://docs.belle2.org/record/BELLE2-NOTE-PH-2019-011.pdf>`_.

    Parameters:
        fitter (str): specify either ``KFit`` or ``TreeFit`` for the vertex reconstructions (default ``TreeFit``)
        b2bii (bool): specify Belle or Belle II reconstruction
        path (basf2.Path): modules are added to this path building the ``Xi-:std`` list
    """

    if not b2bii:
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
    elif b2bii:
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


def stdXi0(gammatype='eff40', b2bii=False, path=None):
    r"""
    Reconstruct the standard :math:`\Xi^0` ``ParticleList`` named ``Xi0:std``.

    .. seealso:: `BELLE2-NOTE-PH-2019-011 <https://docs.belle2.org/record/BELLE2-NOTE-PH-2019-011.pdf>`_.

    Parameters:
        gammatype (str): specify either ``eff60``, ``eff50``, ``eff40``, ``eff30``, or ``eff20``
                         for the pi0 reconstruction from ``gamma:pi0effxx`` (default ``eff40``)
        b2bii (bool): specify Belle or Belle II reconstruction
        path (basf2.Path): modules are added to this path building the ``Xi0:std`` list
    """

    if not b2bii:
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
        stdPhotons(f'pi0{gammatype}', path=path)
        reconstructDecay(f'pi0:reco -> gamma:pi0{gammatype} gamma:pi0{gammatype}',
                         'abs( dM ) < 0.0406',
                         True, path=path)

    elif b2bii:
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
    # pi0 mass range is invariant for b2bii=True, tighter selection is required by user
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


def stdOmega(fitter='TreeFit', b2bii=False, path=None):
    r"""
    Reconstruct the standard :math:`\Omega^-` ``ParticleList`` named ``Omega-:std``.

    .. seealso:: `BELLE2-NOTE-PH-2019-011 <https://docs.belle2.org/record/BELLE2-NOTE-PH-2019-011.pdf>`_.

    Parameters:
        fitter (str): specify either ``KFit`` or ``TreeFit`` for the vertex reconstructions (default ``TreeFit``)
        b2bii (bool): specify Belle or Belle II reconstruction
        path (basf2.Path): modules are added to this path building the ``Omega-:std`` list
    """

    if not b2bii:
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
    elif b2bii:
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

    if not b2bii:
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

    elif b2bii:
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


def goodXi0(xitype='loose', path=None):
    r"""
    Select the standard good :math:`\Xi^0` ``ParticleList`` named ``Xi0:veryloose``, ``Xi0:loose``, or ``Xi0:tight``
    from the reconstructed ``Xi0:std``.

    .. seealso:: `BELLE2-NOTE-PH-2019-011 <https://docs.belle2.org/record/BELLE2-NOTE-PH-2019-011.pdf>`_.

    Parameters:
        xitype (str): specify either ``veryloose``, ``loose``,  or ``tight`` for good ``ParticleList`` selection (default ``loose``)
        path (basf2.Path): modules are added to this path building the ``Xi0:veryloose``, ``Xi0:loose``, or ``Xi0:tight``, list
    """

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
