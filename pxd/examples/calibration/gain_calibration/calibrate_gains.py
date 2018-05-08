#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>benjamin.schwenker@phys.uni-goettingen.de</contact>
  <description>
    Calibrate the PXD gains against collision data from phase 2
  </description>
</header>
"""


# Execute as: basf2 calibrate_gains.py -- --refdata='r03751/PXDClusters_index_0_kind_0.txt' --magnet_on --pixelkind=0

from scipy.optimize import basinhopping
from scipy.optimize import minimize
from scipy.optimize import least_squares
from basf2 import *
from ROOT import Belle2
from generate_pxdclusters import add_generate_pxdclusters_phase2
from generate_pxdclusters import GeneratePXDClusterConfig
from shape_utils import *

import shutil
import glob
import numpy as np
from multiprocessing import Process
import copy
import matplotlib.pyplot as plt
import pickle

maxsize = 7
# shape_list = ["FD0.0", "FD0.0D1.0", "FD0.0D0.1", "FD0.0D1.0D2.0", "FD0.0D1.0D2.0D3.0"]

shape_list = ["FD0.0D1.0", ]  # "FD0.0D0.1"]

# Shape FD0.0D0.1D1.0 has prob 0.023260465209304186
# Shape FD0.0D0.1D1.1 has prob 0.022740454809096183
# Shape FD0.0D1.0D1.1 has prob 0.022100442008840177
# Shape FD0.1D1.0D1.1 has prob 0.021940438808776174
# Shape FD0.0D1.0D2.0D3.0 has prob 0.01752035040700814
# Shape FD0.1D1.0 has prob 0.01386027720554411
# Shape FD0.0D1.1 has prob 0.013020260405208104
# Shape FD0.0D0.1D0.2 has prob 0.01122022440448809
# Shape FD0.0D0.1D1.0D1.1 has prob 0.010780215604312086
# Shape FD0.0D1.0D2.0D3.0D4.0 has prob 0.010160203204064081
# Shape FD0.1D1.0D1.1D2.0 has prob 0.009380187603752074
# Shape FD0.0D1.0D2.0D3.0D4.0D5.0 has prob 0.009180183603672074
# Shape FD0.0D1.0D1.1D2.1 has prob 0.008820176403528071


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


def reduceData(Data):

    CluChargeMap = {}
    for shape in shape_list:
        CluChargeMap[shape] = list()

    for j in range(Data.shape[0]):
        analog_shape = get_shape(Data[j]['f2'])
        charge = np.sum(get_signals(analog_shape))
        shape = get_digital_label(analog_shape)

        if shape in shape_list:
            CluChargeMap[shape].append(charge)

    CluChargeMultiHistValues = list()
    CluChargeMultiHistSigmas = list()

    for shape in shape_list:

        values, bin_edges = np.histogram(np.asarray(CluChargeMap[shape], dtype=np.int), bins=np.arange(0, 255, 8))
        values[values < 80] = 0
        norm = np.sum(values)
        sigmas = np.sqrt(values) / norm
        values = values / norm

        CluChargeMultiHistValues.extend(values)
        CluChargeMultiHistSigmas.extend(sigmas)

    return np.asarray(CluChargeMultiHistValues), np.asarray(CluChargeMultiHistSigmas)


def residuals(x, config, ref_values, ref_sigmas):

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

    # Reduce data
    gen_values, gen_sigmas = reduceData(GenData)

    # Compute the residuals
    sigmas = np.square(np.power(ref_sigmas, 2) + np.power(gen_sigmas, 2))
    sigmas[sigmas <= 0] = 1

    residuals = (gen_values - ref_values)  # /sigmas
    residuals[np.isnan(residuals)] = 0
    return residuals


if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser(description="Calibrate PXDDigitizer parameters against reference data")
    parser.add_argument('--refdata', dest='refdata', default='', type=str, help='Name of file with reference clusters')
    parser.add_argument('--valdir', dest='valdir', default='validation', type=str,
                        help='Dir to put validation plots and fit results')
    parser.add_argument('--magnet_on', dest='magnet_on', action="store_true", help='Turn on magnetic field')
    parser.add_argument('--pixelkind', dest='pixelkind', default=0, type=int,
                        help='PixelKinds 0, 1, 2, 3 and 4 for z55, z60, z70, z85 and all pixels')
    args = parser.parse_args()

    # We need to configure the generation of pxd clusters
    # which are then compared to reference data
    config = GeneratePXDClusterConfig()
    config.variables['Magnet-Off'] = not args.magnet_on
    config.variables['nEvents'] = 20000
    # config.variables['ThetaParams'] = [0.0, 180.0] #[90, 0.1]
    # config.variables['PhiParams'] = [0.0, 180.0]   #[90, 0.1]
    config.variables['PixelKind'] = args.pixelkind
    config.variables['Outdir'] = 'tmp_cluster'
    # config.variables['thetaGeneration'] = 'normal'  # simulate test beam conditions
    # config.variables['phiGeneration'] = 'normal'
    config.variables['Gq'] = 0.6
    config.variables['Momenta'] = [0.01]
    config.variables['pdgCodes'] = [-11]
    config.variables['SourceDrainBorder'] = 6.0
    config.variables['ClearBorder'] = 4.2
    config.variables['ChargeThreshold'] = 8
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
        config_ref.variables['nEvents'] = 40000
        # Here we can set truth values for some parameters
        config_ref.variables['Gq'] = 1.0
        # config_ref.variables['SourceDrainBorder'] = 4.0
        # config_ref.variables['ClearBorder'] = 4.0
        # config_ref.variables['ThetaParams'] = [90, 20.0]
        # config_ref.variables['PhiParams'] = [90, 20.0]

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

    # Now, let's try to fit some parameters (Gq, Clearborder, etc.) by fitting
    # simulated cluster shapes against reference cluster shapes.
    config.addFitVariable('Gq')
    # config.addFitVariable('SourceDrainBorder')
    # config.addFitVariable('ClearBorder')

    # Start values for minimization = default values in config
    x0 = config.getFitVariableValues()

    print("Start fitting of calibration constants ...")

    ref_values, ref_sigmas = reduceData(RefData)

    res = least_squares(
        residuals,
        x0,
        method='trf',
        loss='linear',
        ftol=1e-01,
        diff_step=(0.01,),
        args=(
            config,
            ref_values,
            ref_sigmas))

    print("Minimization result: ", res.x)
    config.setFitVariableValues(res.x)

    if args.refdata == '':
        print("Truth parameters used for reference simulation: \n", config_ref.variables)

    print("Final parameters after minimization: \n", config.variables)

    ####################################################################
    # Validation of calibration constants

    print("Start validation of calibration constants ...")

    # Generate some data for validation of the calibration constants
    print(config.getFitVariableValues())
    # might run into memory problems with large datasets
    if RefData.shape[0] > 50000:
        config.variables['nEvents'] = 50000
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

    ref_multi_values, ref_multi_sigmas = reduceData(RefData)
    gen_multi_values, gen_multi_sigmas = reduceData(GenData)

    ax = fig.add_subplot(111)
    bins = np.arange(0, ref_multi_values.shape[0])
    ax.plot(bins, ref_multi_values, 'r--', label='reference')
    ax.plot(bins, gen_multi_values, 'b--', label='simulation: eToADU={:.0f}e/ADU'.format(eToADU))

    ax.set_title('Multi cluster charge')
    ax.set_xlabel('charge / ADU')
    ax.set_ylabel('pdf')
    ax.legend(loc='upper right')
    fig.savefig(args.valdir + '/Validation_MultiCharge.png')
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
