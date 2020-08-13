#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Created on Wed Jan  31 20:54:00 2018
Plot PXD position estimator payload
@author: benni
"""

import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

from basf2 import *
from ROOT import Belle2
import ROOT


class PlotClusterPositionEstimatorPayload(Module):
    """Plot the PXDClusterPositionEstimator playload """

    def __init__(self, resultdir):
        """Initialize"""
        super().__init__()  # don't forget to call parent constructor
        #: Position estimator payload
        self.position_estimator = Belle2.PyDBObj('PXDClusterPositionEstimatorPar')
        #: Directory to put all plots
        self.resultdir = resultdir
        #: Counter for number of different payloads
        self.counter = 0

    def event(self):
        """Plot position payload in case it has changed"""

        # We plot the payload whenever it changes
        if self.position_estimator.hasChanged():
            B2INFO("PXDClusterPositionEstimator payload has changed. Plot the new payload.")
            self.plot()
            self.counter += 1

    def average_covariance(self, shape_classifier):
        """Compute the average covariance for a shape classifier"""
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

    def plot(self):
        """Plot position estimator payload"""
        for pair in self.position_estimator.getGridMap():
            pixelkind = pair.first
            grid = pair.second

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
            for uBin in range(1, grid.GetXaxis().GetNbins() + 1):
                for vBin in range(1, grid.GetYaxis().GetNbins() + 1):

                    # Shape classifier for angle bin
                    shape_classifier = self.position_estimator.getShapeClassifier(uBin, vBin, pixelkind)

                    # Bin is centered around angles
                    thetaU = grid.GetXaxis().GetBinCenter(uBin)
                    thetaV = grid.GetYaxis().GetBinCenter(vBin)

                    # Create  plots for classifier
                    stats = self.get_classifier_stats(shape_classifier, pixelkind)

                    # Fill summary dict
                    summary_dict['thetaU'].append(thetaU)
                    summary_dict['thetaV'].append(thetaV)
                    summary_dict['pixelkind'].append(pixelkind)
                    summary_dict['shapes'].append(stats['shapes'])
                    summary_dict['corrections'].append(stats['corrections'])
                    summary_dict['coverage'].append(stats['coverage'])
                    summary_dict['sigma_u'].append(stats['sigma_u'] * 10000)  # cm -> um
                    summary_dict['sigma_v'].append(stats['sigma_v'] * 10000)  # cm -> um
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
            fig.savefig(self.resultdir + '/Corrections_Heatmap_kind_{:d}.png'.format(pixelkind), dpi=100)
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
            fig.savefig(self.resultdir + '/Shapes_Heatmap_kind_{:d}.png'.format(pixelkind), dpi=100)
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
            fig.savefig(self.resultdir + '/Coverage_Heatmap_kind_{:d}.png'.format(pixelkind), dpi=100)
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
            fig.savefig(self.resultdir + '/SigmaU_Heatmap_kind_{:d}.png'.format(pixelkind), dpi=100)
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
            fig.savefig(self.resultdir + '/SigmaV_Heatmap_kind_{:d}.png'.format(pixelkind), dpi=100)
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
            fig.savefig(self.resultdir + '/CorrelationUV_Heatmap_kind_{:d}.png'.format(pixelkind), dpi=100)
            fig.clf()
            plt.close(fig)

    def get_classifier_stats(self, shape_classifier, pixelkind):
        """Compute some statistics for a shape classifier"""
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
            offsets_array = item.second
            shape_likelyhood = shapeLikelyhoodMap[shape_index]

            nShapes += 1

            for eta_index, offset in enumerate(offsets_array):
                coverage += likelyhoodMap[shape_index][eta_index]
                nCorrections += 1

        if nShapes > 0:
            # Average covariance of all hits in classifier
            cov = self.average_covariance(shape_classifier)
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

    import argparse
    import os
    import shutil
    parser = argparse.ArgumentParser(description="Plot summary of hit estimator")
    parser.add_argument('--resultdir', default="results", type=str, help='Put all plots in this directory')
    args = parser.parse_args()

    # Remove old stuff
    if os.path.isdir(os.getcwd() + '/' + args.resultdir):
        shutil.rmtree(os.getcwd() + '/' + args.resultdir)

    # Create nice clean folder for results
    os.mkdir(os.getcwd() + '/' + args.resultdir)

    # Now let's create a path to simulate our events.
    main = create_path()
    main.add_module("EventInfoSetter", evtNumList=[1])
    main.add_module(PlotClusterPositionEstimatorPayload(args.resultdir))
    main.add_module("Progress")

    process(main)
    print(statistics)
