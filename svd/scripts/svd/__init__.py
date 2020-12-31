#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
from ROOT import Belle2
import sys


def add_new_svd_reconstruction(path, isROIsimulation=False, createRecoDigits=True):

    if(isROIsimulation):
        clusterizerName = '__ROISVDClusterizer'
        recocreatorName = '__ROISVDRecoDigitCreator'
        dataFormatName = '__ROISVDDataFormat'
        recoDigitsName = '__ROIsvdRecoDigits'
        clustersName = '__ROIsvdClusters'
        shaperDigitsName = ""
        missingAPVsClusterCreatorName = '__ROISVDMissingAPVsClusterCreator'
    else:
        clusterizerName = 'SVDClusterizer'
        recocreatorName = 'SVDRecoDigitCreator'
        dataFormatName = 'SVDDataFormat'
        recoDigitsName = ""
        clustersName = ""
        shaperDigitsName = ""
        missingAPVsClusterCreatorName = 'SVDMissingAPVsClusterCreator'

    # data format check NOT appended
    if dataFormatName not in [e.name() for e in path.modules()]:
        dataFormat = register_module('SVDDataFormatCheck')
        dataFormat.param('ShaperDigits', shaperDigitsName)

    if clusterizerName not in [e.name() for e in path.modules()]:
        clusterizer = register_module('SVDClusterizer')
        clusterizer.set_name(clusterizerName)
        clusterizer.param('Clusters', clustersName)
        clusterizer.param('timeAlgorithm6Samples', "CoG6")
        clusterizer.param('timeAlgorithm3Samples', "CoG6")
        clusterizer.param('chargeAlgorithm6Samples', "MaxSample")
        clusterizer.param('chargeAlgorithm3Samples', "MaxSample")
        clusterizer.param('positionAlgorithm6Samples', "oldDefault")
        clusterizer.param('positionAlgorithm3Samples', "oldDefault")
        clusterizer.param('stripTimeAlgorithm6Samples', "dontdo")
        clusterizer.param('stripTimeAlgorithm3Samples', "dontdo")
        clusterizer.param('stripChargeAlgorithm6Samples', "MaxSample")
        clusterizer.param('stripChargeAlgorithm3Samples', "MaxSample")
        clusterizer.param('useDB', False)
        path.add_module(clusterizer)

    if missingAPVsClusterCreatorName not in [e.name() for e in path.modules()]:
        missingAPVCreator = register_module('SVDMissingAPVsClusterCreator')
        missingAPVCreator.set_name(missingAPVsClusterCreatorName)
        path.add_module(missingAPVCreator)

    # Add SVDSpacePointCreator
    add_svd_SPcreation(path, isROIsimulation)

    if createRecoDigits:
        # Add SVDRecoDigit creator module
        # useful for DQM and validation
        if recocreatorName not in [e.name() for e in path.modules()]:
            recoDigitCreator = register_module('SVDRecoDigitCreator')
            recoDigitCreator.set_name(recocreatorName)
            recoDigitCreator.param('RecoDigits', recoDigitsName)
            recoDigitCreator.param('Clusters', clustersName)
            recoDigitCreator.param('timeAlgorithm6Samples', "CoG6")
            recoDigitCreator.param('timeAlgorithm3Samples', "CoG6")
            recoDigitCreator.param('chargeAlgorithm6Samples', "MaxSample")
            recoDigitCreator.param('chargeAlgorithm3Samples', "MaxSample")
            recoDigitCreator.param('useDB', False)
            path.add_module(recoDigitCreator)


def add_svd_reconstruction(path, isROIsimulation=False, useNN=False, useCoG=True, applyMasking=False):

    if(useNN and useCoG):
        print("WARNING! you can't select both NN and CoG for SVD reconstruction. Using the default algorithm (TB-equivalent)")
        add_svd_reconstruction_tb(path, isROIsimulation)
    elif(useNN):
        add_svd_reconstruction_nn(path, isROIsimulation)

    elif(useCoG):
        add_svd_reconstruction_CoG(path, isROIsimulation, applyMasking)


def add_svd_reconstruction_nn(path, isROIsimulation=False, direct=False):

    if direct:
        if(isROIsimulation):
            clusterizerName = '__ROISVDClusterizerDirect'
            clusterName = '__ROIsvdClusters'
        else:
            clusterizerName = 'SVDClusterizerDirect'
            clusterName = ""

        if clusterizerName not in [e.name() for e in path.modules()]:
            clusterizer = b2.register_module('SVDClusterizerDirect')
            clusterizer.set_name(clusterizerName)
            clusterizer.param('Clusters', clusterName)
            path.add_module(clusterizer)
    else:
        if(isROIsimulation):
            fitterName = '__ROISVDNNShapeReconstructor'
            clusterizerName = '__ROISVDNNClusterizer'
            clusterName = '__ROIsvdClusters'
        else:
            fitterName = 'SVDNNShapeReconstructor'
            clusterizerName = 'SVDNNClusterizer'
            clusterName = ''

        if fitterName not in [e.name() for e in path.modules()]:
            fitter = b2.register_module('SVDNNShapeReconstructor')
            fitter.set_name(fitterName)
            path.add_module(fitter)

        if clusterizerName not in [e.name() for e in path.modules()]:
            clusterizer = b2.register_module('SVDNNClusterizer')
            clusterizer.set_name(clusterizerName)
            clusterizer.param('Clusters', clusterName)
            path.add_module(clusterizer)


