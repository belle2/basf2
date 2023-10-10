#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Plot PXD position estimator payload
"""

import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

import basf2 as b2
from ROOT import Belle2


class PlotClusterPositionEstimatorPayload(b2.Module):
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
            b2.B2INFO("PXDClusterPositionEstimator payload has changed. Plot the new payload.")
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
            ax.set_title(f'Number of corrections kind={pixelkind:d}', size=20)
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
            fig.savefig(self.resultdir + f'/Corrections_Heatmap_kind_{pixelkind:d}.png', dpi=100)
            fig.clf()
            plt.close(fig)

            pivot_table = df.pivot(index='thetaU', columns='thetaV', values='shapes')
            fig = plt.figure(figsize=(12, 12))
            ax = fig.add_subplot(111)
            ax.set_xlabel('thetaU / degree', size=20)
            ax.set_ylabel('thetaV / degree', size=20)
            ax.set_title(f'Number of shapes kind={pixelkind:d}', size=20)
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
            fig.savefig(self.resultdir + f'/Shapes_Heatmap_kind_{pixelkind:d}.png', dpi=100)
            fig.clf()
            plt.close(fig)

            pivot_table = df.pivot(index='thetaU', columns='thetaV', values='coverage')
            fig = plt.figure(figsize=(12, 12))
            ax = fig.add_subplot(111)
            ax.set_xlabel('thetaU / degree', size=20)
            ax.set_ylabel('thetaV / degree', size=20)
            ax.set_title(f'Coverage kind={pixelkind:d}', size=20)
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
            fig.savefig(self.resultdir + f'/Coverage_Heatmap_kind_{pixelkind:d}.png', dpi=100)
            fig.clf()
            plt.close(fig)

            pivot_table = df.pivot(index='thetaU', columns='thetaV', values='sigma_u')
            fig = plt.figure(figsize=(12, 12))
            ax = fig.add_subplot(111)
            ax.set_xlabel('thetaU / degree', size=20)
            ax.set_ylabel('thetaV / degree', size=20)
            ax.set_title(f'Average cluster sigma u kind={pixelkind:d}', size=20)
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
            fig.savefig(self.resultdir + f'/SigmaU_Heatmap_kind_{pixelkind:d}.png', dpi=100)
            fig.clf()
            plt.close(fig)

            pivot_table = df.pivot(index='thetaU', columns='thetaV', values='sigma_v')
            fig = plt.figure(figsize=(12, 12))
            ax = fig.add_subplot(111)
            ax.set_xlabel('thetaU / degree', size=20)
            ax.set_ylabel('thetaV / degree', size=20)
            ax.set_title(f'Average cluster sigma v kind={pixelkind:d}', size=20)
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
            fig.savefig(self.resultdir + f'/SigmaV_Heatmap_kind_{pixelkind:d}.png', dpi=100)
            fig.clf()
            plt.close(fig)

            pivot_table = df.pivot(index='thetaU', columns='thetaV', values='corr_uv')
            fig = plt.figure(figsize=(12, 12))
            ax = fig.add_subplot(111)
            ax.set_xlabel('thetaU / degree', size=20)
            ax.set_ylabel('thetaV / degree', size=20)
            ax.set_title(f'Average uv correlation kind={pixelkind:d}', size=20)
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
            fig.savefig(self.resultdir + f'/CorrelationUV_Heatmap_kind_{pixelkind:d}.png', dpi=100)
            fig.clf()
            plt.close(fig)

    def get_classifier_stats(self, shape_classifier, pixelkind):
        """Compute some statistics for a shape classifier"""
        # Read corrections data
        offsetMap = shape_classifier.getOffsetMap()
        likelyhoodMap = shape_classifier.getLikelyhoodMap()

        # Some counters
        nCorrections = 0
        nShapes = 0
        coverage = 0.0

        for item in offsetMap:
            shape_index = item.first
            offsets_array = item.second

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
    main = b2.create_path()
    main.add_module("EventInfoSetter", evtNumList=[1])
    main.add_module(PlotClusterPositionEstimatorPayload(args.resultdir))
    main.add_module("Progress")

    b2.process(main)
    print(b2.statistics)
