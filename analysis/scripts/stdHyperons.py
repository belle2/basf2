#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from basf2 import *
from modularAnalysis import *
from variables import variables

from stdCharged import stdPi, stdK
from stdV0s import stdLambdas
from stdPhotons import stdPhotons
from stdPi0s import stdPi0s


def stdXi(fitter='kfitter', b2bii=False, path=None):
    """
    Reconstruct the standard :math:`\Xi^-` ``ParticleList`` named ``Xi-:std``.

    .. seealso:: `BELLE2-NOTE-PH-2019-011 <https://docs.belle2.org/record/BELLE2-NOTE-PH-2019-011.pdf>`_.

    Parameters:
    fitter (str): specify either ``kfitter`` or ``treefit`` for the vertex reconstructions (default ``kfitter``)
    b2bii (bool): specify Belle or Belle II reconstruction
    path (basf2.Path): modules are added to this path building the ``Xi-:std`` list
    """

    if not b2bii:
        stdLambdas(path=path)
        # 3.5 MeV Range around the nominal mass
        cutAndCopyList(
            'Lambda0:reco',
            'Lambda0:merged',
            '[ abs( M - 1.115683 ) < 0.0035 ] and \
    [ cosAngleBetweenMomentumAndVertexVector > 0.0 ] and \
    [ daughter(0,protonID) > 0.01 ] and \
    [ chiProb > 0.0 ]',
            True, path=path)
    elif b2bii:
        stdPi('all', path=path)
        # Rough Lambda0 cuts from J. Yelton Observations of an Excited Omega- Baryon
        vertexKFit('Lambda0:mdst', conf_level=0.0, path=path)  # Re-vertexing, recover vertex variables and error matrix
        cutAndCopyList(
            'Lambda0:reco',
            'Lambda0:mdst',
            '[ abs( M - 1.115683 ) < 0.0035 ] and \
   [ cosAngleBetweenMomentumAndVertexVector > 0.0 ] and \
   [ formula( [ x^2 + y^2 ]^[0.5] ) > 0.35 ] and \
   [ daughter(0,atcPIDBelle(4,3)) > 0.2 ] and \
   [ daughter(0,atcPIDBelle(4,2)) > 0.2 ] and \
   [ chiProb > 0.0 ]',
            True, path=path)

    # stdXi-
    if fitter == 'kfitter':
        massVertexKFit('Lambda0:reco', 0.0, '', path=path)
        reconstructDecay('Xi-:reco -> Lambda0:reco pi-:all', '1.295 < M < 1.35', path=path)
        vertexKFit('Xi-:reco', conf_level=0.0, path=path)
    elif fitter == 'treefitter':
        reconstructDecay('Xi-:reco -> Lambda0:reco pi-:all', '1.295 < M < 1.35', path=path)
        vertexTree('Xi-:reco', conf_level=0.0, massConstraint=[3122], path=path)
    else:
        print("WARNING: fitter == kfitter or fitter == treefitter")

    cutAndCopyList(
        'Xi-:std',
        'Xi-:reco',
        '[ cosAngleBetweenMomentumAndVertexVector > 0.0 ] and \
       [ formula( [ x^2 + y^2 + z^2 ]^[0.5] ) > 0. and \
       formula( [ x^2 + y^2 + z^2 ]^[0.5] ) < formula( [ daughter(0,x)^2 + daughter(0,y)^2 + daughter(0,z)^2 ]^[0.5] ) ] and \
       [ chiProb > 0.0 ]',
        True,
        path=path)


