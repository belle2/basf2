#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file calibrates the PXD gain factors, the conversion factors from ADU to
# number of eletron hole pairs in the sensor. The script requires reference cluster data
# from a phase 2 beam run which can be created using the script create_reference_sample.py
#
# Execute as: basf2 calibrate_gains.py -- --refdata='r03751_all' --valdir='r03751_validation' --gains='gains.out'
#
# author: benjamin.schwenker@pyhs.uni-goettingen.de


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
import multiprocessing
import copy
import matplotlib.pyplot as plt
import pickle
import copy

# Compute calibration for this list of sensors and sensor areas
sensor_list = {"1.1.1": [0, 1], "1.1.2": [0, 1], "2.1.1": [2, 3], "2.1.2": [2, 3]}

# Folder containing all validation plots
valdir = 'ValidationPlots'

# Make cluster charge validation plots for size 1,..maxsize-1
maxsize = 7

# Use this list of shapes to fit the calibration constants
shape_list = ["FD0.0D1.0", ]

# shape_list = ["FD0.0", "FD0.0D1.0", "FD0.0D0.1", "FD0.0D1.0D2.0", "FD0.0D1.0D2.0D3.0"]
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


def filterClusters(Data, sensorID, pixelkind):
    cluster_sensorID = Belle2.VxdID(int(Data[0]))
    cluster_pixelkind = int(Data[1])

    if (cluster_pixelkind, cluster_sensorID) == (pixelkind, sensorID):
        return True
    else:
        return False


class CalculationProcess(multiprocessing.Process):
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


