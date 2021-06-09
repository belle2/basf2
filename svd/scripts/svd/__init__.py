#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
import sys


def add_svd_reconstruction(path, isROIsimulation=False, createRecoDigits=False, applyMasking=False):
    """
    Adds the SVD reconstruction to the path.

    Reconstruction starts with :ref:`SVDShaperDigits<svdshapers>`
    and :ref:`SVDEventInfo<svdeventinfo>` and provides
    :ref:`SVDClusters<svdclusters>` and :ref:`SVDSpacePoints<svdsps>`.

    @param path: add the modules to this basf2 path.
    @param isROIsimulation: SVD reconstruction can be run during simulation\
    in order to simulate the PXD Data Reduction with ROI finding.
    @param createRecoDigits: if True, :ref:`SVDRecoDigits<svdrecos>` are created.
    @param applyMasking: if True, hot strips found in :ref:`SVDHotStripsCalibration<svdhotstrips>` are masked.
    """

    if(isROIsimulation):
        clusterizerName = '__ROISVDClusterizer'
        recocreatorName = '__ROISVDRecoDigitCreator'
        dataFormatName = '__ROISVDDataFormat'
        # recoDigitsName = '__ROIsvdRecoDigits'
        clustersName = '__ROIsvdClusters'
        shaperDigitsName = ""
        missingAPVsClusterCreatorName = '__ROISVDMissingAPVsClusterCreator'
    else:
        clusterizerName = 'SVDClusterizer'
        recocreatorName = 'SVDRecoDigitCreator'
        dataFormatName = 'SVDDataFormat'
        # recoDigitsName = ""
        clustersName = ""
        shaperDigitsName = ""
        missingAPVsClusterCreatorName = 'SVDMissingAPVsClusterCreator'

        # mask HotStrips from SVDHotStripsCalibration payloads
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

    # data format check NOT appended
    if dataFormatName not in [e.name() for e in path.modules()]:
        dataFormat = b2.register_module('SVDDataFormatCheck')
        dataFormat.param('ShaperDigits', shaperDigitsName)

    if clusterizerName not in [e.name() for e in path.modules()]:
        clusterizer = b2.register_module('SVDClusterizer')
        clusterizer.set_name(clusterizerName)
        clusterizer.param('ShaperDigits', shaperDigitsName)
        clusterizer.param('Clusters', clustersName)
        clusterizer.param('timeAlgorithm6Samples', "CoG3")
        clusterizer.param('timeAlgorithm3Samples', "CoG3")
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
        missingAPVCreator = b2.register_module('SVDMissingAPVsClusterCreator')
        missingAPVCreator.set_name(missingAPVsClusterCreatorName)
        path.add_module(missingAPVCreator)

    # Add SVDSpacePointCreator
    add_svd_SPcreation(path, isROIsimulation)

    if createRecoDigits and not isROIsimulation:
        # Add SVDRecoDigit creator module if not ROI simulation
        # useful for SVD performance studies
        add_svd_create_recodigits(path, recocreatorName, shaperDigitsName)


def add_svd_create_recodigits(path, recocreatorName="SVDRecoDigitCreator", shaperDigitsName=""):
    """
    Adds the strip reconstruction to the path.

    Produce :ref:`SVDRecoDigits<svdrecos>` from :ref:`SVDShaperDigits<svdshapers>`.

    @param path: add the modules to this basf2 path.
    @param recocreatorName: name of the module.
    @param shaperDigitsName: name of the SVDShaperDigits StoreArray.
 """

    if recocreatorName not in [e.name() for e in path.modules()]:
        recoDigitCreator = b2.register_module('SVDRecoDigitCreator')
        recoDigitCreator.param('ShaperDigits', shaperDigitsName)
        recoDigitCreator.param('timeAlgorithm6Samples', "CoG6")
        recoDigitCreator.param('timeAlgorithm3Samples', "CoG6")
        recoDigitCreator.param('chargeAlgorithm6Samples', "MaxSample")
        recoDigitCreator.param('chargeAlgorithm3Samples', "MaxSample")
        recoDigitCreator.param('useDB', False)
        path.add_module(recoDigitCreator)


