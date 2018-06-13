#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
from basf2 import *
from ROOT import Belle2

from iov_conditional import phase_2_conditional


def add_svd_reconstruction(path, isROIsimulation=False, useNN=False, useCoG=True, applyMasking=False):

    if(useNN and useCoG):
        print("WARNING! you can't select both NN and CoG for SVD reconstruction. Using the default algorithm (TB-equivalent)")
        add_svd_reconstruction_tb(path, isROIsimulation)

    elif(not useNN and not useCoG):
        add_svd_reconstruction_tb(path, isROIsimulation)

    elif(useNN):
        add_svd_reconstruction_nn(path, isROIsimulation)

    elif(useCoG):
        add_svd_reconstruction_CoG(path, isROIsimulation, applyMasking)


def add_svd_reconstruction_tb(path, isROIsimulation=False):

    if(isROIsimulation):
        splitterName = '__ROISVDDigitSplitter'
        sorterName = '__ROISVDDigitSorter'
        clusterizerName = '__ROISVDClusterizer'
        clusterName = '__ROIsvdClusters'
    else:
        splitterName = 'SVDDigitSplitter'
        sorterName = 'SVDDigitSorter'
        clusterizerName = 'SVDClusterizer'
        clusterName = ""

    if splitterName not in [e.name() for e in path.modules()]:
        splitter = register_module('SVDDigitSplitter')
        splitter.set_name(splitterName)
        path.add_module(splitter)

    if sorterName not in [e.name() for e in path.modules()]:
        sorter = register_module('SVDDigitSorter')
        sorter.set_name(sorterName)
        path.add_module(sorter)

    if clusterizerName not in [e.name() for e in path.modules()]:
        clusterizer = register_module('SVDClusterizer')
        clusterizer.set_name(clusterizerName)
        clusterizer.param('Clusters', clusterName)
        path.add_module(clusterizer)


def add_svd_reconstruction_nn(path, isROIsimulation=False, direct=False):

    if direct:
        if(isROIsimulation):
            clusterizerName = '__ROISVDClusterizerDirect'
            clusterName = '__ROIsvdClusters'
        else:
            clusterizerName = 'SVDClusterizerDirect'
            clusterName = ""

        if clusterizerName not in [e.name() for e in path.modules()]:
            clusterizer = register_module('SVDClusterizerDirect')
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
            fitter = register_module('SVDNNShapeReconstructor')
            fitter.set_name(fitterName)
            path.add_module(fitter)

        if clusterizerName not in [e.name() for e in path.modules()]:
            clusterizer = register_module('SVDNNClusterizer')
            clusterizer.set_name(clusterizerName)
            clusterizer.param('Clusters', clusterName)
            path.add_module(clusterizer)


def add_svd_reconstruction_CoG(path, isROIsimulation=False, applyMasking=False):

    if(isROIsimulation):
        fitterName = '__ROISVDCoGTimeEstimator'
        clusterizerName = '__ROISVDSimpleClusterizer'
        clusterName = '__ROIsvdClusters'
        recoDigitsName = '__ROIsvdRecoDigits'
        shaperDigitsName = ""
    else:
        fitterName = 'SVDCoGTimeEstimator'
        clusterizerName = 'SVDSimpleClusterizer'
        clusterName = ""
        recoDigitsName = ""
        shaperDigitsName = ""

# add strip masking if needed
    if(applyMasking):
        if(isROIsimulation):
            shaperDigitsName = '__ROISVDShaperDigitsUnmasked'
            maskingName = '__ROISVDStripMasking'
        else:
            shaperDigitsName = 'SVDShaperDigitsUnmasked'
            maskingName = 'SVDStripMasking'

        if maskingName not in [e.name() for e in path.modules()]:
            masking = register_module('SVDStripMasking')
            masking.set_name(maskingName)
            masking.param('ShaperDigitsUnmasked', shaperDigitsName)
            path.add_module(masking)

# Create phase 2 path
    phase2_path = basf2.create_path()

    if fitterName not in [e.name() for e in path.modules()]:
        fitter = register_module('SVDCoGTimeEstimator')
        fitter.set_name(fitterName)
        fitter.param('Correction_StripCalPeakTime', True)
        fitter.param('Correction_TBTimeWindow', False)
        fitter.param('Correction_ShiftMeanToZero', False)
        fitter.param('Correction_ShiftMeanToZeroTBDep', False)
        fitter.param('ShaperDigits', shaperDigitsName)
        fitter.param('RecoDigits', recoDigitsName)
        phase2_path.add_module(fitter)

