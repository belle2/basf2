#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <contact>tracking@belle2.kek.jp</contact>
  <output>FullTrackingValidation.root</output>
  <description>This module validates that full track finding and fitting with particle gun muon tracks of varying pt.</description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'FullTrackingValidation.root'
N_EVENTS = 2000
ACTIVE = True

import basf2
# necessary for the logging to work properly
from basf2 import *
basf2.set_random_seed(1337)

import argparse

from tracking.validation.run import TrackingValidationRun
from tracking.validation.module import TrackingValidationModule

import ROOT
import itertools


# computes bins to have one bin containing
# exactly one of the discrete values passed

def computeBins(discreteValues):
    if len(discreteValues) < 2:
        B2FATAL('discreteVaules list must at least contain two items')
    discreteValues.sort()
    # add leading item which has the same distance as entry 0 to 1
    discreteValues = [2 * discreteValues[0] - discreteValues[1]] \
        + discreteValues
    # add trailing item which has the same distance as entry n-1 to n
    discreteValues = discreteValues + [2 * discreteValues[-1]
                                       - discreteValues[-2]]

    bin_borders = []
    # loop over the discrete entries and compute the bin borders
    # exactly between each entry
    for i in range(0, len(discreteValues) - 2):
        avg_low = (discreteValues[i] + discreteValues[i + 1]) / 2.
        avg_high = (discreteValues[i + 1] + discreteValues[i + 2]) / 2.
        bin_borders = bin_borders + [(avg_low, avg_high, discreteValues[i
                                     + 1])]

    return bin_borders


class FilterMcParticleInPtBin(object):

    def __init__(self, ptBin):
        self.ptBin = ptBin

    def doesPrPass(self, filterProperties):
        return self.doesPass(filterProperties)

    def doesMcPass(self, filterProperties):
        return self.doesPass(filterProperties)

    # due to the specific ParticleGun setup of this validation run,
    # all tracks generated in one event have the exact same Pt
    # therefore, a cut on the first entry can be applied
    def doesPass(self, filterProperties):
        # if len(mcTrackCands) > 0:
        pt = filterProperties.mcParticles[0].getMomentum().Pt()
        # print " bin " + str( self.ptBin) + " -- PT " + str(pt)
        doesPass = pt > self.ptBin[0] and pt < self.ptBin[1]
        # print "does Pass " + str(doesPass)
        return doesPass


        # else:
        #    print "not sufficient mc particles to determine bin"

# Specialized version of the tracking validation
# which creates seperate root files with dedicated plots
# for pt bins

class FullTracking(TrackingValidationRun):

    n_events = N_EVENTS
    finder_module = 'StandardReco'
    pulls = True
    output_file_name = VALIDATION_OUTPUT_FILE

    def __init__(self, ptBinBorders):
        super(FullTracking, self).__init__()
        self.ptBinBorders = ptBinBorders

    def preparePathValidation(self, main_path):

        output_file_name_noExt = os.path.splitext(self.output_file_name)[0]
        ext = os.path.splitext(self.output_file_name)[1]

        # common exclude list for this test scenario
        exclude_mc = ['d_0', 'multiplicity']

        # add plotting over the whole pt spectrum
        fit = bool(self.fit_geometry)
        trackingValidationModule = TrackingValidationModule(
            self.name,
            contact=self.contact,
            fit=fit,
            pulls=self.pulls,
            exclude_profile_mc_parameter=exclude_mc,
            output_file_name=self.output_file_name,
            )
        main_path.add_module(trackingValidationModule)

        for ptBin in self.ptBinBorders:
            # create a special file name for this ptbin, using the discrete pt value configured
            bin_file_name = output_file_name_noExt + str(ptBin[2]) + ext
            postfix = 'PtBin_' + str(ptBin[2])
            title_postfix = ' Pt = ' + str(ptBin[2]) + ' GeV'
            passObj = FilterMcParticleInPtBin(ptBin)
            trackingValidationModule = TrackingValidationModule(
                self.name,
                contact=self.contact,
                fit=fit,
                pulls=self.pulls,
                output_file_name=bin_file_name,
                track_filter_object=passObj,
                plot_name_postfix=postfix,
                plot_title_postfix=title_postfix,
                exclude_profile_mc_parameter=exclude_mc + ['p_t'],
                use_expert_folder=False,
                )
            main_path.add_module(trackingValidationModule)


def getPtBinsWithWeight(pt_values):
    # each entry gets the same weight
    pt_weights = [1. / len(pt_values)] * len(pt_values)
    # weights are in the second half of the array going to ParticleGun
    return pt_values + pt_weights


def main():
    argument_parser = argparse.ArgumentParser()

    argument_parser.add_argument('-s', '--show', action='store_true',
                                 help='Show generated plots in a TBrowser immediatly.'
                                 )

    arguments = argument_parser.parse_args()

    # Configure particle gun to produce with a discrete list of pt values #
    #######################################################################

    # the discrete pt values in GeV for which to generate particles using the
    # particle gun
    ptDiscrete = [
        0.1,
        0.25,
        0.4,
        0.6,
        1.,
        1.5,
        2.,
        3.,
        4.,
        ]
    ptBinBorders = computeBins(ptDiscrete)
    validation_run = FullTracking(ptBinBorders=ptBinBorders)

    pt_bins_weight = getPtBinsWithWeight(ptDiscrete)
    validation_run.generator_module = basf2.register_module('ParticleGun')
    validation_run.generator_module.param({
        'pdgCodes': [13, -13],
        'nTracks': 10,
        'varyNTracks': 0,
        'momentumGeneration': 'discretePt',
        'momentumParams': pt_bins_weight,
        'vertexGeneration': 'fixed',
        'xVertexParams': [0.0],
        'yVertexParams': [0.0],
        'zVertexParams': [0.0],
        'thetaGeneration': 'uniformCos',
        })

    # Execute the validation run #
    ##############################
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    if ACTIVE:
        main()