def add_svd_reconstruction_CoG(path, isROIsimulation=False, applyMasking=False):

    if(isROIsimulation):
        fitterName = '__ROISVDCoGTimeEstimator'
        clusterizerName = '__ROISVDSimpleClusterizer'
        dataFormatName = '__ROISVDDataFormat'
        clusterName = '__ROIsvdClusters'
        recoDigitsName = '__ROIsvdRecoDigits'
        shaperDigitsName = ""
        missingAPVsClusterCreatorName = '__ROISVDMissingAPVsClusterCreator'
    else:
        fitterName = 'SVDCoGTimeEstimator'
        clusterizerName = 'SVDSimpleClusterizer'
        dataFormatName = 'SVDDataFormat'
        clusterName = ""
        recoDigitsName = ""
        shaperDigitsName = ""
        missingAPVsClusterCreatorName = 'SVDMissingAPVsClusterCreator'

# add strip masking if needed
    if(applyMasking):
        if(isROIsimulation):
            shaperDigitsName = '__ROISVDShaperDigitsUnmasked'
            maskingName = '__ROISVDStripMasking'
        else:
            shaperDigitsName = 'SVDShaperDigitsUnmasked'
            maskingName = 'SVDStripMasking'

        if maskingName not in [e.name() for e in path.modules()]:
            masking = b2.register_module('SVDStripMasking')
            masking.set_name(maskingName)
            masking.param('ShaperDigitsUnmasked', shaperDigitsName)
            path.add_module(masking)

    if dataFormatName not in [e.name() for e in path.modules()]:
        dataFormat = b2.register_module('SVDDataFormatCheck')
        dataFormat.param('ShaperDigits', shaperDigitsName)

    if fitterName not in [e.name() for e in path.modules()]:
        fitter = b2.register_module('SVDCoGTimeEstimator')
        fitter.set_name(fitterName)
        fitter.param('RecoDigits', recoDigitsName)
        path.add_module(fitter)

    if clusterizerName not in [e.name() for e in path.modules()]:
        clusterizer = b2.register_module('SVDSimpleClusterizer')
        clusterizer.set_name(clusterizerName)
        clusterizer.param('RecoDigits', recoDigitsName)
        clusterizer.param('Clusters', clusterName)
        clusterizer.param('useDB', True)
        path.add_module(clusterizer)

    if missingAPVsClusterCreatorName not in [e.name() for e in path.modules()]:
        missingAPVCreator = b2.register_module('SVDMissingAPVsClusterCreator')
        missingAPVCreator.set_name(missingAPVsClusterCreatorName)
        path.add_module(missingAPVCreator)

    # Add SVDSpacePointCreator
    add_svd_SPcreation(path, isROIsimulation)


def add_svd_reconstruction_nn(path, isROIsimulation=False, direct=False):

    if direct:
        if(isROIsimulation):
            clusterizerName = '__ROISVDClusterizerDirect'
            clusterName = '__ROIsvdClusters'
        else:
            clusterizerName = 'SVDClusterizerDirect'
            clusterName = ""

        if clusterizerName not in [e.name() for e in path.modules()]:
            clusterizer = b2.register_module('SVDClusterizerDirect')
            clusterizer.set_name(clusterizerName)
            clusterizer.param('Clusters', clusterName)
            path.add_module(clusterizer)
    else:
        if(isROIsimulation):
            fitterName = '__ROISVDNNShapeReconstructor'
            clusterizerName = '__ROISVDNNClusterizer'
            clusterName = '__ROIsvdClusters'
        else:
            fitterName = 'SVDNNShapeReconstructor'
            clusterizerName = 'SVDNNClusterizer'
            clusterName = ''

        if fitterName not in [e.name() for e in path.modules()]:
            fitter = b2.register_module('SVDNNShapeReconstructor')
            fitter.set_name(fitterName)
            path.add_module(fitter)

        if clusterizerName not in [e.name() for e in path.modules()]:
            clusterizer = b2.register_module('SVDNNClusterizer')
            clusterizer.set_name(clusterizerName)
            clusterizer.param('Clusters', clusterName)
            path.add_module(clusterizer)