def validate(RefData, config, sensorID, pixelkind):

    # Might run into memory problems with large datasets
    if RefData.shape[0] > 50000:
        config.variables['nEvents'] = 50000
    else:
        config.variables['nEvents'] = RefData.shape[0]

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
    ax.set_title('Cluster Charge Sensor={} Region={}'.format(str(sensorID), pixelkind))
    ax.set_xlim(0, 200)
    ax.set_xlabel('charge / ADU')
    ax.set_ylabel('frequency')
    ax.legend(loc='upper right')
    fig.savefig(valdir + '/Validation_Charge_sensorID_{:s}_kind_{}.png'.format(str(sensorID), pixelkind))
    fig.clf()

    ax = fig.add_subplot(111)
    bins = np.arange(0, 255)
    ax.hist(CluChargeRef, bins=bins, normed=1, alpha=0.4, label='reference', align='left')
    ax.hist(CluChargeGen, bins=bins, normed=1, alpha=0.4, label='simulation: eToADU={:.0f}e/ADU'.format(eToADU), align='left')
    ax.set_title('Cluster Charge Sensor={} Region={}'.format(str(sensorID), pixelkind))
    ax.set_xlim(0, 255)
    ax.set_xlabel('charge / ADU')
    ax.set_ylabel('frequency')
    ax.legend(loc='upper right')
    ax.set_yscale('log')
    fig.savefig(valdir + '/Validation_Charge_log_sensorID_{:s}_kind_{}.png'.format(str(sensorID), pixelkind))
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
        ax.set_title('Cluster Charge ClusterSize={:d} Sensor={} Region={}'.format(i, str(sensorID), pixelkind))
        ax.set_xlim(0, 200)
        ax.set_xlabel('charge / ADU')
        ax.set_ylabel('pdf')
        ax.legend(loc='upper right')
        fig.savefig(valdir + '/Validation_Charge_for_size_{:d}_sensorID_{:s}_kind_{}.png'.format(i, str(sensorID), pixelkind))
        fig.clf()

    ref_multi_values, ref_multi_sigmas = reduceData(RefData)
    gen_multi_values, gen_multi_sigmas = reduceData(GenData)

    ax = fig.add_subplot(111)
    bins = np.arange(0, ref_multi_values.shape[0])
    ax.plot(bins, ref_multi_values, 'r--', label='reference')
    ax.plot(bins, gen_multi_values, 'b--', label='simulation: eToADU={:.0f}e/ADU'.format(eToADU))

    ax.set_title('Multi Cluster Charge Sensor={} Region={}'.format(str(sensorID), pixelkind))
    ax.set_xlabel('charge / ADU')
    ax.set_ylabel('pdf')
    ax.legend(loc='upper right')
    fig.savefig(valdir + '/Validation_MultiCharge_sensorID_{:s}_kind_{}.png'.format(str(sensorID), pixelkind))
    fig.clf()

    ax = fig.add_subplot(111)
    bins = np.arange(0, 10)
    ax.hist(CluSizeRef, bins=bins, normed=1, alpha=0.4, label='reference', align='left')
    ax.hist(CluSizeGen, bins=bins, normed=1, alpha=0.4, label='simulation', align='left')
    ax.set_title('Cluster Size Sensor={} Region={}'.format(str(sensorID), pixelkind))
    ax.set_xlabel('size / pixel')
    ax.set_ylabel('frequency')
    ax.set_xticks(bins[:-1])
    ax.legend(loc='upper right')
    fig.savefig(valdir + '/Validation_Size_sensorID_{:s}_kind_{}.png'.format(str(sensorID), pixelkind))
    fig.clf()

    ax = fig.add_subplot(111)
    bins = np.arange(0, 10)
    ax.hist(CluSizeURef, bins=bins, normed=1, alpha=0.4, label='reference', align='left')
    ax.hist(CluSizeUGen, bins=bins, normed=1, alpha=0.4, label='simulation', align='left')
    ax.set_title('Cluster Size U Sensor={} Region={}'.format(str(sensorID), pixelkind))
    ax.set_xlabel('sizeU / pixel')
    ax.set_ylabel('frequency')
    ax.set_xticks(bins[:-1])
    ax.legend(loc='upper right')
    fig.savefig(valdir + '/Validation_SizeU_sensorID_{:s}_kind_{}.png'.format(str(sensorID), pixelkind))
    fig.clf()

    ax = fig.add_subplot(111)
    bins = np.arange(0, 10)
    ax.hist(CluSizeVRef, bins=bins, normed=1, alpha=0.4, label='reference', align='left')
    ax.hist(CluSizeVGen, bins=bins, normed=1, alpha=0.4, label='simulation', align='left')
    ax.set_title('Cluster Size V Sensor={} Region={}'.format(str(sensorID), pixelkind))
    ax.set_xlabel('sizeV / pixel')
    ax.set_ylabel('frequency')
    ax.set_xticks(bins[:-1])
    ax.legend(loc='upper right')
    fig.savefig(valdir + '/Validation_SizeV_sensorID_{:s}_kind_{}.png'.format(str(sensorID), pixelkind))
    fig.clf()

    print("Validation plots created")


def fit(RefData, config):

    # Start values for minimization = default values in config
    x0 = config.getFitVariableValues()

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

    return config


def sort_and_fit(sensorID, pixelkind, Data):

    # We make an independent copy of config
    fit_config = copy.deepcopy(config)
    # Set the pixelkind
    fit_config.variables['PixelKind'] = pixelkind
    # Set a temporary output dir
    fit_config.variables['Outdir'] = config.variables['Outdir'] + '/_sensor_{:d}_kind_{:d}'.format(int(sensorID), pixelkind)

    # Create a temporary folder for generated data
    if os.path.isdir(os.getcwd() + '/' + fit_config.variables['Outdir']):
        shutil.rmtree(os.getcwd() + '/' + fit_config.variables['Outdir'])
    os.mkdir(os.getcwd() + '/' + fit_config.variables['Outdir'])

    # Filter traning data
    vf_filter_clusters = np.vectorize(filterClusters)

    # Filter track data for angle bins
    it = np.where(vf_filter_clusters(Data, sensorID, pixelkind))
    RefData = Data[it]

    # Now, let's run the calibration
    print("Start fitting gains on sensor {} and pixelkind {}  ...".format(sensorID, pixelkind))
    calibrated_config = fit(RefData, fit_config)

    # Now, let's run the validation
    print("Start validating gains on sensor {} and pixelkind {}  ...".format(sensorID, pixelkind))
    validate(RefData, calibrated_config, sensorID, pixelkind)

    # Return the results
    gains_dict = {}
    gains_dict[(str(sensorID), pixelkind)] = calibrated_config

    return gains_dict


