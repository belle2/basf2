#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *


def setup_rawSecMapMerger(path=0, rootFileNames='dummy.root', logLevel=LogLevel.INFO, debugVal=1):
    """This function adds the RawSecMapMerger-module to given path.

    @param path if set to 0 (standard) the created modules will not be added, but returned.
    If a path is given, 'None' is returned but will be added to given path instead.

    @param rootFileNames needs the full fileName containing the training sample.

    @param logLevel set to logLevel level of your choice.

    @param debugVal set to debugLevel of choice - will be ignored if logLevel is not set to LogLevel.DEBUG
    """
    merger = register_module('RawSecMapMerger')
    merger.logging.log_level = logLevel
    merger.logging.debug_level = debugVal
    merger.param('rootFileNames', rootFileNames)
    if path is 0:
        return merger
    else:
        path.add_module(merger)
        return None


def setup_mcTF(
        path=0,
        whichParticles=['primary'],
        nameOutput='mcTracks',
        usePXD=False,
        minNdf=6,
        logLevel=LogLevel.INFO,
        debugVal=1):
    """This function adds the TrackFinderMCTruth-module to given path.

    @param path if set to 0 (standard) the created modules will not be added, but returned.
    If a path is given, 'None' is returned but will be added to given path instead.

    @param whichParticles expects a list of strings. sets the criteria for allowed track reco.

    @param nameOutput determines the name of the storeArray containing the created mcTCs.

    @param usePXD If False pxdClusters are ignored.

    @param minNdf sets the minimal TC-length depending on the ndf (n degrees of freedom),
    + 2ndf per pxdCluster, +1 ndf per svdCluster.

    @param logLevel set to logLevel level of your choice.

    @param debugVal set to debugLevel of choice - will be ignored if logLevel is not set to LogLevel.DEBUG
    """
    doPXD = 0
    if usePXD:
        doPXD = 1
    mctrackfinder = register_module('TrackFinderMCTruth')
    mctrackfinder.logging.log_level = LogLevel.INFO
    param_mctrackfinder = {
        'UseCDCHits': 0,
        'UseSVDHits': 1,
        'UsePXDHits': doPXD,
        'Smearing': 0,
        'UseClusters': True,
        'MinimalNDF': minNdf,
        'WhichParticles': whichParticles,
        'GFTrackCandidatesColName': nameOutput,
    }
    mctrackfinder.param(param_mctrackfinder)
    if path is 0:
        return mctrackfinder
    else:
        path.add_module(mctrackfinder)
        return None


def setup_gfTCtoSPTCConverters(
        path=0,
        pxdSPs='pxdOnly',
        svdSPs='nosingleSP',
        gfTCinput='mcTracks',
        sptcOutput='checkedSPTCs',
        usePXD=True,
        logLevel=LogLevel.INFO,
        debugVal=1):
    """This function adds the modules needed to convert genfit-TCs to SpacePointTCs to given path.

    @param path if set to 0 (standard) the created modules will not be added, but returned.
    If a path is given, 'None' is returned but will be added to given path instead.

    @param pxdSPs the name of the storeArray containing PXDSPs - gets ignored if usePXD is False.

    @param svdSPs the name of the storeArray containing SVDSPs.

    @param gfTCinput defines the name of input-genfit-TCs.

    @param sptcOutput defines the name of output-SPTCs.

    @param usePXD If False pxdClusters are ignored.

    @param logLevel set to logLevel level of your choice.

    @param debugVal set to debugLevel of choice - will be ignored if logLevel is not set to LogLevel.DEBUG
    """
    doPXD = 0
    spacePointNames = []
    detectorTypes = []
    trueHitNames = []
    clusterNames = []
    if usePXD:
        spacePointNames.append(pxdSPs)
        detectorTypes.append('PXD')
        trueHitNames.append('')
        clusterNames.append('')
    spacePointNames.append(svdSPs)
    detectorTypes.append('SVD')
    trueHitNames.append('')
    clusterNames.append('')

    # module to create relations between SpacePoints and TrueHits -> some of
    # the following modules will be utilizing these relations!
    sp2thConnector = register_module('SpacePoint2TrueHitConnector')
    sp2thConnector.logging.log_level = logLevel
    sp2thConnector.param('DetectorTypes', detectorTypes)
    sp2thConnector.param('TrueHitNames', trueHitNames)
    sp2thConnector.param('ClusterNames', clusterNames)
    sp2thConnector.param('SpacePointNames', spacePointNames)
    sp2thConnector.param('outputSuffix', '_relTH')
    sp2thConnector.param('storeSeperate', False)
    sp2thConnector.param('registerAll', False)
    sp2thConnector.param('maxGlobalPosDiff', 0.05)
    sp2thConnector.param('maxPosSigma', 5)
    sp2thConnector.param('minWeight', 0)
    sp2thConnector.param('requirePrimary', True)
    sp2thConnector.param('positionAnalysis', False)

    # TCConverter, genfit -> SPTC
    trackCandConverter = register_module('GFTC2SPTCConverter')
    trackCandConverter.logging.log_level = logLevel
    trackCandConverter.param('genfitTCName', gfTCinput)
    trackCandConverter.param('SpacePointTCName', 'SPTracks')
    trackCandConverter.param('NoSingleClusterSVDSP', svdSPs)
    trackCandConverter.param('PXDClusterSP', pxdSPs)
    trackCandConverter.param('checkTrueHits', True)
    trackCandConverter.param('useSingleClusterSP', False)
    trackCandConverter.param('checkNoSingleSVDSP', True)

    # SpacePointTrackCand referee
    sptcReferee = register_module('SPTCReferee')
    sptcReferee.logging.log_level = LogLevel.INFO
    sptcReferee.param('sptcName', 'SPTracks')
    sptcReferee.param('newArrayName', sptcOutput)
    sptcReferee.param('storeNewArray', True)
    sptcReferee.param('checkCurling', True)
    sptcReferee.param('splitCurlers', True)
    sptcReferee.param('keepOnlyFirstPart', True)
    sptcReferee.param('kickSpacePoint', True)
    sptcReferee.param('checkSameSensor', True)
    sptcReferee.param('useMCInfo', True)

    if path is 0:
        return [sp2thConnector, trackCandConverter, sptcReferee]
    else:
        path.add_module(sp2thConnector)
        path.add_module(trackCandConverter)
        path.add_module(sptcReferee)
        return None