def add_rel5_svd_reconstruction(path, isROIsimulation=False, applyMasking=False):
    """
    Adds the old (up to release-05) SVD recontruction to the path.

    Reconstruction starts with :ref:`SVDShaperDigits<svdshapers>` and
    :ref:`SVDEventInfo<svdeventinfo>` and provides :ref:`SVDClusters<svdclusters>`
    and :ref:`SVDSpacePoints<svdsps>`.

    @param path: add the modules to this basf2 path.
    @param isROIsimulation: SVD reconstruction can be run during simulation\
    in order to simulate the PXD Data Reduction with ROI finding.
    @param applyMasking: if True, hot strips found in :ref:`SVDHotStripsCalibration<svdhotstrips>` are masked.
    """

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


def add_svd_simulation(path, useConfigFromDB=False, daqMode=2, relativeShift=9):
    """
    Adds the SVD simulation to the path.

    Simulation ends with :ref:`SVDShaperDigits<svdshapers>` and :ref:`SVDEventInfo<svdeventinfo>`.

    @param path: add the modules to this basf2 path.
    @param useConfigFromDB: if True, read the SVD configuration from :ref:`SVDGlobalConfigParameters<svdglobalconfig>`.
    @param daqMode: = 2 for the default 6-sample mode, = 1 for the 3-sample mode, = 3 for the 3-mixed-6 sample mode.
    @param relativeShift: relative time shift between the 3-sample and the 6-sample mode in units of 1/4 of APV clock.\
    If ``useConfigFromDB`` is True, the value of this parameter is overwritten.
    """

    svdevtinfoset = b2.register_module("SVDEventInfoSetter")
    svdevtinfoset.param("useDB", useConfigFromDB)
    path.add_module(svdevtinfoset)

    digitizer = b2.register_module('SVDDigitizer')
    path.add_module(digitizer)

    if not useConfigFromDB:
        if daqMode != 2 and daqMode != 1 and daqMode != 3:
            print("OOPS the acquisition mode that you want to simulate is not available.")
            print("Please choose among daqMode = 2 (6-sample) and daqMode = 1 (3-sample). Exiting now.")
            sys.exit()

        # TODO add check of relative shift value
        svdevtinfoset.param("daqMode", daqMode)
        svdevtinfoset.param("relativeShift", relativeShift)


def add_svd_unpacker(path):
    """
    Adds the SVD Unpacker to the path.

    The unpacker produces :ref:`SVDShaperDigits<svdshapers>` and :ref:`SVDEventInfo<svdeventinfo>`.

    @param path: add the modules to this basf2 path.
    """

    unpacker = b2.register_module('SVDUnpacker')
    path.add_module(unpacker)


def add_svd_unpacker_simulate3sampleDAQ(path, latencyShift=-1, relativeShift=-1):
    """
    Adds the SVD Unpacker to the path, emulating the 3-sample mode from the 6-sample mode.

    @param path: add the modules to this basf2 path.
    @param latencyShift: relative time shift between the 3-sample and the 6-sample mode, in APV clocks.\
                         0 <= latencyShift <=3
    @param relativeShift: relative time shift between the 3-sample and the 6-sample mode, in units of 1/4 of APV clock.\
                         0 <= relativeShift <=12

    .. warning:: at least one between ``relativeShift`` and ``latencyShift`` should be set (different from -1).
    """

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
        if latencyShift < 0 or latencyShift > 3:
            B2FATAL("the latencyShift must be an integer >=0 and <= 3")
        else:
            emulator.param("StartingSample", latencyShift)

    if relativeShift == -1:
        emulator.param("chooseRelativeShift", False)
    else:
        emulator.param("chooseRelativeShift", True)
        if relativeShift < 0 or relativeShift > 12:
            B2FATAL("the relativeShift must be an integer >=0 and <= 12")
        else:
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
    """
    Adds the SVD Packer to the path.

    @param path: add the modules to this basf2 path.
    """

    packer = b2.register_module('SVDPacker')
    path.add_module(packer)


def add_svd_SPcreation(path, isROIsimulation=False):
    """
    Adds the SVD SpacePoint Creator to the path.

    @param path: add the modules to this basf2 path.
    @param isROIsimulation: SVD reconstruction can be run during simulation\
    in order to simulate the PXD Data Reduction with ROI finding.
    """

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
