#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from basf2 import *
from modularAnalysis import *
from variables import variables
from stdCharged import *
from stdV0s import mergedLambdas
from stdPhotons import stdPhotons
from stdPi0s import stdPi0s


def loadStdXi(fitter='kfitter', BELLE=False, path=analysis_main):
    if not BELLE:
        mergedLambdas(path=path)
        # 3.5 MeV Range about the nominal mass
        cutAndCopyList(
            'Lambda0:reco',
            'Lambda0:merged',
            '[ M > 1.112183 and M < 1.119183 ]',
            True, path=path)
    elif BELLE:
        stdPi('all', path=path)
        # Rough Lambda0 cuts from J. Yelton Observations of an Excited Omega- Baryon
        #
        cutAndCopyList(
            'Lambda0:reco',
            'Lambda0:mdst',
            '[ M > 1.112183 and M < 1.119183 ] and \
   [ formula( [ x^2 + y^2 ]^[0.5] ) > 0.35 ] and \
   [ daughter(0,atcPIDBelle(4,3)) > 0.2 ] and \
   [ daughter(0,atcPIDBelle(4,2)) > 0.2 ]',
            True, path=path)

    # stdXi-
    if fitter == 'kfitter':
        massVertexKFit('Lambda0:reco', 0.0, '', path=path)
        reconstructDecay('Xi-:reco -> Lambda0:reco pi-:all', '1.295 < M < 1.35', path=path)
        vertexKFit('Xi-:reco', conf_level=0.0, path=path)
    elif fitter == 'treefitter':
        reconstructDecay('Xi-:reco -> Lambda0:reco pi-:all', '1.295 < M < 1.35', path=path)
        vertexTree('Xi-:reco', conf_level=0.0, massConstraint=[3122], path=path)

    if not BELLE:
        cutAndCopyList(
            'Xi-:std',
            'Xi-:reco',
            '[ daughter(0,daughter(0,p)) > 0.3 and daughter(0,daughter(1,p)) > 0.1 and daughter(1,p) > 0.1 ] and \
       [ daughter(0,cosAngleBetweenMomentumAndVertexVector) > 0. and cosAngleBetweenMomentumAndVertexVector > 0.] and \
       [ formula( [ x^2 + y^2 + z^2 ]^[0.5] ) > 0. and \
       formula( [ x^2 + y^2 + z^2 ]^[0.5] ) < formula( [ daughter(0,x)^2 + daughter(0,y)^2 + daughter(0,z)^2 ]^[0.5] ) ] and \
       [ chiProb > 0.0 ] and \
       [ daughter(0,daughter(0,protonID)) > 0.01 ]',
            True,
            path=path)
    elif BELLE:
        cutAndCopyList(
            'Xi-:std',
            'Xi-:reco',
            '[ daughter(0,cosAngleBetweenMomentumAndVertexVector) > 0. and cosAngleBetweenMomentumAndVertexVector > 0.] and \
   [ formula( [ x^2 + y^2 + z^2 ]^[0.5] ) > 0.35 and \
   formula( [ x^2 + y^2 + z^2 ]^[0.5] ) < formula( [ daughter(0,x)^2 + daughter(0,y)^2 + daughter(0,z)^2 ]^[0.5] ) ] and \
   [ chiProb > 0.0 ]',
            True,
            path=path)

    matchMCTruth('Xi-:std', path=path)