def stdXi0(gammatype='eff40', b2bii=False, path=None):
    """
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
            '[ abs( M - 1.115683 ) < 0.0035 ] and \
       [ cosAngleBetweenMomentumAndVertexVector > 0.0 ] and \
       [ daughter(0,protonID) > 0.01 ] and \
       [ chiProb > 0.0 ]',
            True, path=path)
        # 7*sigma_core Range around nominal mass for sigma_core~7.8MeV
        stdPhotons(f'pi0{gammatype}', path=path)
        reconstructDecay(f'pi0:reco -> gamma:pi0{gammatype} gamma:pi0{gammatype}',
                         'abs( M - 0.134977 ) < 0.0546',
                         True, path=path)

    elif b2bii:
        # Rough pi0/Lambda0 cuts from J. Yelton Observations of an Excited Omega- Baryon
        cutAndCopyList(
            'pi0:reco',
            'pi0:mdst',
            '[ abs( M - 0.134977 ) < 0.0189 ] and \
   [ [ daughter(0,clusterReg) == 1 and daughter(0,E) > 0.05 ] or [ daughter(0,clusterReg) == 3 and daughter(0,E) > 0.05 ]  or \
   [ daughter(0,clusterReg) == 2 and  daughter(0,E) > 0.03 ] ] and \
   [ [ daughter(1,clusterReg) == 1 and daughter(1,E) > 0.05 ] or [ daughter(1,clusterReg) == 3 and daughter(1,E) > 0.05 ]  or \
   [ daughter(1,clusterReg) == 2 and  daughter(1,E) > 0.03 ] ]',
            path=path)
        vertexKFit('Lambda0:mdst', conf_level=0.0, path=path)  # Re-vertexing, recover vertex variables and error matrix
        cutAndCopyList(
            'Lambda0:reco',
            'Lambda0:mdst',
            '[ abs( M - 1.115683 ) < 0.0035 ] and \
   [ cosAngleBetweenMomentumAndVertexVector > 0.0 ] and \
   [ formula( [ x^2 + y^2 ]^[0.5] ) > 0.35 ] and \
   [ daughter(0,atcPIDBelle(4,3)) > 0.2 ] and \
   [ daughter(0,atcPIDBelle(4,2)) > 0.2 ] and \
   [ chiProb > 0.0 ]',
            True, path=path)

    reconstructDecay(
        'Xi0:prelim -> Lambda0:reco pi0:reco',
        '1.225 < M < 1.405',
        path=path)
    vertexTree('Xi0:prelim', conf_level=0.0, massConstraint=[3122], ipConstraint=True, updateAllDaughters=True, path=path)
    # Reconstructed core resolution pi0~7.8 MeV selecting 3*sigma_core around the nominal mass
    # pi0 mass range is invariant for b2bii=True, tighter selection is required by user
    applyCuts('Xi0:prelim', '[ daughter(1,M) > 0.111577 and daughter(1,M) < 0.158377 ]', path=path)
    vertexTree('Xi0:prelim', conf_level=0.0, massConstraint=[111, 3122], ipConstraint=True, updateAllDaughters=False, path=path)

    cutAndCopyList(
        'Xi0:std',
        'Xi0:prelim',
        '[ cosAngleBetweenMomentumAndVertexVector > 0.0 ] and \
         [ daughter(0,cosAngleBetweenMomentumAndVertexVectorInXYPlane) < cosAngleBetweenMomentumAndVertexVectorInXYPlane ] and \
         [ formula( [ x^2 + y^2 + z^2 ]^[0.5] ) > 0.0 and \
         formula( [ x^2 + y^2 + z^2 ]^[0.5] ) < formula( [ daughter(0,x)^2 + daughter(0,y)^2 + daughter(0,z)^2 ]^[0.5] ) ] and \
         [ chiProb > 0.0 ]',
        True,
        path=path)


def stdOmega(fitter='kfitter', b2bii=False, path=None):
    """
    Reconstruct the standard :math:`\Omega^-` ``ParticleList`` named ``Omega-:std``.

    .. seealso:: `BELLE2-NOTE-PH-2019-011 <https://docs.belle2.org/record/BELLE2-NOTE-PH-2019-011.pdf>`_.

    Parameters:
    fitter (str): specify either ``kfitter`` or ``treefit`` for the vertex reconstructions (default ``kfitter``)
    b2bii (bool): specify Belle or Belle II reconstruction
    path (basf2.Path): modules are added to this path building the ``Omega-:std`` list
    """

    if not b2bii:
        stdLambdas(path=path)
        # 3.5 MeV Range around the nominal mass
        cutAndCopyList(
            'Lambda0:reco',
            'Lambda0:merged',
            '[ abs( M - 1.115683 ) < 0.0035 ] and \
    [ cosAngleBetweenMomentumAndVertexVector > 0.0 ] and \
    [ daughter(0,protonID) > 0.01 ] and \
    [ chiProb > 0.0 ]',
            True, path=path)
    elif b2bii:
        stdPi('all', path=path)
        # Rough Lambda0 cuts from J. Yelton Observations of an Excited Omega- Baryon
        vertexKFit('Lambda0:mdst', conf_level=0.0, path=path)  # Re-vertexing, recover vertex variables and error matrix
        cutAndCopyList(
            'Lambda0:reco',
            'Lambda0:mdst',
            '[ abs( M - 1.115683 ) < 0.0035 ] and \
   [ cosAngleBetweenMomentumAndVertexVector > 0.0 ] and \
   [ formula( [ x^2 + y^2 ]^[0.5] ) > 0.35 ] and \
   [ daughter(0,atcPIDBelle(4,3)) > 0.2 ] and \
   [ daughter(0,atcPIDBelle(4,2)) > 0.2 ] and \
   [ chiProb > 0.0 ]',
            True, path=path)

    stdK('all', path=path)
    # stdOmega-
    if fitter == 'kfitter':
        massVertexKFit('Lambda0:reco', 0.0, '', path=path)
        reconstructDecay('Omega-:reco -> Lambda0:reco K-:all', '1.622 < M < 1.722', path=path)
        vertexKFit('Omega-:reco', conf_level=0.0, path=path)
    elif fitter == 'treefitter':
        reconstructDecay('Omega-:reco -> Lambda0:reco K-:all', '1.622 < M < 1.722', path=path)
        vertexTree('Omega-:reco', conf_level=0.0, massConstraint=[3122], path=path)
    else:
        print("WARNING: fitter == kfitter or fitter == treefitter")

    if not b2bii:
        cutAndCopyList(
            'Omega-:std',
            'Omega-:reco',
            '[ cosAngleBetweenMomentumAndVertexVector > 0.0] and \
       [ formula( [ x^2 + y^2 + z^2 ]^[0.5] ) > 0. and \
       formula( [ x^2 + y^2 + z^2 ]^[0.5] ) < formula( [ daughter(0,x)^2 + daughter(0,y)^2 + daughter(0,z)^2 ]^[0.5] ) ] and \
       [ chiProb > 0.0 ] and \
       [ daughter(1,kaonID) > 0.01 ]',
            True,
            path=path)

    elif b2bii:
        cutAndCopyList(
            'Omega-:std',
            'Omega-:reco',
            '[ cosAngleBetweenMomentumAndVertexVector > 0.0 ] and \
       [ formula( [ x^2 + y^2 + z^2 ]^[0.5] ) > 0. and \
       formula( [ x^2 + y^2 + z^2 ]^[0.5] ) < formula( [ daughter(0,x)^2 + daughter(0,y)^2 + daughter(0,z)^2 ]^[0.5] ) ] and \
       [ chiProb > 0.0 ] and \
       [ daughter(1,atcPIDBelle(3,4)) > 0.2 and daughter(1,atcPIDBelle(3,2)) > 0.2 ]',
            True,
            path=path)


def goodXi(xitype='loose', path=None):
    """
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
            '[ formula( [ x^2 + y^2 + z^2 ]^[0.5] ) > 0.1 ]',
            True,
            path=path)

    elif xitype == 'loose':
        cutAndCopyList(
            'Xi-:loose',
            'Xi-:std',
            '[ formula( [ x^2 + y^2 + z^2 ]^[0.5] ) > 0.35 ]',
            True,
            path=path)

    elif xitype == 'tight':
        cutAndCopyList(
            'Xi-:tight',
            'Xi-:std',
            '[ formula([daughter(0,cosAngleBetweenMomentumAndVertexVector)/cosAngleBetweenMomentumAndVertexVector])<1.00085 ] and \
       [ formula( [ x^2 + y^2 + z^2 ]^[0.5] ) > 0.35 ]',
            True,
            path=path)