def mp_sort_and_fit(Data):
    def worker(sensorID, pixelkinds, out_q, Data):
        for pixelkind in pixelkinds:
            outdict = sort_and_fit(sensorID, pixelkind, Data)
            out_q.put(outdict)

    # Each process will trains a pixelkinds on a grid. It will get sourcefiles and trainer and a
    # queue to put his output dict into
    out_q = multiprocessing.Queue()
    procs = []

    for key, value in sensor_list.items():
        sensorID = Belle2.VxdID(key)
        pixelkinds = value
        # Start process for sorting and training
        p = multiprocessing.Process(
            target=worker,
            args=(sensorID, pixelkinds, out_q, Data))
        procs.append(p)
        p.start()

    # Collect all results into a single result dict. We know how many dicts
    # with results to expect.
    resultdict = {}
    for i in range(4):
        resultdict.update(out_q.get())

    # Wait for all worker processes to finish
    for p in procs:
        p.join()

    return resultdict


if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser(description="Calibrate PXDDigitizer parameters against reference data")
    parser.add_argument('--refdata', dest='refdata', default='', type=str, help='Name of file with reference clusters')
    parser.add_argument('--gains', dest='gains', default='gains.out', type=str, help='Name of file with pickled calibrations')
    parser.add_argument('--valdir', dest='valdir', default='validation', type=str,
                        help='Dir to put validation plots')
    args = parser.parse_args()
    valdir = args.valdir

    # We need to configure the generation of pxd clusters
    # which are then compared to reference data
    config = GeneratePXDClusterConfig()
    config.variables['Magnet-Off'] = False
    config.variables['nEvents'] = 20000
    config.variables['Outdir'] = 'tmp_cluster'
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

    # Now, let's try to fit some parameters (Gq, Clearborder, etc.) by fitting
    # simulated cluster shapes against reference cluster shapes.
    config.addFitVariable('Gq')
    # config.addFitVariable('SourceDrainBorder')
    # config.addFitVariable('ClearBorder')

    # Create a  folder for generated data (can be removed at the end)
    if os.path.isdir(os.getcwd() + '/' + config.variables['Outdir']):
        shutil.rmtree(os.getcwd() + '/' + config.variables['Outdir'])
    os.mkdir(os.getcwd() + '/' + config.variables['Outdir'])

    # Create a  folder for validation plots
    if os.path.isdir(os.getcwd() + '/' + valdir):
        shutil.rmtree(os.getcwd() + '/' + valdir)
    os.mkdir(os.getcwd() + '/' + valdir)

    # Check if reference data exits
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

        # Create a temporary folder for refdata
        if os.path.isdir(os.getcwd() + '/' + config_ref.variables['Outdir']):
            shutil.rmtree(os.getcwd() + '/' + config_ref.variables['Outdir'])
        os.mkdir(os.getcwd() + '/' + config_ref.variables['Outdir'])

        # Simulate refdata
        p = CalculationProcess(config_ref)
        p.start()
        p.join()

        # Now we have some reference data in this folder
        args.refdata = 'tmp_refdata'

    # Create emtpy array with correct shape
    RefData = np.array(
        [], dtype='int16, int16, object, float32, float32')

    # Fill array
    for clusterfile in glob.glob(args.refdata + '/*.txt')[:5]:
        data = np.loadtxt(
            clusterfile,
            skiprows=1,
            dtype='int16, int16, object, float32, float32')
        RefData = np.concatenate((RefData, data), axis=0)

    # Now, let's perform the gain calibration
    gains_dict = mp_sort_and_fit(RefData)

    # Pickle the calibration for later use
    pickle.dump(gains_dict, open(args.gains, 'wb'))