def loadStdXi0(gammatype='eff40', BELLE=False, path=analysis_main):
    if not BELLE:
        mergedLambdas(path=path)
        # 3.5 MeV Range about nominal mass (~7*sigma_core)
        cutAndCopyList(
            'Lambda0:reco',
            'Lambda0:merged',
            '[ M > 1.112183 and M < 1.119183 ]',
            True, path=path)
        # 7*sigma_core Range about nominal mass for sigma_core~7.8MeV
        if gammatype == 'eff60':
            stdPhotons('pi0eff60', path=path)
            reconstructDecay(
                'pi0:reco -> gamma:pi0eff60 gamma:pi0eff60',
                '0.08377 < M < 0.189577',
                True,
                path=path)
        elif gammatype == 'eff50':
            stdPhotons('pi0eff50', path=path)
            reconstructDecay(
                'pi0:reco -> gamma:pi0eff50 gamma:pi0eff50',
                '0.08377 < M < 0.189577',
                True,
                path=path)
        elif gammatype == 'eff40':
            stdPhotons('pi0eff40', path=path)
            reconstructDecay(
                'pi0:reco -> gamma:pi0eff40 gamma:pi0eff40',
                '0.08377 < M < 0.189577',
                True,
                path=path)
        elif gammatype == 'eff30':
            stdPhotons('pi0eff30', path=path)
            reconstructDecay(
                'pi0:reco -> gamma:pi0eff30 gamma:pi0eff30',
                '0.08377 < M < 0.189577',
                True,
                path=path)
        elif gammatype == 'eff20':
            stdPhotons('pi0eff20', path=path)
            reconstructDecay(
                'pi0:reco -> gamma:pi0eff20 gamma:pi0eff20',
                '0.08377 < M < 0.189577',
                True,
                path=path)
        else:
            return

        reconstructDecay(
            'Xi0:prelim -> Lambda0:reco pi0:reco',
            '1.225 < M < 1.405',
            path=path,
            ignoreIfTooManyCandidates=False)
        vertexTree('Xi0:prelim', conf_level=0.0, massConstraint=[3122], ipConstraint=True, updateAllDaughters=True, path=path)
        # Reconstructed core resolution pi0~7.8 MeV selecting 4*sigma_core about the nominal mass
        applyCuts('Xi0:prelim', '[ daughter(1,M) > 0.111577 and daughter(1,M) < 0.158377 ]', path=path)
        vertexTree('Xi0:prelim', conf_level=0.0, massConstraint=[111, 3122], ipConstraint=True, updateAllDaughters=False, path=path)

        cutAndCopyList(
            'Xi0:std',
            'Xi0:prelim',
            '[ daughter(0,daughter(0,p)) > 0.3 and daughter(0,daughter(1,p)) > 0.1 and daughter(1,p) > 0.1 ] and \
         [ daughter(0,cosAngleBetweenMomentumAndVertexVector) > 0. and cosAngleBetweenMomentumAndVertexVector > 0.] and \
         [ daughter(0,cosAngleBetweenMomentumAndVertexVector) < cosAngleBetweenMomentumAndVertexVector ] and \
         [ formula( [ x^2 + y^2 + z^2 ]^[0.5] ) > 0.0 and \
         formula( [ x^2 + y^2 + z^2 ]^[0.5] ) < formula( [ daughter(0,x)^2 + daughter(0,y)^2 + daughter(0,z)^2 ]^[0.5] ) ] and \
         [ chiProb > 0.0 ] and \
         [ daughter(0,daughter(0,protonID)) > 0.01 ]',
            True,
            path=path)

        matchMCTruth('Xi0:std', path=path)

    elif BELLE:
        # Rough pi0/Lambda0 cuts from J. Yelton Observations of an Excited Omega- Baryon
        cutAndCopyList(
            'pi0:reco',
            'pi0:mdst',
            '[ M > 0.116077 and M < 0.153877 ] and \
   [ [ daughter(0,clusterReg) == 1 and daughter(0,E) > 0.05 ] or [ daughter(0,clusterReg) == 3 and daughter(0,E) > 0.05 ]  or \
   [ daughter(0,clusterReg) == 2 and  daughter(0,E) > 0.03 ] ] and \
   [ [ daughter(1,clusterReg) == 1 and daughter(1,E) > 0.05 ] or [ daughter(1,clusterReg) == 3 and daughter(1,E) > 0.05 ]  or \
   [ daughter(1,clusterReg) == 2 and  daughter(1,E) > 0.03 ] ]',
            path=path)
        cutAndCopyList(
            'Lambda0:reco',
            'Lambda0:mdst',
            '[ M > 1.112183 and M < 1.119183 ] and \
   [ cosAngleBetweenMomentumAndVertexVector > 0. ] and \
   [ formula( [ x^2 + y^2 ]^[0.5] ) > 0.35 ] and \
   [ daughter(0,atcPIDBelle(4,3)) > 0.2 ] and \
   [ daughter(0,atcPIDBelle(4,2)) > 0.2 ]',
            True, path=path)
        # Selected Xi0 from J. Yelton Observations of an Excited Omega- Baryon
        reconstructDecay(
            'Xi0:prelim -> Lambda0:reco pi0:reco',
            '1.28486 < M < 1.34486',
            path=path,
            ignoreIfTooManyCandidates=False)
        vertexTree('Xi0:prelim', conf_level=0.0, massConstraint=[3122], ipConstraint=True, updateAllDaughters=True, path=path)
        applyCuts('Xi0:prelim',
                  '[ daughter(1,M) > 0.124577 and daughter(1,M) < 0.145377 ]',
                  path=path)
        vertexTree('Xi0:prelim', conf_level=0.0, massConstraint=[111, 3122], ipConstraint=True, updateAllDaughters=False, path=path)

        cutAndCopyList(
            'Xi0:std',
            'Xi0:prelim',
            '[ daughter(0,cosAngleBetweenMomentumAndVertexVector) > 0. and cosAngleBetweenMomentumAndVertexVector > 0.] and \
   [ formula( [ x^2 + y^2 +z^2 ]^[0.5] ) > 2.0 and \
   formula( [ x^2 + y^2 + z^2 ]^[0.5] ) < formula( [ daughter(0,x)^2 + daughter(0,y)^2 + daughter(0,z)^2 ]^[0.5] ) and \
   formula( [ daughter(0,x)^2 + daughter(0,y)^2 + daughter(0,z)^2 ]^[0.5] ) > 3.0 ] and \
   [ daughter(1,p) > 0.2 ] and \
   [ chiProb > 0.0 ]',
            True, path=path)

        matchMCTruth('Xi0:std', path=path)