# Create phase 3 path
    phase3_path = basf2.create_path()

    if fitterName not in [e.name() for e in path.modules()]:
        fitter = register_module('SVDCoGTimeEstimator')
        fitter.set_name(fitterName)
        fitter.param('Correction_StripCalPeakTime', True)
        fitter.param('Correction_TBTimeWindow', True)
        fitter.param('Correction_ShiftMeanToZero', False)
        fitter.param('Correction_ShiftMeanToZeroTBDep', False)
        fitter.param('RecoDigits', recoDigitsName)
        phase3_path.add_module(fitter)

    # Add IoVDependentCondition Module that selects p2 or p3 path
    phase_2_conditional(path, phase2_path=phase2_path, phase3_path=phase3_path)

    if clusterizerName not in [e.name() for e in path.modules()]:
        clusterizer = register_module('SVDSimpleClusterizer')
        clusterizer.set_name(clusterizerName)
        clusterizer.param('RecoDigits', recoDigitsName)
        clusterizer.param('Clusters', clusterName)
        path.add_module(clusterizer)

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
            clusterizer = register_module('SVDClusterizerDirect')
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
            fitter = register_module('SVDNNShapeReconstructor')
            fitter.set_name(fitterName)
            path.add_module(fitter)

        if clusterizerName not in [e.name() for e in path.modules()]:
            clusterizer = register_module('SVDNNClusterizer')
            clusterizer.set_name(clusterizerName)
            clusterizer.param('Clusters', clusterName)
            path.add_module(clusterizer)


def add_svd_simulation(path, createDigits=False):

    digitizer = register_module('SVDDigitizer')
    if createDigits:
        digitizer.param('GenerateDigits', True)
    path.add_module(digitizer)


def add_svd_unpacker(path):

    unpacker = register_module('SVDUnpacker')
    path.add_module(unpacker)


def add_svd_packer(path):

    path.add_module('SVDDigitSplitter')
    path.add_module('SVDDigitSorter')
    packer = register_module('SVDPacker')
    path.add_module(packer)


def add_svd_SPcreation(path, isROIsimulation=False):

    if(isROIsimulation):
        svdSPCreatorName = '__ROISVDSpacePointCreator'
        svd_clusters = '__ROIsvdClusters'
        nameSPs = 'SpacePoints__ROI'
    else:
        svdSPCreatorName = 'SVDSpacePointCreator'
        svd_clusters = ''
        nameSPs = 'SpacePoints'

    # Create phase2 path
    phase2_path = basf2.create_path()

    if svdSPCreatorName not in [e.name() for e in path.modules()]:
        # always use svd!
        spCreatorSVD = register_module('SVDSpacePointCreator')
        spCreatorSVD.set_name(svdSPCreatorName)
        spCreatorSVD.param('OnlySingleClusterSpacePoints', False)
        spCreatorSVD.param('NameOfInstance', 'SVDSpacePoints')
        spCreatorSVD.param('SpacePoints', nameSPs)
        spCreatorSVD.param('SVDClusters', svd_clusters)
        # remove the cut on cluster time for Phase2 reconstruction
        spCreatorSVD.param('MinClusterTime', -999)
        phase2_path.add_module(spCreatorSVD)

    # Create phase3 path
    phase3_path = basf2.create_path()

    if svdSPCreatorName not in [e.name() for e in path.modules()]:
        # always use svd!
        spCreatorSVD = register_module('SVDSpacePointCreator')
        spCreatorSVD.set_name(svdSPCreatorName)
        spCreatorSVD.param('OnlySingleClusterSpacePoints', False)
        spCreatorSVD.param('NameOfInstance', 'SVDSpacePoints')
        spCreatorSVD.param('SpacePoints', nameSPs)
        spCreatorSVD.param('SVDClusters', svd_clusters)
        phase3_path.add_module(spCreatorSVD)

    # Add IoVDependentCondition Module that selects phase2 or phase3 path
    phase_2_conditional(path, phase2_path=phase2_path, phase3_path=phase3_path)