def goodXi0(xitype='loose', path=None):
    """
    Select the standard good :math:`\Xi^0` ``ParticleList`` named ``Xi0:veryloose``, ``Xi0:loose``, or ``Xi0:tight``
    from the reconstructed ``Xi0:std``.

    .. seealso:: `BELLE2-NOTE-PH-2019-011 <https://docs.belle2.org/record/BELLE2-NOTE-PH-2019-011.pdf>`_.

    Parameters:
    xitype (str): specify either ``veryloose``, ``loose``,  or ``tight`` for good ``ParticleList`` selection (default ``loose``)
    path (basf2.Path): modules are added to this path building the ``Xi0:veryloose``, ``Xi0:loose``, or ``Xi0:tight``, list
    """

    if xitype == 'veryloose':
        # Reconstructed core resolution pi0~7.8 MeV, cut at 3*sigma_core around the nomin"al mass
        cutAndCopyList(
            'Xi0:veryloose',
            'Xi0:std',
            '[ daughter(1,p) > 0.150 ] and \
         [ formula( [ x^2 + y^2 + z^2 ]^[0.5] ) > 0.25 ] and \
         [ abs( daughter(1,M) - 0.134977 ) < 0.0234 ]',
            True,
            path=path)

    elif xitype == 'loose':
        # Reconstructed core resolution pi0~7.8 MeV, cut at 3*sigma_core around the nominal mass
        cutAndCopyList(
            'Xi0:loose',
            'Xi0:std',
            '[ daughter(1,p) > 0.150 ] and \
         [ formula( [ x^2 + y^2 + z^2 ]^[0.5] ) > 1.5 ] and \
         [ abs( daughter(1,M) - 0.134977 ) < 0.0234 ]',
            True,
            path=path)

    elif xitype == 'tight':
        # Reconstructed core resolution pi0~7.8 MeV, cut at 2*sigma_core around the nominal mass
        cutAndCopyList(
            'Xi0:tight',
            'Xi0:std',
            '[ daughter(1,p) > 0.150 ] and \
         [ formula( [ x^2 + y^2 + z^2 ]^[0.5] ) > 1.5 ] and \
         [ abs( daughter(1,M) - 0.134977 ) < 0.0156 ]',
            True,
            path=path)


def goodOmega(omegatype='veryloose', path=None):
    """
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
            '[ daughter(1,p) > 0.175 ] and \
       [ formula( [ x^2 + y^2 + z^2 ]^[0.5] ) > 0.1 ]',
            True,
            path=path)

    elif omegatype == 'loose':
        cutAndCopyList(
            'Omega-:loose',
            'Omega-:std',
            '[ daughter(1,p) > 0.275 ] and \
       [ formula( [ x^2 + y^2 + z^2 ]^[0.5] ) > 0.5 ]',
            True,
            path=path)

    elif omegatype == 'tight':
        cutAndCopyList(
            'Omega-:tight',
            'Omega-:std',
            '[ daughter(1,p) > 0.275 ] and \
       [ formula( [ daughter(0,cosAngleBetweenMomentumAndVertexVector) / cosAngleBetweenMomentumAndVertexVector ] ) < 1.0001 ] and \
       [ formula( [ x^2 + y^2 + z^2 ]^[0.5] ) > 0.5 ]',
            True,
            path=path)
