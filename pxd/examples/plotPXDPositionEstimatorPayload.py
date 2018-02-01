#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Created on Wed Jan  31 20:54:00 2018
Plot PXD position estimator payload
@author: benni
"""

import math
import numpy as np
import matplotlib.pyplot as plt
import argparse
import os
import shutil
import pandas as pd
import seaborn as sns

import ROOT
from ROOT import Belle2


def getPitchV(pixelkind=0):
    """Pixel pitch in mm"""
    if pixelkind == 0:
        return 0.055
    elif pixelkind == 1:
        return 0.060
    elif pixelkind == 2:
        return 0.070
    elif pixelkind == 3:
        return 0.085


def getPitchU(pixelkind=0):
    """Pixel pitch in mm"""
    return 0.05


def average_covariance(shape_classifier):
    flat_covs = []
    weights = []

    offsetMap = shape_classifier.getOffsetMap()
    likelyhoodMap = shape_classifier.getLikelyhoodMap()

    for item in offsetMap:
        shape_index = item.first
        offsets = item.second

        for eta_index, offset in enumerate(offsets):
            likelyhood = likelyhoodMap[shape_index][eta_index]
            flat_covs.append([offset.getUSigma2(), offset.getVSigma2(), offset.getUVCovariance()])
            weights.append(likelyhood)

    weights = np.array(weights)
    flat_covs = np.array(flat_covs)
    flat_average = np.average(flat_covs, axis=0, weights=weights)
    return np.array([[flat_average[0], flat_average[2]], [flat_average[2], flat_average[1]]])


def plot_estimator(estimator, indexer, resultdir):

    # Remove old stuff
    if os.path.isdir(os.getcwd() + '/' + resultdir):
        shutil.rmtree(os.getcwd() + '/' + resultdir)

    # Create nice clean folder for results
    os.mkdir(os.getcwd() + '/' + resultdir)

    for pixelkind in estimator.getPixelkinds():

        # Keep some numbers to gain overview
        summary_dict = {'shapes': [],
                        'corrections': [],
                        'coverage': [],
                        'sigma_u': [],
                        'sigma_v': [],
                        'corr_uv': [],
                        'thetaU': [],
                        'thetaV': [],
                        'pixelkind': [],
                        }

        # Loop over angular grid and plot shape classifiers
        for uBin in range(1, estimator.getGrid(pixelkind).GetXaxis().GetNbins() + 1):
            for vBin in range(1, estimator.getGrid(pixelkind).GetYaxis().GetNbins() + 1):

                # Shape classifier for angle bin
                shape_classifier = estimator.getShapeClassifier(uBin, vBin, pixelkind)

                # Bin is centered around angles
                thetaU = estimator.getGrid(pixelkind).GetXaxis().GetBinCenter(uBin)
                thetaV = estimator.getGrid(pixelkind).GetYaxis().GetBinCenter(vBin)

                # Create  plots for classifier
                stats = plot_classifier(shape_classifier, indexer, pixelkind)

                # Fill summary dict
                summary_dict['thetaU'].append(thetaU)
                summary_dict['thetaV'].append(thetaV)
                summary_dict['pixelkind'].append(pixelkind)
                summary_dict['shapes'].append(stats['shapes'])
                summary_dict['corrections'].append(stats['corrections'])
                summary_dict['coverage'].append(stats['coverage'])
                summary_dict['sigma_u'].append(stats['sigma_u'] * 1000)  # in um
                summary_dict['sigma_v'].append(stats['sigma_v'] * 1000)  # in um
                summary_dict['corr_uv'].append(stats['corr_uv'])

        # Create heatmaps to gain overview
        df = pd.DataFrame(summary_dict)

        pivot_table = df.pivot(index='thetaU', columns='thetaV', values='corrections')
        fig = plt.figure(figsize=(12, 12))
        ax = fig.add_subplot(111)
        ax.set_xlabel('thetaU / degree', size=20)
        ax.set_ylabel('thetaV / degree', size=20)
        ax.set_title('Number of corrections kind={:d}'.format(pixelkind), size=20)
        ax = sns.heatmap(
            pivot_table,
            mask=pivot_table.isnull(),
            annot=True,
            fmt="d",
            linewidths=.5,
            square=True,
            cmap='Blues_r',
            cbar_kws={
                'label': '#corrections'})
        ax.invert_yaxis()
        fig.savefig(resultdir + '/Corrections_Heatmap_kind_{:d}.png'.format(pixelkind), dpi=100)
        fig.clf()
        plt.close(fig)

        pivot_table = df.pivot(index='thetaU', columns='thetaV', values='shapes')
        fig = plt.figure(figsize=(12, 12))
        ax = fig.add_subplot(111)
        ax.set_xlabel('thetaU / degree', size=20)
        ax.set_ylabel('thetaV / degree', size=20)
        ax.set_title('Number of shapes kind={:d}'.format(pixelkind), size=20)
        ax = sns.heatmap(
            pivot_table,
            mask=pivot_table.isnull(),
            annot=True,
            fmt="d",
            linewidths=.5,
            square=True,
            cmap='Blues_r',
            cbar_kws={
                'label': '#shapes'})
        ax.invert_yaxis()
        fig.savefig(resultdir + '/Shapes_Heatmap_kind_{:d}.png'.format(pixelkind), dpi=100)
        fig.clf()
        plt.close(fig)

        pivot_table = df.pivot(index='thetaU', columns='thetaV', values='coverage')
        fig = plt.figure(figsize=(12, 12))
        ax = fig.add_subplot(111)
        ax.set_xlabel('thetaU / degree', size=20)
        ax.set_ylabel('thetaV / degree', size=20)
        ax.set_title('Coverage kind={:d}'.format(pixelkind), size=20)
        ax = sns.heatmap(
            pivot_table,
            mask=pivot_table.isnull(),
            annot=True,
            fmt=".1f",
            linewidths=.5,
            square=True,
            cmap='Blues_r',
            cbar_kws={
                'label': 'coverage / %'})
        ax.invert_yaxis()
        fig.savefig(resultdir + '/Coverage_Heatmap_kind_{:d}.png'.format(pixelkind), dpi=100)
        fig.clf()
        plt.close(fig)

        pivot_table = df.pivot(index='thetaU', columns='thetaV', values='sigma_u')
        fig = plt.figure(figsize=(12, 12))
        ax = fig.add_subplot(111)
        ax.set_xlabel('thetaU / degree', size=20)
        ax.set_ylabel('thetaV / degree', size=20)
        ax.set_title('Average cluster sigma u kind={:d}'.format(pixelkind), size=20)
        ax = sns.heatmap(
            pivot_table,
            mask=pivot_table.isnull(),
            annot=True,
            fmt=".1f",
            linewidths=.5,
            square=True,
            cmap='Blues_r',
            cbar_kws={
                'label': 'sigma u / um'})
        ax.invert_yaxis()
        fig.savefig(resultdir + '/SigmaU_Heatmap_kind_{:d}.png'.format(pixelkind), dpi=100)
        fig.clf()
        plt.close(fig)

        pivot_table = df.pivot(index='thetaU', columns='thetaV', values='sigma_v')
        fig = plt.figure(figsize=(12, 12))
        ax = fig.add_subplot(111)
        ax.set_xlabel('thetaU / degree', size=20)
        ax.set_ylabel('thetaV / degree', size=20)
        ax.set_title('Average cluster sigma v kind={:d}'.format(pixelkind), size=20)
        ax = sns.heatmap(
            pivot_table,
            mask=pivot_table.isnull(),
            annot=True,
            fmt=".1f",
            linewidths=.5,
            square=True,
            cmap='Blues_r',
            cbar_kws={
                'label': 'sigma v / um'})
        ax.invert_yaxis()
        fig.savefig(resultdir + '/SigmaV_Heatmap_kind_{:d}.png'.format(pixelkind), dpi=100)
        fig.clf()
        plt.close(fig)

        pivot_table = df.pivot(index='thetaU', columns='thetaV', values='corr_uv')
        fig = plt.figure(figsize=(12, 12))
        ax = fig.add_subplot(111)
        ax.set_xlabel('thetaU / degree', size=20)
        ax.set_ylabel('thetaV / degree', size=20)
        ax.set_title('Average uv correlation kind={:d}'.format(pixelkind), size=20)
        ax = sns.heatmap(
            pivot_table,
            mask=pivot_table.isnull(),
            annot=True,
            fmt=".1f",
            linewidths=.5,
            square=True,
            cmap='Blues_r',
            cbar_kws={
                'label': 'correlation'})
        ax.invert_yaxis()
        fig.savefig(resultdir + '/CorrelationUV_Heatmap_kind_{:d}.png'.format(pixelkind), dpi=100)
        fig.clf()
        plt.close(fig)


def plot_classifier(shape_classifier, indexer, pixelkind):

    # Set the pixel pitch
    PitchU = getPitchU(pixelkind)
    PitchV = getPitchV(pixelkind)

    # Read corrections data
    offsetMap = shape_classifier.getOffsetMap()
    likelyhoodMap = shape_classifier.getLikelyhoodMap()
    shapeLikelyhoodMap = shape_classifier.getShapeLikelyhoodMap()

    # Some counters
    nCorrections = 0
    nShapes = 0
    coverage = 0.0

    for item in offsetMap:
        shape_index = item.first
        shape_name = indexer.getShapeString(shape_index)
        offsets_array = item.second
        shape_likelyhood = shapeLikelyhoodMap[shape_index]

        nShapes += 1

        for eta_index, offset in enumerate(offsets_array):
            coverage += likelyhoodMap[shape_index][eta_index]
            nCorrections += 1

    if nShapes > 0:
        # Average covariance of all hits in classifier
        cov = average_covariance(shape_classifier)
        sigma_u = np.sqrt(cov[0, 0])
        sigma_v = np.sqrt(cov[1, 1])
        corr_uv = cov[0, 1] / (np.sqrt(cov[0, 0]) * np.sqrt(cov[1, 1]))
    else:
        sigma_u = 0.0
        sigma_v = 0.0
        corr_uv = 0.0

    # Create some summary statistics
    stats = {}
    stats['shapes'] = nShapes
    stats['corrections'] = nCorrections
    stats['coverage'] = 100 * coverage
    stats['sigma_u'] = sigma_u
    stats['sigma_v'] = sigma_v
    stats['corr_uv'] = corr_uv

    # return some statistics
    return stats


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="Plot summary of hit estimator")
    parser.add_argument('--resultdir', default="results", type=str, help='Put all plots in this directory')
    parser.add_argument('--dbname', default="localdb", type=str, help='Name of local database')
    args = parser.parse_args()

    rfile1 = ROOT.TFile(args.dbname + '/dbstore_PXDClusterPositionEstimatorPar_rev_1.root', "OPEN")
    estimator = rfile1.Get("PXDClusterPositionEstimatorPar")

    rfile2 = ROOT.TFile(args.dbname + '/dbstore_PXDClusterShapeIndexPar_rev_1.root', "OPEN")
    indexer = rfile2.Get("PXDClusterShapeIndexPar")

    # Plot positions + errors hit estimator
    plot_estimator(estimator, indexer, args.resultdir)