def loadStdOmega(fitter='kfitter', path=analysis_main):
    mergedLambdas(path=path)
    # 3.5 MeV Range about the nominal mass
    cutAndCopyList(
        'Lambda0:reco',
        'Lambda0:merged',
        '[ M > 1.112183 and M < 1.119183 ]',
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

    cutAndCopyList(
        'Omega-:std',
        'Omega-:reco',
        '[ daughter(0,daughter(0,p)) > 0.3 and daughter(0,daughter(1,p)) > 0.1 and daughter(1,p) > 0.0 ] and \
       [ daughter(0,cosAngleBetweenMomentumAndVertexVector) > 0. and cosAngleBetweenMomentumAndVertexVector > 0.] and \
       [ formula( [ x^2 + y^2 + z^2 ]^[0.5] ) > 0. and \
       formula( [ x^2 + y^2 + z^2 ]^[0.5] ) < formula( [ daughter(0,x)^2 + daughter(0,y)^2 + daughter(0,z)^2 ]^[0.5] ) ] and \
       [ chiProb > 0.0 ] and \
       [ daughter(0,daughter(0,protonID)) > 0.01 and daughter(1,kaonID) > 0.01 ]',
        True,
        path=path)

    matchMCTruth('Omega-:std', path=path)


def goodXi(xitype='loose', path=analysis_main):
    if xitype == 'veryloose':
        cutAndCopyList(
            'Xi-:veryloose',
            'Xi-:std',
            '[ daughter(0,daughter(0,p)) > 0.3 and daughter(0,daughter(1,p)) > 0.1 and daughter(1,p) > 0.1 ] and \
       [ daughter(0,cosAngleBetweenMomentumAndVertexVector) > 0. and cosAngleBetweenMomentumAndVertexVector > 0.] and \
       [ formula( [ x^2 + y^2 + z^2 ]^[0.5] ) > 0.1 and \
       formula( [ x^2 + y^2 + z^2 ]^[0.5] ) < formula( [ daughter(0,x)^2 + daughter(0,y)^2 + daughter(0,z)^2 ]^[0.5] ) ] and \
       [ chiProb > 0.0 and daughter(0,chiProb) > 0.0 ] and \
       [ daughter(0,daughter(0,protonID)) > 0.01 ]',
            True,
            path=path)

    elif xitype == 'loose':
        cutAndCopyList(
            'Xi-:loose',
            'Xi-:std',
            '[ daughter(0,daughter(0,p)) > 0.3 and daughter(0,daughter(1,p)) > 0.1 and daughter(1,p) > 0.1 ] and \
       [ daughter(0,cosAngleBetweenMomentumAndVertexVector) > 0. and cosAngleBetweenMomentumAndVertexVector > 0.] and \
       [ formula( [ x^2 + y^2 + z^2 ]^[0.5] ) > 0.35 and \
       formula( [ x^2 + y^2 + z^2 ]^[0.5] ) < formula( [ daughter(0,x)^2 + daughter(0,y)^2 + daughter(0,z)^2 ]^[0.5] ) ] and \
       [ chiProb > 0.0 and daughter(0,chiProb) > 0.0 ] and \
       [ daughter(0,daughter(0,protonID)) > 0.01 ]',
            True,
            path=path)

    elif xitype == 'tight':
        cutAndCopyList(
            'Xi-:tight',
            'Xi-:std',
            '[ daughter(0,daughter(0,p)) > 0.3 and daughter(0,daughter(1,p)) > 0.1 and daughter(1,p) > 0.1 ] and \
       [ daughter(0,cosAngleBetweenMomentumAndVertexVector) > 0. and cosAngleBetweenMomentumAndVertexVector > 0.] and \
       [ formula([ daughter(0,cosAngleBetweenMomentumAndVertexVector) / cosAngleBetweenMomentumAndVertexVector ]) < 1.00085 ] and \
       [ formula( [ x^2 + y^2 + z^2 ]^[0.5] ) > 0.35 and \
        formula( [ x^2 + y^2 + z^2 ]^[0.5] ) < formula( [ daughter(0,x)^2 + daughter(0,y)^2 + daughter(0,z)^2 ]^[0.5] ) ] and \
       [ chiProb > 0.0 and daughter(0,chiProb) > 0.0 ] and \
       [ daughter(0,daughter(0,protonID)) > 0.01 ]',
            True,
            path=path)


def goodXi0(xitype='loose', path=analysis_main):
    if xitype == 'veryloose':
        # Reconstructed core resolution pi0~7.8 MeV, cut at 3*sigma_core about the nominal mass
        cutAndCopyList(
            'Xi0:veryloose',
            'Xi0:std',
            '[ daughter(0,daughter(0,p)) > 0.3 and daughter(0,daughter(1,p)) > 0.1 and daughter(1,p) > 0.2 ] and \
         [ daughter(0,cosAngleBetweenMomentumAndVertexVector) > 0. and cosAngleBetweenMomentumAndVertexVector > 0.] and \
         [ daughter(0,cosAngleBetweenMomentumAndVertexVector) < cosAngleBetweenMomentumAndVertexVector ] and \
         [ formula( [ x^2 + y^2 + z^2 ]^[0.5] ) > 0.25 and \
         formula( [ x^2 + y^2 + z^2 ]^[0.5] ) < formula( [ daughter(0,x)^2 + daughter(0,y)^2 + daughter(0,z)^2 ]^[0.5] ) ] and \
         [ chiProb > 0.0 and daughter(0,chiProb) > 0.0 ] and \
         [ daughter(0,daughter(0,protonID)) > 0.01 ] and \
         [ daughter(1,M) > 0.111577 and daughter(1,M) < 0.158377 ]',
            True,
            path=path)

    elif xitype == 'loose':
        # Reconstructed core resolution pi0~7.8 MeV, cut at 3*sigma_core about the nominal mass
        cutAndCopyList(
            'Xi0:loose',
            'Xi0:std',
            '[ daughter(0,daughter(0,p)) > 0.3 and daughter(0,daughter(1,p)) > 0.1 and daughter(1,p) > 0.2 ] and \
         [ daughter(0,cosAngleBetweenMomentumAndVertexVector) > 0. and cosAngleBetweenMomentumAndVertexVector > 0.] and \
         [ daughter(0,cosAngleBetweenMomentumAndVertexVector) < cosAngleBetweenMomentumAndVertexVector ] and \
         [ formula( [ x^2 + y^2 + z^2 ]^[0.5] ) > 1.5 and \
         formula( [ x^2 + y^2 + z^2 ]^[0.5] ) < formula( [ daughter(0,x)^2 + daughter(0,y)^2 + daughter(0,z)^2 ]^[0.5] ) ] and \
         [ chiProb > 0.0 and daughter(0,chiProb) > 0.0 ] and \
         [ daughter(0,daughter(0,protonID)) > 0.01 ] and \
         [ daughter(1,M) > 0.111577 and daughter(1,M) < 0.158377 ]',
            True,
            path=path)

    elif xitype == 'tight':
        # Reconstructed core resolution pi0~7.8 MeV, cut at 2*sigma_core about the nominal mass
        cutAndCopyList(
            'Xi0:tight',
            'Xi0:std',
            '[ daughter(0,daughter(0,p)) > 0.3 and daughter(0,daughter(1,p)) > 0.1 and daughter(1,p) > 0.2 ] and \
         [ daughter(0,cosAngleBetweenMomentumAndVertexVector) > 0. and cosAngleBetweenMomentumAndVertexVector > 0.] and \
         [ daughter(0,cosAngleBetweenMomentumAndVertexVector) < cosAngleBetweenMomentumAndVertexVector ] and \
         [ formula( [ x^2 + y^2 + z^2 ]^[0.5] ) > 1.5 and \
         formula( [ x^2 + y^2 + z^2 ]^[0.5] ) < formula( [ daughter(0,x)^2 + daughter(0,y)^2 + daughter(0,z)^2 ]^[0.5] ) ] and \
         [ chiProb > 0.0 and daughter(0,chiProb) > 0.0 ] and \
         [ daughter(0,daughter(0,protonID)) > 0.01 ] and \
         [ daughter(1,M) > 0.119377 and daughter(1,M) < 0.150577 ]',
            True,
            path=path)


def goodOmega(omegatype='loose', path=analysis_main):
    if omegatype == 'veryloose':
        cutAndCopyList(
            'Omega-:veryloose',
            'Omega-:std',
            '[ daughter(0,daughter(0,p)) > 0.3 and daughter(0,daughter(1,p)) > 0.1 and daughter(1,p) > 0.175 ] and \
       [ daughter(0,cosAngleBetweenMomentumAndVertexVector) > 0. and cosAngleBetweenMomentumAndVertexVector > 0.] and \
       [ formula( [ x^2 + y^2 + z^2 ]^[0.5] ) > 0.1 and \
       formula( [ x^2 + y^2 + z^2 ]^[0.5] ) < formula( [ daughter(0,x)^2 + daughter(0,y)^2 + daughter(0,z)^2 ]^[0.5] ) ] and \
       [ chiProb > 0.0 and daughter(0,chiProb) > 0.0 ] and \
       [ daughter(0,daughter(0,protonID)) > 0.01 and daughter(1,kaonID) > 0.01 ]',
            True,
            path=path)

    elif omegatype == 'loose':
        cutAndCopyList(
            'Omega-:loose',
            'Omega-:std',
            '[ daughter(0,daughter(0,p)) > 0.3 and daughter(0,daughter(1,p)) > 0.1 and daughter(1,p) > 0.275 ] and \
       [ daughter(0,cosAngleBetweenMomentumAndVertexVector) > 0. and cosAngleBetweenMomentumAndVertexVector > 0.] and \
       [ formula( [ x^2 + y^2 + z^2 ]^[0.5] ) > 0.5 and \
       formula( [ x^2 + y^2 + z^2 ]^[0.5] ) < formula( [ daughter(0,x)^2 + daughter(0,y)^2 + daughter(0,z)^2 ]^[0.5] ) ] and \
       [ chiProb > 0.0 and daughter(0,chiProb) > 0.0 ] and \
       [ daughter(0,daughter(0,protonID)) > 0.01 and daughter(1,kaonID) > 0.01 ]',
            True,
            path=path)

    elif omegatype == 'tight':
        cutAndCopyList(
            'Omega-:tight',
            'Omega-:std',
            '[ daughter(0,daughter(0,p)) > 0.3 and daughter(0,daughter(1,p)) > 0.1 and daughter(1,p) > 0.275 ] and \
       [ daughter(0,cosAngleBetweenMomentumAndVertexVector) > 0. and cosAngleBetweenMomentumAndVertexVector > 0.] and \
       [ formula( [ daughter(0,cosAngleBetweenMomentumAndVertexVector) / cosAngleBetweenMomentumAndVertexVector ] ) < 1.0001 ] and \
       [ formula( [ x^2 + y^2 + z^2 ]^[0.5] ) > 0.5 and \
        formula( [ x^2 + y^2 + z^2 ]^[0.5] ) < formula( [ daughter(0,x)^2 + daughter(0,y)^2 + daughter(0,z)^2 ]^[0.5] ) ] and \
       [ chiProb > 0.0 and daughter(0,chiProb) > 0.0 ] and \
       [ daughter(0,daughter(0,protonID)) > 0.01  and daughter(1,kaonID) > 0.01]',
            True,
            path=path)
