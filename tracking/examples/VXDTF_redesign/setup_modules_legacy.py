#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import types


def setup_rawSecMapMerger(path=0, rootFileNames='dummy.root', logLevel=LogLevel.INFO, debugVal=1):
    """This function adds the RawSecMapMerger-module to given path.

    @param path if set to 0 (standard) the created modules will not be added, but returned.
    If a path is given, 'None' is returned but will be added to given path instead.

    @param rootFileNames needs the full fileName containing the training sample.

    @param logLevel set to logLevel level of your choice.

    @param debugVal set to debugLevel of choice - will be ignored if logLevel is not set to LogLevel.DEBUG
    """
    print("setup RawSecMapMerger...")
    merger = register_module('RawSecMapMerger')
    merger.logging.log_level = logLevel
    merger.logging.debug_level = debugVal
    merger.param('rootFileNames', rootFileNames)
    if path is 0:
        return merger
    else:
        path.add_module(merger)
        return None


def setup_qualityEstimators(path=0, estimatorType="random", sptcName="", logLevel=LogLevel.INFO, debugVal=1):
    """This function adds the a qualityEstimator-Module to given path.

    @param path if set to 0 (standard) the created modules will not be added, but returned.
    If a path is given, 'None' is returned but will be added to given path instead.

    @param estimatorType needs the name of the quality estimator to be used
      - currently supported: "random", "circleFit", TODO "kalman", "daf" and optionally "lineFit" & "helixFit".

    @param sptcName sets the name of the sptcStoreArray on which the quality should be estimated.

    @param logLevel set to logLevel level of your choice.

    @param debugVal set to debugLevel of choice - will be ignored if logLevel is not set to LogLevel.DEBUG
    """
    print("setup qualityEstimators...")
    if estimatorType is "random":
        qEstimator = register_module('QualityEstimatorVXDRandom')
        qEstimator.param('tcArrayName', sptcName)
    elif estimatorType is "circleFit":
        qEstimator = register_module('QualityEstimatorVXDCircleFit')
        qEstimator.param('tcArrayName', sptcName)
    else:
        print("ERROR! unknown estimatorType " + estimatorType + " is given - can not proceed!")
        exit
    qEstimator.logging.log_level = logLevel
    qEstimator.logging.debug_level = debugVal
    if path is 0:
        return qEstimator
    else:
        path.add_module(qEstimator)
        return None


def setup_trackSetEvaluators(path=0, evaluatorType="hopfield", sptcName="", networkName="", logLevel=LogLevel.INFO, debugVal=1):
    """This function adds the a trackSetEvaluator-Module to given path.

    @param path if set to 0 (standard) the created modules will not be added, but returned.
    If a path is given, 'None' is returned but will be added to given path instead.

    @param evaluatorType needs the name of the trackset evaluator to be used - currently supported: "greedy", "hopfield".

    @param sptcName sets the name of the sptcStoreArray on which the subset shall be found.

    @param sptcName sets the name of the sptcNetwork on which the subset shall be found.

    @param logLevel set to logLevel level of your choice.

    @param debugVal set to debugLevel of choice - will be ignored if logLevel is not set to LogLevel.DEBUG
    """
    print("setup trackSetEvaluators...")
    if evaluatorType is "hopfield":
        tsEvaluator = register_module('TrackSetEvaluatorHopfieldNN')
    elif evaluatorType is "greedy":
        tsEvaluator = register_module('TrackSetEvaluatorGreedy')
    else:
        print("ERROR! unknown evaluatorType " + evaluatorType + " is given - can not proceed!")
        exit
    tsEvaluator.param('tcArrayName', sptcName)
    tsEvaluator.param('tcNetworkName', networkName)
    tsEvaluator.logging.log_level = logLevel
    tsEvaluator.logging.debug_level = debugVal
    if path is 0:
        return tsEvaluator
    else:
        path.add_module(tsEvaluator)
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
    print("setup TrackFinderMCTruth...")
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