def setup_spCreatorPXD(path=0, nameOutput='pxdOnly', logLevel=LogLevel.INFO, debugVal=1):
    """This function adds the SpacePointCreatorPXD-module to given path.

    @param path if set to 0 (standard) the created modules will not be added, but returned.
    If a path is given, 'None' is returned but will be added to given path instead.

    @param nameOutput determines the name of the storeArray containing the created spacePoints.

    @param logLevel set to logLevel level of your choice.

    @param debugVal set to debugLevel of choice - will be ignored if logLevel is not set to LogLevel.DEBUG
    """
    spCreatorPXD = register_module('SpacePointCreatorPXD')
    spCreatorPXD.logging.log_level = logLevel
    spCreatorPXD.logging.debug_level = debugVal
    spCreatorPXD.param('NameOfInstance', nameOutput)
    spCreatorPXD.param('SpacePoints', nameOutput)
    if path is 0:
        return spCreatorPXD
    else:
        path.add_module(spCreatorPXD)
        return None


def setup_spCreatorSVD(path=0, nameOutput='nosingleSP', createSingleClusterSPs=False, logLevel=LogLevel.INFO, debugVal=1):
    """This function adds the SpacePointCreatorSVD-module to given path.

    @param path if set to 0 (standard) the created modules will not be added, but returned.
    If a path is given, 'None' is returned but will be added to given path instead.

    @param nameOutput determines the name of the storeArray containing the created spacePoints.

    @param createSingleClusterSPs If False only SpacePoints where U and V clusters could be found are created.

    @param logLevel set to logLevel level of your choice.

    @param debugVal set to debugLevel of choice - will be ignored if logLevel is not set to LogLevel.DEBUG
    """
    spCreatorSVD = register_module('SpacePointCreatorSVD')
    spCreatorSVD.logging.log_level = logLevel
    spCreatorSVD.logging.debug_level = debugVal
    spCreatorSVD.param('OnlySingleClusterSpacePoints', createSingleClusterSPs)
    spCreatorSVD.param('NameOfInstance', nameOutput)
    spCreatorSVD.param('SpacePoints', nameOutput)
    if path is 0:
        return spCreatorSVD
    else:
        path.add_module(spCreatorSVD)
        return None


