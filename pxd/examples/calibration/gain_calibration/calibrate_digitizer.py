#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>benjamin.schwenker@phys.uni-goettingen.de</contact>
  <description>
    Calibrate the PXD digitizer against collision data from phase 2
  </description>
</header>
"""


# Execute as: basf2 calibrate_digitizer.py -- --refdata='r03751/PXDClusters_index_0_kind_0.txt' --magnet_on --pixelkind=0

from scipy.optimize import basinhopping
from scipy.optimize import minimize
from scipy.optimize import least_squares
from basf2 import *
from ROOT import Belle2
from generate_pxdclusters import add_generate_pxdclusters_phase2
from generate_pxdclusters import GeneratePXDClusterConfig
from shape_utils import *
from FullDigitalShapes import FullDigitalShapeClassifierTrainer

import shutil
import glob
import numpy as np
from multiprocessing import Process
import copy
import matplotlib.pyplot as plt
import pickle

maxsize = 7


class CalculationProcess(Process):
    def __init__(self, config):
        self.config = config
        super(CalculationProcess, self).__init__()

    def run(self):
        path = create_path()
        add_generate_pxdclusters_phase2(path, self.config)
        process(path)
        # Print call statistics
        print(statistics)


def residuals(x, config, labelfunc, shapefunc, RefLabels, RefProbs, RefProbSigmas):

    # Now, let's update config from variable tuple x
    config.setFitVariableValues(x)
    print(config.getFitVariableValues())
    # Generate new data file
    p = CalculationProcess(config)
    p.start()
    p.join()

    # Create emtpy array with correct shape
    GenData = np.array(
        [], dtype='int16, int16, object, float32, float32')

    # Find all files with simulated data and load them into a single array
    for clusterfile in glob.glob(config.variables['Outdir'] + '/*.txt'):
        data = np.loadtxt(
            clusterfile,
            skiprows=1,
            dtype='int16, int16, object, float32, float32')
        GenData = np.concatenate((GenData, data), axis=0)

    # Reduce clusters to shapes
    shapes = shapefunc(GenData['f2'])

    # Find unique labels in shapes
    GenLabels, GenCounts = np.unique(labelfunc(shapes, thetaU=0, thetaV=0), return_counts=True)
    it = np.where(~(GenLabels == 'None'))
    GenLabels = GenLabels[it]
    GenCounts = GenCounts[it]
    GenProbs = GenCounts.astype(np.float) / GenData.shape[0]
    GenProbSigmas = np.sqrt(GenCounts.astype(np.float)) / GenData.shape[0]

    print(
        'Number of labels in generated data is {:d}. This number should be >>1.'.format(GenLabels.shape[0]))

    # residuals is an 1d array containing residuals for a labelprobs
    residuals = np.zeros(RefProbs.shape[0])

    for i, reflabel in enumerate(RefLabels):
        if RefProbs[i] < 0.001:
            continue

        # try to find matching label from simulations
        matches = np.where(GenLabels == reflabel)[0]
        if matches.shape[0] == 1:
            j = matches[0]
            if GenCounts[j] < 100:
                continue
            # store residuals scaled by their sigmas
            sigma = np.sqrt(RefProbSigmas[i]**2 + GenProbSigmas[j]**2)
            residuals[i] = (RefProbs[i] - GenProbs[j]) / sigma

    print('residuals: ', residuals)
    return residuals


if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser(description="Calibrate PXDDigitizer parameters against reference data")
    parser.add_argument('--refdata', dest='refdata', default='', type=str, help='Name of file with reference clusters')
    parser.add_argument('--valdir', dest='valdir', default='validation', type=str,
                        help='Dir to put validation plots and fit results')
    parser.add_argument('--magnet_on', dest='magnet_on', action="store_true", help='Turn on magnetic field')
    parser.add_argument('--nevents', dest='nevents', default=20000, type=int, help='Number of clusters to be simulated')
    parser.add_argument('--theta', dest='theta', default=90.0, type=float, help='Theta angle for particle gun')
    parser.add_argument('--phi', dest='phi', default=90.0, type=float, help='Phi angle for particle gun')
    parser.add_argument('--pixelkind', dest='pixelkind', default=0, type=int,
                        help='PixelKinds 0, 1, 2, 3 and 4 for z55, z60, z70, z85 and all pixels')
    parser.add_argument('--shape_classifier', dest='shape_classifier', default='', type=str, help='Name of shape classifier')
    args = parser.parse_args()

    # We need to configure the generation of pxd clusters
    # which are then compared to reference data
    config = GeneratePXDClusterConfig()
    config.variables['Magnet-Off'] = not args.magnet_on
    config.variables['nEvents'] = args.nevents
    config.variables['ThetaParams'] = [args.theta, 0.1]
    config.variables['PhiParams'] = [args.phi, 0.1]
    config.variables['PixelKind'] = args.pixelkind
    config.variables['Outdir'] = 'tmp_cluster'
    config.variables['thetaGeneration'] = 'normal'  # simulate test beam conditions
    config.variables['phiGeneration'] = 'normal'
    config.variables['Momenta'] = [2.4]
    config.variables['pdgCodes'] = [-11]
    config.variables['SourceDrainBorder'] = 6.0
    config.variables['ClearBorder'] = 4.2
    config.variables['ChargeThreshold'] = 7
    config.variables['NoiseSN'] = 3
    config.variables['SeedSN'] = 5
    config.variables['ClusterSN'] = 8
    config.variables['ElectronicNoise'] = 350
    config.variables['PedestalMean'] = 155
    config.variables['PedestalRMS'] = 45

    # Create a temporary folder for generated data
    if os.path.isdir(os.getcwd() + '/' + config.variables['Outdir']):
        shutil.rmtree(os.getcwd() + '/' + config.variables['Outdir'])
    os.mkdir(os.getcwd() + '/' + config.variables['Outdir'])

    if args.refdata == '':
        # No reference data was supplied. We will instead simulate
        # some reference data with known seetings
        config_ref = copy.deepcopy(config)
        config_ref.variables['Outdir'] = 'tmp_refdata'
        config_ref.variables['nEvents'] = 100000
        # Here we can set truth values for some parameters
        config_ref.variables['Gq'] = 1.0
        config_ref.variables['SourceDrainBorder'] = 4.0
        config_ref.variables['ClearBorder'] = 4.0
        config_ref.variables['ThetaParams'] = [args.theta, 20.0]
        config_ref.variables['PhiParams'] = [args.phi, 20.0]

        # Create a temporary folder for refdata
        if os.path.isdir(os.getcwd() + '/' + config_ref.variables['Outdir']):
            shutil.rmtree(os.getcwd() + '/' + config_ref.variables['Outdir'])
        os.mkdir(os.getcwd() + '/' + config_ref.variables['Outdir'])

        # Simulate refdata
        p = CalculationProcess(config_ref)
        p.start()
        p.join()

        # Create emtpy array with correct shape
        RefData = np.array(
            [], dtype='int16, int16, object, float32, float32')

        # Find all files with simulated refdata and load them into a single array
        for clusterfile in glob.glob(config_ref.variables['Outdir'] + '/*.txt'):
            data = np.loadtxt(
                clusterfile,
                skiprows=1,
                dtype='int16, int16, object, float32, float32')
            RefData = np.concatenate((RefData, data), axis=0)

    else:
        # Reference clusters from test beam
        refdata = args.refdata
        RefData = np.loadtxt(
            refdata,
            skiprows=1,
            dtype='int16, int16, object, float32, float32')

        # Filter for specific pixelkind
        if not args.pixelkind == 4:
            it = np.where(RefData['f1'] == args.pixelkind)
            RefData = RefData[it]

    ####################################################################
    # Fitting of calibration constants

    # create/train a shape classifier from the
    # reference data or use a pretrained one

    if args.shape_classifier == '':
        # No shape classifier was supplied. Build one and pickle it for later use.
        trainer = FullDigitalShapeClassifierTrainer(mincluster=400)
        shape_classifier = trainer.createShapeClassifier(RefData, thetaU=0.0, thetaV=0.0)
        pickle.dump(shape_classifier, open('calibrate_classifier.out', 'wb'))
        print('Shape classifier build and pickled.')
    else:
        # Use supplied shape classifier
        shape_classifier = pickle.load(open(args.shape_classifier, 'rb'))
        print('Using pretrained shape classifier.')

    # Now, let's try to fit some parameters (Gq, Clearborder, etc.) by fitting
    # simulated cluster shapes against reference cluster shapes.
    config.addFitVariable('Gq')
    config.addFitVariable('SourceDrainBorder')
    config.addFitVariable('ClearBorder')

    # Start values for minimization = default values in config
    x0 = config.getFitVariableValues()

    print("Start fitting of calibration constants ...")

    # build helper functions
    shapefunc = np.vectorize(get_shape)
    labelfunc = np.vectorize(shape_classifier.getLabelString)

    # Reduce clusters to shapes
    shapes = shapefunc(RefData['f2'])

    # Find unique labels in shapes
    RefLabels, RefCounts = np.unique(labelfunc(shapes, thetaU=0, thetaV=0), return_counts=True)
    # Remove the 'None' label. This is an overflow label that can contain all sorts of thrash
    it = np.where(~(RefLabels == 'None'))
    RefLabels = RefLabels[it]
    RefCounts = RefCounts[it]
    # Compute label probabilities
    RefProbs = RefCounts.astype(np.float) / RefData.shape[0]
    RefProbSigmas = np.sqrt(RefCounts.astype(np.float)) / RefData.shape[0]

    print(
        'Number of labels in reference data is {:d}. This number should be >>1.'.format(RefLabels.shape[0]))

    res = least_squares(
        residuals,
        x0,
        method='trf',
        loss='linear',
        ftol=1e-01,
        diff_step=(
            0.01,
            0.1,
            0.1),
        args=(
            config,
            labelfunc,
            shapefunc,
            RefLabels,
            RefProbs,
            RefProbSigmas))
    print("Minimization result: ", res.x)

    if args.refdata == '':
        print("Truth parameters used for reference simulation: \n", config_ref.variables)

    print("Final parameters after minimization: \n", config.variables)

    ####################################################################
    # Validation of calibration constants

    print("Start validation of calibration constants ...")

    # Generate some data for validation of the calibration constants
    print(config.getFitVariableValues())
    # might run into memory problems with large datasets
    if RefData.shape[0] > 400000:
        config.variables['nEvents'] = 400000
    else:
        config.variables['nEvents'] = RefData.shape[0]
    config.variables['Outdir'] = args.valdir

    # Create a temporary folder for generated data
    if os.path.isdir(os.getcwd() + '/' + config.variables['Outdir']):
        shutil.rmtree(os.getcwd() + '/' + config.variables['Outdir'])
    os.mkdir(os.getcwd() + '/' + config.variables['Outdir'])

    p = CalculationProcess(config)
    p.start()
    p.join()

    # Estimate label probabilities und their stastical uncertainty for generated data
    # use all produced validation data
    # Create emtpy array with correct shape
    GenData = np.array(
        [], dtype='int16, int16, object, float32, float32')

    # Find all files with simulated data and load them into a single array
    for clusterfile in glob.glob(config.variables['Outdir'] + '/*.txt'):
        data = np.loadtxt(
            clusterfile,
            skiprows=1,
            dtype='int16, int16, object, float32, float32')
        GenData = np.concatenate((GenData, data), axis=0)

    # Reduce clusters to shapes
    shapes = shapefunc(GenData['f2'])

    GenLabels, GenCounts = np.unique(labelfunc(shapes, thetaU=0, thetaV=0), return_counts=True)
    it = np.where(~(GenLabels == 'None'))
    GenLabels = GenLabels[it]
    GenCounts = GenCounts[it]
    GenProbs = GenCounts.astype(np.float) / GenData.shape[0]
    GenProbSigmas = np.sqrt(GenCounts.astype(np.float)) / GenData.shape[0]

    # Validation: cluster charges and sizes

    CluChargeRef = np.zeros((RefData.shape[0], 1), dtype=np.int)
    CluSizeRef = np.zeros((RefData.shape[0], 1), dtype=np.int)
    CluSizeURef = np.zeros((RefData.shape[0], 1), dtype=np.int)
    CluSizeVRef = np.zeros((RefData.shape[0], 1), dtype=np.int)

    CluChargeRefDifferential = {}
    for i in range(1, maxsize):
        CluChargeRefDifferential[i] = list()

    for j in range(RefData.shape[0]):
        shape = get_shape(RefData[j]['f2'])
        size = get_size(shape)
        charge = np.sum(get_signals(shape))
        CluChargeRef[j, 0] = charge
        CluSizeRef[j, 0] = size
        CluSizeURef[j, 0] = get_usize(shape)
        CluSizeVRef[j, 0] = get_vsize(shape)

        if size in range(1, maxsize):
            CluChargeRefDifferential[size].append(charge)

    CluChargeGen = np.zeros((GenData.shape[0], 1), dtype=np.int)
    CluSizeGen = np.zeros((GenData.shape[0], 1), dtype=np.int)
    CluSizeUGen = np.zeros((GenData.shape[0], 1), dtype=np.int)
    CluSizeVGen = np.zeros((GenData.shape[0], 1), dtype=np.int)

    CluChargeGenDifferential = {}
    for i in range(1, maxsize):
        CluChargeGenDifferential[i] = list()

    for j in range(GenData.shape[0]):
        shape = get_shape(GenData[j]['f2'])
        size = get_size(shape)
        charge = np.sum(get_signals(shape))
        CluChargeGen[j, 0] = charge
        CluSizeGen[j, 0] = size
        CluSizeUGen[j, 0] = get_usize(shape)
        CluSizeVGen[j, 0] = get_vsize(shape)

        if size in range(1, maxsize):
            CluChargeGenDifferential[size].append(charge)

    fig = plt.figure(0)

    # We do not know the ADCUnit very well. Therefor we finally
    # quote the total eToADU as result.
    eToADU = config.variables['ADCUnit'] / config.variables['Gq']

    ax = fig.add_subplot(111)
    bins = np.arange(0, 255)
    ax.hist(CluChargeRef, bins=bins, normed=1, alpha=0.4, label='reference', align='left')
    ax.hist(CluChargeGen, bins=bins, normed=1, alpha=0.4, label='simulation: eToADU={:.0f}e/ADU'.format(eToADU), align='left')
    ax.set_title('Cluster charge')
    ax.set_xlim(0, 200)
    ax.set_xlabel('charge / ADU')
    ax.set_ylabel('frequency')
    ax.legend(loc='upper right')
    fig.savefig(args.valdir + '/Validation_Charge.png')
    fig.clf()

    ax = fig.add_subplot(111)
    bins = np.arange(0, 255)
    ax.hist(CluChargeRef, bins=bins, normed=1, alpha=0.4, label='reference', align='left')
    ax.hist(CluChargeGen, bins=bins, normed=1, alpha=0.4, label='simulation: eToADU={:.0f}e/ADU'.format(eToADU), align='left')
    ax.set_title('Cluster charge')
    ax.set_xlim(0, 255)
    ax.set_xlabel('charge / ADU')
    ax.set_ylabel('frequency')
    ax.legend(loc='upper right')
    ax.set_yscale('log')
    fig.savefig(args.valdir + '/Validation_Charge_log.png')
    fig.clf()

    for i in range(1, maxsize):
        CluChargeRefTmp = np.asarray(CluChargeRefDifferential[i], dtype=np.int)
        CluChargeGenTmp = np.asarray(CluChargeGenDifferential[i], dtype=np.int)

        ax = fig.add_subplot(111)
        bins = np.arange(0, 255)
        ax.hist(CluChargeRefTmp, bins=bins, normed=1, alpha=0.4, label='reference', align='left')
        ax.hist(
            CluChargeGenTmp,
            bins=bins,
            normed=1,
            alpha=0.4,
            label='simulation: eToADU={:.0f}e/ADU'.format(eToADU),
            align='left')
        ax.set_title('Cluster charge for size={:d} clusters'.format(i))
        ax.set_xlim(0, 200)
        ax.set_xlabel('charge / ADU')
        ax.set_ylabel('pdf')
        ax.legend(loc='upper right')
        fig.savefig(args.valdir + '/Validation_Charge_for_size_{:d}.png'.format(i))
        fig.clf()

    ax = fig.add_subplot(111)
    bins = np.arange(0, 10)
    ax.hist(CluSizeRef, bins=bins, normed=1, alpha=0.4, label='reference', align='left')
    ax.hist(CluSizeGen, bins=bins, normed=1, alpha=0.4, label='simulation', align='left')
    ax.set_title('Cluster size')
    ax.set_xlabel('size / pixel')
    ax.set_ylabel('frequency')
    ax.set_xticks(bins[:-1])
    ax.legend(loc='upper right')
    fig.savefig(args.valdir + '/Validation_Size.png')
    fig.clf()

    ax = fig.add_subplot(111)
    bins = np.arange(0, 10)
    ax.hist(CluSizeURef, bins=bins, normed=1, alpha=0.4, label='reference', align='left')
    ax.hist(CluSizeUGen, bins=bins, normed=1, alpha=0.4, label='simulation', align='left')
    ax.set_title('Cluster size U')
    ax.set_xlabel('sizeU / pixel')
    ax.set_ylabel('frequency')
    ax.set_xticks(bins[:-1])
    ax.legend(loc='upper right')
    fig.savefig(args.valdir + '/Validation_SizeU.png')
    fig.clf()

    ax = fig.add_subplot(111)
    bins = np.arange(0, 10)
    ax.hist(CluSizeVRef, bins=bins, normed=1, alpha=0.4, label='reference', align='left')
    ax.hist(CluSizeVGen, bins=bins, normed=1, alpha=0.4, label='simulation', align='left')
    ax.set_title('Cluster size V')
    ax.set_xlabel('sizeV / pixel')
    ax.set_ylabel('frequency')
    ax.set_xticks(bins[:-1])
    ax.legend(loc='upper right')
    fig.savefig(args.valdir + '/Validation_SizeV.png')
    fig.clf()

    print("Validation plots created")