def setup_gfTtoSPTCConverters(
        path=0,
        pxdSPs='pxdOnly',
        svdSPs='nosingleSP',
        gfTCinput='mcTracks',
        sptcOutput='checkedSPTCs',
        usePXD=True,
        logLevel=LogLevel.INFO,
        debugVal=1):
    """This function adds the modules needed to convert GenFit-TCs to SpacePointTCs to given path.

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
    print("setup gfTCtoSPTCConverters...")
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
    trackCandConverter.param('skipCluster', True)

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
    """This function adds the PXDSpacePointCreator-module to given path.

    @param path if set to 0 (standard) the created modules will not be added, but returned.
    If a path is given, 'None' is returned but will be added to given path instead.

    @param nameOutput determines the name of the storeArray containing the created spacePoints.

    @param logLevel set to logLevel level of your choice.

    @param debugVal set to debugLevel of choice - will be ignored if logLevel is not set to LogLevel.DEBUG
    """
    print("setup PXDSpacePointCreator...")
    spCreatorPXD = register_module('PXDSpacePointCreator')
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
    """This function adds the SVDSpacePointCreator-module to given path.

    @param path if set to 0 (standard) the created modules will not be added, but returned.
    If a path is given, 'None' is returned but will be added to given path instead.

    @param nameOutput determines the name of the storeArray containing the created spacePoints.

    @param createSingleClusterSPs If False only SpacePoints where U and V clusters could be found are created.

    @param logLevel set to logLevel level of your choice.

    @param debugVal set to debugLevel of choice - will be ignored if logLevel is not set to LogLevel.DEBUG
    """
    print("setup SVDSpacePointCreator...")
    spCreatorSVD = register_module('SVDSpacePointCreator')
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


def setup_sp2thConnector(path=0,
                         pxdSPs='pxdOnly',
                         svdSPs='nosingleSP',
                         suffix='_relTH',
                         usePXD=True,
                         logLevel=LogLevel.INFO,
                         dbgLvl=1):
    """This function adds the SpacePoint2TrueHitConnector module with the setting that are needed vor the
    PurityCalculatorTools to work properly. NOTE: it will crate a new StoreArray of SpacePoints, where all
    SpacePoints are related to all possible TrueHits.

    @param path the path to which the module will be added. Set to 0 (default) if the module should be returned instead.
    @param pxdSPs name(s) of the StoreArray(s) containing the PXD SpacePoints

    @param svdSPs name(s) of the StoreArrays(s) containing the SVD SpacePoints

    @param suffix suffix that will be added to the SpacePoint StoreArray names for the new StoreArrays
    (use these SapcePoints if you need relations to TrueHits!)

    @param usePXD switch of usage of the PXD

    @param logLevel the chosen LogLevel

    @param dbgLevel the chosen debugLevel
    """
    # get the number of PXD SpacePoint arrays
    nPXDs = 1
    if not isinstance(pxdSPs, str):
        nPXDs = len(pxdSPs)

    spacePointNames = []
    detectorTypes = []
    clusterNames = []
    trueHitNames = []
    if usePXD:
        spacePointNames.append(pxdSPs)
        detectorTypes = ['PXD'] * nPXDs
        clusterNames = [''] * nPXDs
        trueHitNames = [''] * nPXDs
    spacePointNames.append(svdSPs)
    detectorTypes.append('SVD')
    clusterNames.append('')
    trueHitNames.append('')

    sp2thConnector = register_module('SpacePoint2TrueHitConnector')
    sp2thConnector.logging.log_level = logLevel
    sp2thConnector.logging.debug_level = dbgLvl  # set to little output for debug
    sp2thConnector.param('DetectorTypes', detectorTypes)
    sp2thConnector.param('TrueHitNames', trueHitNames)
    sp2thConnector.param('ClusterNames', clusterNames)
    sp2thConnector.param('SpacePointNames', spacePointNames)
    # fixed params
    sp2thConnector.param('registerAll', True)
    sp2thConnector.param('outputSuffix', suffix)
    sp2thConnector.param('storeSeperate', True)
    sp2thConnector.param('maxGlobalPosDiff', 0.05)
    sp2thConnector.param('maxPosSigma', 5)
    sp2thConnector.param('minWeight', 0)
    sp2thConnector.param('requirePrimary', False)
    sp2thConnector.param('positionAnalysis', False)
    sp2thConnector.param('requireProximity', True)
    if path is 0:
        return sp2thConnector
    else:
        path.add_module(sp2thConnector)
        return None


def setup_pGun(
    path=0, pdgCODES=[13], numTracks=1, momParams=[
        0.1, 0.15], thetaParams=[
            17., 150.], phiParams=[
                0., 360.], logLevel=LogLevel.WARNING, debugVal=1, smearVertex=True):
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

    @param smearVertex if true, the primary vertex is smeared, if false no smearing is done.
    """
    # fixed, uniform, normal, polyline, uniformPt, normalPt, inversePt, polylinePt or discrete
    print("setup ParticleGun...")
    particlegun = register_module('ParticleGun')
    particlegun.param('pdgCodes', pdgCODES)
    particlegun.param('nTracks', numTracks)
    particlegun.param('momentumGeneration', 'uniformPt')
    particlegun.param('momentumParams', momParams)
    particlegun.param('thetaGeneration', 'uniform')
    particlegun.param('thetaParams', thetaParams)
    particlegun.param('phiGeneration', 'uniform')
    particlegun.param('phiParams', phiParams)
    # particlegun.param('momentumGeneration', 'fixed')
    # particlegun.param('momentumParams', momParams[0])
    # particlegun.param('thetaGeneration', 'fixed')
    # particlegun.param('thetaParams', thetaParams[0])
    # particlegun.param('phiGeneration', 'fixed')
    # particlegun.param('phiParams', phiParams[0])
    if smearVertex:
        particlegun.param('vertexGeneration', 'uniform')
        particlegun.param('xVertexParams', [-0.1, 0.1])
        particlegun.param('yVertexParams', [-0.1, 0.1])
        particlegun.param('zVertexParams', [-0.5, 0.5])
    else:
        particlegun.param('vertexGeneration', 'fixed')
        particlegun.param('xVertexParams', [0.])
        particlegun.param('yVertexParams', [0.])
        particlegun.param('zVertexParams', [0.])
    particlegun.logging.log_level = logLevel
    particlegun.logging.debug_level = debugVal
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
    print("setup SecMapTrainerWithSpacePoints...")
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
    print("setup realistic Clusterizer...")
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
    print("setup VXDSimpleClusterizer...")
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