def setup_pGun(
    path=0, pdgCODES=[13], numTracks=1, momParams=[
        0.1, 0.15], thetaParams=[
            17., 150.], phiParams=[
                0., 360.], logLevel=LogLevel.WARNING, debugVal=1):
    """This function adds a particleGun to given path.

    @param path if set to 0 (standard) the created modules will not be added, but returned.
    If a path is given, 'None' is returned but will be added to given path instead.

    @param pdgCODES should contain a list of pdgCodes allowed to be used for the particle guns.
    Particles and antiparticles of the same type have to be listed separately.

    @param numTracks sets the number of tracks per event (WARNING: 0 is not working, in this case you should not call the function).

    @param momParams should be a list containing 2 entries. First is minValue, second is maxValue in GeV.

    @param thetaParams should be a list containing 2 entries. First is minValue, second is maxValue in Grad.

    @param phiParams should be a list containing 2 entries. First is minValue, second is maxValue in Grad.

    @param logLevel set to logLevel level of your choice.

    @param debugVal set to debugLevel of choice - will be ignored if logLevel is not set to LogLevel.DEBUG
    """
    # fixed, uniform, normal, polyline, uniformPt, normalPt, inversePt, polylinePt or discrete
    param_pGun = {
        'pdgCodes': pdgCODES,
        'nTracks': numTracks,
        'momentumGeneration': 'uniformPt',
        'momentumParams': momParams,
        'thetaGeneration': 'uniform',
        'thetaParams': thetaParams,
        'phiGeneration': 'uniform',
        'phiParams': phiParams,
        'vertexGeneration': 'uniform',
        'xVertexParams': [-0.1, 0.1],
        'yVertexParams': [-0.1, 0.1],
        'zVertexParams': [-0.5, 0.5],
    }

    particlegun = register_module('ParticleGun')
    particlegun.logging.log_level = logLevel
    particlegun.logging.debug_level = debugVal
    particlegun.param(param_pGun)
    if path is 0:
        return particlegun
    else:
        path.add_module(particlegun)
        return None


def setup_secMapTrainerWithSpacePoints(
    path=0, pTcuts=[
        0.025, 0.15, 0.4], secConfigU=[
            0., 0.33, 0.67, 1.0], secConfigV=[
                0., 0.33, 0.67, 1.0], setupFileName='secMap2016', logLevel=LogLevel.WARNING, debugVal=1):
    """This function adds the realistic digitizers and clusterizers to given path.

    @param path if set to 0 (standard) the created modules will not be added, but returned.
    If a path is given, 'None' is returned but will be added to given path instead.

    @param pTcuts sets the pT-cuts for the single sectorMaps to be created.

    @param secConfigU sets the sector-cuts for u-side of sensor in relative coordinates.

    @param secConfigV sets the sector-cuts for v-side of sensor in relative coordinates.

    @param setupFileName defines the fileName to be used, sets the name of the sectorMap as well.

    @param logLevel set to logLevel level of your choice.

    @param debugVal set to debugLevel of choice - will be ignored if logLevel is not set to LogLevel.DEBUG
    """

    param_filterCalc = {
        'detectorType': ['SVD'],
        'maxXYvertexDistance': 1.,
        'pTcuts': pTcuts,
        'pTCutSmearer': 5.,
        'highestAllowedLayer': 6,
        'sectorConfigU': secConfigU,
        'sectorConfigV': secConfigV,
        'setOrigin': [0., 0., 0.],
        'magneticFieldStrength': 1.5,
        'testBeam': 0,
        'secMapWriteToRoot': 1,
        'secMapWriteToAscii': 0,
        'rootFileName': [setupFileName, 'UPDATE'],
        'sectorSetupFileName': setupFileName,
        'spTCarrayName': 'checkedSPTCs',
        'smearHits': 0,
        'uniSigma': 0.3,
        'noCurler': 1,
        'useOldSecCalc': 0,
    }

    filterCalc = register_module('SecMapTrainerWithSpacePoints')
    filterCalc.logging.log_level = logLevel
    filterCalc.logging.debug_level = debugVal
    filterCalc.param(param_filterCalc)
    if path is 0:
        return filterCalc
    else:
        path.add_module(filterCalc)
        return None


def setup_realClusters(path=0, usePXD=False, logLevel=LogLevel.INFO, debugVal=1):
    """This function adds the realistic digitizers and clusterizers to given path.

    @param path if set to 0 (standard) the created modules will not be added, but returned.
    If a path is given, 'None' is returned but will be added to given path instead.

    @param usePXD if set to True, pxdClusters are added.

    @param logLevel set to logLevel level of your choice.

    @param debugVal set to debugLevel of choice - will be ignored if logLevel is not set to LogLevel.DEBUG
    """
    moduleList = []

    if usePXD is True:
        pxdDigitizer = register_module('PXDDigitizer')
        moduleList.append(pxdDigitizer)
        pxdClusterizer = register_module('PXDClusterizer')
        moduleList.append(pxdClusterizer)
    svdDigitizer = register_module('SVDDigitizer')
    moduleList.append(svdDigitizer)
    svdClusterizer = register_module('SVDClusterizer')
    moduleList.append(svdClusterizer)
    if path is 0:
        return moduleList
    else:
        for aModule in moduleList:
            path.add_module(aModule)
        return None