def add_svd_simulation(path, daqMode=2, latencyShift=-1, relativeShift=-1):

    if daqMode != 2 and daqMode != 1:
        print("OOPS the acquisition mode that you want to simulate is not available.")
        print("Please choose among daqMode = 2 (6-sample) and daqMode = 1 (3-sample). Exiting now.")
        sys.exit()

    # 6-sample acquisition mode
    if daqMode == 2:
        svdevtinfoset = b2.register_module("SVDEventInfoSetter")
        svdevtinfoset.param("daqMode", daqMode)
        path.add_module(svdevtinfoset)

        digitizer = b2.register_module('SVDDigitizer')
        path.add_module(digitizer)

    # 3-sample acquisition mode
    # we previously simulated this mode with StartSampling = 58 (default StartSampling = -2)
    if daqMode == 1:
        if relativeShift == -1 and latencyShift == -1:
            print("OOPS please choose if you want to use the relativeShift or the latencyShift. Exiting now.")
            sys.exit(1)
        if relativeShift != -1 and latencyShift != -1:
            print("OOPS please choose only one between relativeShift and latencyShift. Exiting now.")
            sys.exit(1)

        svdevtinfoset = b2.register_module("SVDEventInfoSetter")
        svdevtinfoset.param("daqMode", 2)
        svdevtinfoset.param("SVDEventInfo", "SVDEventInfoOriginal")
        path.add_module(svdevtinfoset)

        digitizer = b2.register_module('SVDDigitizer')
        digitizer.param("ShaperDigits", "SVDShaperDigitsOriginal")
        path.add_module(digitizer)

        # emulate the 3-sample acquisition
        emulator = b2.register_module("SVD3SamplesEmulator")
        emulator.param("SVDEventInfo", "SVDEventInfoOriginal")
        emulator.param("SVDShaperDigits", "SVDShaperDigitsOriginal")
        if latencyShift == -1:
            emulator.param("chooseStartingSample", False)
        else:
            emulator.param("chooseStartingSample", True)
            emulator.param("StartingSample", latencyShift)

        if relativeShift == -1:
            emulator.param("chooseRelativeShift", False)
        else:
            emulator.param("chooseRelativeShift", True)
            emulator.param("relativeShift", relativeShift)

        emulator.param("outputSVDEventInfo", "SVDEventInfo")
        emulator.param("outputSVDShaperDigits", "SVDShaperDigits3SampleAll")
        path.add_module(emulator)

        # emulate online zero-suppression
        zsonline = b2.register_module("SVDZeroSuppressionEmulator")
        zsonline.param("ShaperDigits", "SVDShaperDigits3SampleAll")
        zsonline.param("ShaperDigitsIN", "SVDShaperDigits")
        path.add_module(zsonline)

        # 3-mixed-6 sample mode not available


def add_svd_unpacker(path):

    unpacker = b2.register_module('SVDUnpacker')
    path.add_module(unpacker)


def add_svd_unpacker_simulate3sampleDAQ(path, latencyShift=-1, relativeShift=-1):

    if relativeShift == -1 and latencyShift == -1:
        print("OOPS please choose if you want to use the relativeShift or the latencyShift. Exiting now.")
        sys.exit(1)

    if relativeShift != -1 and latencyShift != -1:
        print("OOPS please choose only one between relativeShift and latencyShift. Exiting now.")
        sys.exit(1)

    unpacker = b2.register_module('SVDUnpacker')
    unpacker.param("SVDEventInfo", "SVDEventInfoOriginal")
    unpacker.param("svdShaperDigitListName", "SVDShaperDigitsOriginal")
    path.add_module(unpacker)

    # emulate the 3-sample acquisition
    emulator = b2.register_module("SVD3SamplesEmulator")
    emulator.param("SVDEventInfo", "SVDEventInfoOriginal")
    emulator.param("SVDShaperDigits", "SVDShaperDigitsOriginal")
    if latencyShift == -1:
        emulator.param("chooseStartingSample", False)
    else:
        emulator.param("chooseStartingSample", True)
        emulator.param("StartingSample", latencyShift)

    if relativeShift == -1:
        emulator.param("chooseRelativeShift", False)
    else:
        emulator.param("chooseRelativeShift", True)
        emulator.param("relativeShift", relativeShift)

    emulator.param("outputSVDEventInfo", "SVDEventInfo")
    emulator.param("outputSVDShaperDigits", "SVDShaperDigits3SampleAll")
    path.add_module(emulator)

    # emulate online zero-suppression
    zsonline = b2.register_module("SVDZeroSuppressionEmulator")
    zsonline.param("ShaperDigits", "SVDShaperDigits3SampleAll")
    zsonline.param("ShaperDigitsIN", "SVDShaperDigits")
    path.add_module(zsonline)


def add_svd_packer(path):

    packer = b2.register_module('SVDPacker')
    path.add_module(packer)


def add_svd_SPcreation(path, isROIsimulation=False):

    if(isROIsimulation):
        svdSPCreatorName = '__ROISVDSpacePointCreator'
        svd_clusters = '__ROIsvdClusters'
        nameSPs = 'SVDSpacePoints__ROI'
    else:
        svdSPCreatorName = 'SVDSpacePointCreator'
        svd_clusters = ''
        nameSPs = 'SVDSpacePoints'

    if svdSPCreatorName not in [e.name() for e in path.modules()]:
        spCreatorSVD = b2.register_module('SVDSpacePointCreator')
        spCreatorSVD.set_name(svdSPCreatorName)
        spCreatorSVD.param('NameOfInstance', 'SVDSpacePoints')
        spCreatorSVD.param('SpacePoints', nameSPs)
        spCreatorSVD.param('SVDClusters', svd_clusters)
        path.add_module(spCreatorSVD)