def setup_bg(path=0, bgFilesFolder="", usePXD=False, logLevel=LogLevel.INFO, debugVal=1):
    """This function adds the background to given path.

    @param path if set to 0 (standard) g4sim will not be added, but returned.
    If a path is given, 'None' is returned but will be added to given path instead.

    @param useBkg if set to True (standard = False) additional background is added to the simulation.

    @param bkgFiles if set to 0 (standard) g4sim will not be added, but returned.
    If a path is given, 'None' is returned but will be added to given path instead.

    @param logLevel set to logLevel level of your choice.

    @param debugVal set to debugLevel of choice - will be ignored if logLevel is not set to LogLevel.DEBUG
    """
    print("setup BeamBkgMixer...")
    import glob

    bg = glob.glob(bgFilesFolder + '*.root')
    print("setup_BeamBkgMixer - files to be imported:")
    print(bg)
    bkgmixer = register_module('BeamBkgMixer')
    bkgmixer.logging.log_level = logLevel
    bkgmixer.logging.debug_level = debugVal
    components = ['SVD']
    if usePXD:
        components.append('PXD')
    bkgmixer.param('backgroundFiles', bg)  # specify BG files
    bkgmixer.param('components', components)  # mix BG only for those components
    # bkgmixer.param('minTime', -5000)  # set time window start time [ns]
    # bkgmixer.param('maxTime', 15000)  # set time window stop time [ns]
    # bkgmixer.param('scaleFactors', [('Coulomb_LER', 1.05), ('Coulomb_HER',
    # 1.08), ('RBB_LER', 0.8)])  # scale rates of some backgrounds
    if (path is 0):
        return bkgmixer
    else:
        path.add_module(bkgmixer)
        return None