def setup_simpleClusters(path=0, onlyPrimaries=False, useEDeposit=True, logLevel=LogLevel.INFO, debugVal=1):
    """This function adds the simpleClusterizer to given path.

    @param path if set to 0 (standard) the created modules will not be added, but returned.
    If a path is given, 'None' is returned but will be added to given path instead.

    @param onlyPrimaries if set to True, only clusters for primary particles are created and bkg-hits are ignored.

    @param useEDeposit if False, particles will not have eDeposit. WARMING: if you still want to get hits for that case:
    useEDeposit: If you want to work w/o E-deposit, edit pxd/data/PXD.xml and svd/data/SVD.xml,
    where you have to activate see neutrons = true.

    @param logLevel set to logLevel level of your choice.

    @param debugVal set to debugLevel of choice - will be ignored if logLevel is not set to LogLevel.DEBUG
    """
    simpleClusterizer = register_module('VXDSimpleClusterizer')
    simpleClusterizer.logging.log_level = logLevel
    simpleClusterizer.logging.debug_level = debugVal
    simpleClusterizer.param('setMeasSigma', 0)
    simpleClusterizer.param('onlyPrimaries', onlyPrimaries)
    if useEDeposit is False:
        simpleClusterizer.param('energyThresholdU', -0.0001)
        simpleClusterizer.param('energyThresholdV', -0.0001)
        simpleClusterizer.param('energyThreshold', -0.0001)
    if path is 0:
        return simpleClusterizer
    else:
        path.add_module(simpleClusterizer)
        return None


def setup_sim(path=0, useEDeposit=True):
    """This function adds the g4simulation to given path.

    @param path if set to 0 (standard) g4sim will not be added, but returned.
    If a path is given, 'None' is returned but will be added to given path instead.

    @param useEDeposit if False, particles will not have eDeposit. WARMING: if you still want to get hits for that case:
    useEDeposit: If you want to work w/o E-deposit, edit pxd/data/PXD.xml and svd/data/SVD.xml,
    where you have to activate see neutrons = true.
    """
    g4sim = register_module('FullSim')
    g4sim.param('StoreAllSecondaries', True)
    if useEDeposit is False:
        g4sim.param('UICommands', [
            '/process/list',
            '/process/inactivate                msc',
            '/process/inactivate              hIoni',
            '/process/inactivate            ionIoni',
            '/process/inactivate              eIoni',
            '/process/inactivate              eBrem',
            '/process/inactivate            annihil',
            '/process/inactivate               phot',
            '/process/inactivate              compt',
            '/process/inactivate               conv',
            '/process/inactivate             hBrems',
            '/process/inactivate          hPairProd',
            '/process/inactivate              muMsc',
            '/process/inactivate             muIoni',
            '/process/inactivate            muBrems',
            '/process/inactivate         muPairProd',
            '/process/inactivate        CoulombScat',
            '/process/inactivate    PhotonInelastic',
            '/process/inactivate     ElectroNuclear',
            '/process/inactivate    PositronNuclear',
            '/process/inactivate              Decay',
            '/process/inactivate         hadElastic',
            '/process/inactivate   NeutronInelastic',
            '/process/inactivate           nCapture',
            '/process/inactivate           nFission',
            '/process/inactivate    ProtonInelastic',
            '/process/inactivate  PionPlusInelastic',
            '/process/inactivate PionMinusInelastic',
            '/process/inactivate  KaonPlusInelastic',
            '/process/inactivate KaonMinusInelastic',
            '/process/inactivate KaonZeroLInelastic',
            '/process/inactivate KaonZeroSInelastic',
            '/process/inactivate AntiProtonInelastic',
            '/process/inactivate AntiNeutronInelastic',
            '/process/inactivate    LambdaInelastic',
            '/process/inactivate AntiLambdaInelastic',
            '/process/inactivate SigmaMinusInelastic',
            '/process/inactivate AntiSigmaMinusInelastic',
            '/process/inactivate SigmaPlusInelastic',
            '/process/inactivate AntiSigmaPlusInelastic',
            '/process/inactivate   XiMinusInelastic',
            '/process/inactivate AntiXiMinusInelastic',
            '/process/inactivate    XiZeroInelastic',
            '/process/inactivate AntiXiZeroInelastic',
            '/process/inactivate OmegaMinusInelastic',
            '/process/inactivate AntiOmegaMinusInelastic',
            '/process/inactivate CHIPSNuclearCaptureAtRest',
            '/process/inactivate muMinusCaptureAtRest',
            '/process/inactivate  DeuteronInelastic',
            '/process/inactivate    TritonInelastic',
            '/process/inactivate      ExtEnergyLoss',
            '/process/inactivate       OpAbsorption',
            '/process/inactivate         OpRayleigh',
            '/process/inactivate            OpMieHG',
            '/process/inactivate         OpBoundary',
            '/process/inactivate              OpWLS',
            '/process/inactivate           Cerenkov',
            '/process/inactivate      Scintillation',
        ])
    if (path is 0):
        return g4sim
    else:
        path.add_module(g4sim)
        return None