def setup_sim(path=0, useEDeposit=True, useMultipleScattering=True, allowDecay=True, verbose=False):
    """This function adds the g4simulation to given path.

    @param path if set to 0 (standard) g4sim will not be added, but returned.
    If a path is given, 'None' is returned but will be added to given path instead.

    @param useEDeposit if False, particles will not have eDeposit. WARMING: if you still want to get hits for that case:
    useEDeposit: If you want to work w/o E-deposit, edit pxd/data/PXD.xml and svd/data/SVD.xml,
    where you have to activate see neutrons = true.

    @param useMultipleScattering if False, multiple scattering in matter is deactivated.

    @param allowDecay if False, particles can not decay.

    @param verbose if True, some extra debug output is given.
    """
    print("setup FullSim...")
    g4sim = register_module('FullSim')
    g4sim.param('StoreAllSecondaries', True)
    uiCommandList = []
    if verbose:
        uiCommandList.append('/process/list')  # prints list of processes available
    if allowDecay:
        uiCommandList.append('/process/inactivate              Decay all')
    if not useEDeposit:
        uiCommandList.append('/process/inactivate        StepLimiter all')
        uiCommandList.append('/process/inactivate             muIoni all')
        uiCommandList.append('/process/inactivate              hIoni all')
        uiCommandList.append('/process/inactivate            ionIoni all')
        uiCommandList.append('/process/inactivate              eIoni all')
        uiCommandList.append('/process/inactivate              eBrem all')
        uiCommandList.append('/process/inactivate             hBrems all')
        uiCommandList.append('/process/inactivate            muBrems all')
        uiCommandList.append('/process/inactivate          hPairProd all')
        uiCommandList.append('/process/inactivate         muPairProd all')
        uiCommandList.append('/process/inactivate            annihil all')
        uiCommandList.append('/process/inactivate               phot all')
        uiCommandList.append('/process/inactivate              compt all')
        uiCommandList.append('/process/inactivate               conv all')
        # uiCommandList.append('/process/inactivate    PhotonInelastic')
        # uiCommandList.append('/process/inactivate    photonInelastic')
        uiCommandList.append('/process/inactivate    electronNuclear all')
        uiCommandList.append('/process/inactivate    positronNuclear all')
        uiCommandList.append('/process/inactivate        muonNuclear all')
        uiCommandList.append('/process/inactivate      photonNuclear all')
        uiCommandList.append('/process/inactivate              Decay all')
        uiCommandList.append('/process/inactivate         hadElastic all')
        uiCommandList.append('/process/inactivate   neutronInelastic all')
        uiCommandList.append('/process/inactivate           nCapture all')
        uiCommandList.append('/process/inactivate           nFission all')
        uiCommandList.append('/process/inactivate    protonInelastic all')
        uiCommandList.append('/process/inactivate    ionInelastic all')
        uiCommandList.append('/process/inactivate  pi+Inelastic all')
        uiCommandList.append('/process/inactivate pi-Inelastic all')
        uiCommandList.append('/process/inactivate  kaon+Inelastic all')
        uiCommandList.append('/process/inactivate kaon-Inelastic all')
        uiCommandList.append('/process/inactivate kaon0LInelastic all')
        uiCommandList.append('/process/inactivate kaon0SInelastic all')
        uiCommandList.append('/process/inactivate anti_protonInelastic all')
        uiCommandList.append('/process/inactivate anti_neutronInelastic all')
        uiCommandList.append('/process/inactivate    lambdaInelastic all')
        uiCommandList.append('/process/inactivate anti-lambdaInelastic all')
        uiCommandList.append('/process/inactivate sigma-Inelastic all')
        uiCommandList.append('/process/inactivate anti_sigma-Inelastic all')
        uiCommandList.append('/process/inactivate sigma+Inelastic all')
        uiCommandList.append('/process/inactivate anti_sigma+Inelastic all')
        uiCommandList.append('/process/inactivate   xi-Inelastic all')
        uiCommandList.append('/process/inactivate anti_xi-Inelastic all')
        uiCommandList.append('/process/inactivate    xi0Inelastic all')
        uiCommandList.append('/process/inactivate anti_xi0Inelastic all')
        uiCommandList.append('/process/inactivate omega-Inelastic all')
        uiCommandList.append('/process/inactivate anti_omega-Inelastic all')
        # uiCommandList.append('/process/inactivate CHIPSNuclearCaptureAtRest')
        uiCommandList.append('/process/inactivate hFritiofCaptureAtRest all')
        uiCommandList.append('/process/inactivate hBertiniCaptureAtRest all')
        uiCommandList.append('/process/inactivate muMinusCaptureAtRest all')
        uiCommandList.append('/process/inactivate  dInelastic all')
        uiCommandList.append('/process/inactivate  anti_deuteronInelastic all')
        uiCommandList.append('/process/inactivate    tInelastic all')
        uiCommandList.append('/process/inactivate    anti_tritonInelastic all')
        uiCommandList.append('/process/inactivate    He3Inelastic all')
        uiCommandList.append('/process/inactivate    anti_He3Inelastic all')
        uiCommandList.append('/process/inactivate    alphaInelastic all')
        uiCommandList.append('/process/inactivate    anti_alphaInelastic all')
        uiCommandList.append('/process/inactivate      ExtEnergyLoss all')
        uiCommandList.append('/process/inactivate       OpAbsorption all')
        uiCommandList.append('/process/inactivate         OpRayleigh all')
        uiCommandList.append('/process/inactivate            OpMieHG all')
        uiCommandList.append('/process/inactivate         OpBoundary all')
        uiCommandList.append('/process/inactivate              OpWLS all')
        uiCommandList.append('/process/inactivate           Cerenkov all')
        uiCommandList.append('/process/inactivate      Scintillation all')

    if not useMultipleScattering:
        uiCommandList.append('/process/inactivate                msc all')
        uiCommandList.append('/process/inactivate        CoulombScat all')
        # uiCommandList.append('/process/inactivate              muMsc')
    # uiCommandList.append('/process/list')
    # if (useMultipleScattering == False or useMultipleScattering == False):
    if verbose:
        uiCommandList.append('/particle/select pi+')
        uiCommandList.append('/particle/process/dump')
        uiCommandList.append('/particle/select pi-')
        uiCommandList.append('/particle/process/dump')
        uiCommandList.append('/particle/select e+')
        uiCommandList.append('/particle/process/dump')
        uiCommandList.append('/particle/select e-')
        uiCommandList.append('/particle/process/dump')
        uiCommandList.append('/particle/select mu+')
        uiCommandList.append('/particle/process/dump')
        uiCommandList.append('/particle/select mu-')
        uiCommandList.append('/particle/select kaon+')
        uiCommandList.append('/particle/process/dump')
        uiCommandList.append('/particle/select kaon-')
        uiCommandList.append('/particle/process/dump')
    g4sim.param('UICommands', uiCommandList)

    # print(uiCommandList)
    if (path is 0):
        return g4sim
    else:
        path.add_module(g4sim)
        return None
