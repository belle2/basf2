#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


from ROOT import Belle2  # make Belle2 namespace available # noqa
from ROOT.Belle2 import TrackFindingCDC as TFCDC

import sys
import random
import numpy as np

from tracking.validation.utilities import is_primary

import tracking.harvest.harvesting as harvesting
import tracking.harvest.refiners as refiners
from tracking.harvest.run import HarvestingRun


import logging


def get_logger():
    return logging.getLogger(__name__)


CONTACT = "oliver.frost@desy.de"


class LegendreBinningValidationRun(HarvestingRun):
    """Harvester to generate, postprocess and inspect MC events for track-segment evaluation"""
    #: number of events to generate
    n_events = 10000
    #: use the generic event generator
    generator_module = "generic"

    @property
    def output_file_name(self):
        """Get the output ROOT filename"""
        return 'legendre_binning.root'

    def harvesting_module(self, path=None):
        """Harvest and post-process the generated events"""
        harvesting_module = LegendreBinningValidationModule(self.output_file_name)
        if path:
            path.add_module(harvesting_module)
        return harvesting_module

    def create_argument_parser(self, **kwds):
        """Convert command-line arguments to basf2 argument list"""
        argument_parser = super().create_argument_parser(**kwds)
        return argument_parser

    def create_path(self):
        """
        Sets up a path that plays back pregenerated events or generates events
        based on the properties in the base class.
        """
        path = super().create_path()

        path.add_module("TFCDC_WireHitPreparer",
                        logLevel=8,
                        flightTimeEstimation="outwards",
                        UseNLoops=1)

        path.add_module('TFCDC_AxialTrackCreatorMCTruth',
                        logLevel=8,
                        useOnlyBeforeTOP=True,
                        fit=True,
                        reconstructedDriftLength=True,
                        reconstructedPositions=True)

        return path


class LegendreBinningValidationModule(harvesting.HarvestingModule):

    """Module to collect information about the generated segments and
    compose validation plots on terminate."""

    def __init__(self, output_file_name):
        """Constructor"""
        super().__init__(foreach='CDCTrackVector',
                         output_file_name=output_file_name)

        #: by default, there is no method to find matching MC tracks
        self.mc_track_lookup = None

        origin_track_fitter = TFCDC.CDCRiemannFitter()
        origin_track_fitter.setOriginConstrained()
        #: Use the CDCReimannFitter with a constrained origin for track fitting
        self.track_fitter = origin_track_fitter

        curv_bounds = []
        with open('fine_curv_bounds.txt') as curv_bounds_file:
            for curv_bound_line in curv_bounds_file:
                curv_bounds.append(float(curv_bound_line))

        bin_bounds = list(zip(curv_bounds[0::2], curv_bounds[1::2]))
        bin_bounds = sorted(bin_bounds)

        #: cached copy of lower bounds
        self.lower_curv_bounds = np.array([bin[0] for bin in bin_bounds])
        #: cached copy of upper bounds
        self.upper_curv_bounds = np.array([bin[1] for bin in bin_bounds])

        assert(len(self.lower_curv_bounds) == len(self.upper_curv_bounds))

    def initialize(self):
        """Receive signal at the start of event processing"""
        super().initialize()
        #: Method to find matching MC tracks
        self.mc_track_lookup = TFCDC.CDCMCTrackLookUp.getInstance()
        #: Method to find matching MC hits
        self.mc_hit_lookup = TFCDC.CDCMCHitLookUp.getInstance()

    def prepare(self):
        """Initialize the MC-hit lookup method"""
        TFCDC.CDCMCHitLookUp.getInstance().fill()

    def pick(self, track):
        """Select tracks with at least 4 segments and associated primary MC particle"""
        mc_track_lookup = self.mc_track_lookup
        mc_particle = mc_track_lookup.getMCParticle(track)

        # Check that mc_particle is not a nullptr
        return mc_particle and is_primary(mc_particle) and track.size() > 3

    def peel(self, track):
        """Aggregate the track and MC information for track-segment analysis"""
        track_fitter = self.track_fitter

        rl_drift_circle = 1
        unit_variance = 0
        observations2D = TFCDC.CDCObservations2D(rl_drift_circle, unit_variance)

        for recoHit3D in track:
            observations2D.append(recoHit3D)

        trajectory2D = track_fitter.fit(observations2D)
        trajectory2D.setLocalOrigin(TFCDC.Vector2D(0, 0))

        n_hits = track.size()
        pt = trajectory2D.getAbsMom2D()
        curv = trajectory2D.getCurvature()
        curl_curv = abs(self.lower_curv_bounds[0])
        bin_curv = curv if abs(curv) < curl_curv else abs(curv)
        curv_var = trajectory2D.getLocalVariance(0)
        impact = trajectory2D.getGlobalImpact()
        phi0 = trajectory2D.getLocalCircle().phi0()

        circle = trajectory2D.getLocalCircle()
        n12 = circle.n12()

        cross_curvs = []
        for recoHit3D in track:
            wire_ref_pos = recoHit3D.getRefPos2D()
            drift_length = recoHit3D.getSignedRecoDriftLength()
            r = wire_ref_pos.norm()
            cross_curv = -2 * (n12.dot(wire_ref_pos) - drift_length) / (r * r - drift_length * drift_length)
            cross_curvs.append(cross_curv)

        cross_curvs = np.array(cross_curvs)
        cross_curv = np.median(cross_curvs)
        cross_curv_var = np.median(np.abs(cross_curvs - cross_curv))

        basic_curv_precision = TFCDC.PrecisionUtil.getBasicCurvPrecision(cross_curv)
        origin_curv_precision = TFCDC.PrecisionUtil.getOriginCurvPrecision(cross_curv)
        non_origin_curv_precision = TFCDC.PrecisionUtil.getNonOriginCurvPrecision(cross_curv)

        bin_id = np.digitize([abs(cross_curv)], self.lower_curv_bounds)
        if bin_id == 0:
            max_curv_precision = 0.00007
        else:
            max_curv_precision = self.upper_curv_bounds[bin_id - 1] - self.lower_curv_bounds[bin_id - 1]

        random_bin_id = random.randrange(len(self.upper_curv_bounds))
        random_lower_curv_bound = self.lower_curv_bounds[random_bin_id]
        random_upper_curv_bound = self.upper_curv_bounds[random_bin_id]
        curv_dense = random.uniform(random_lower_curv_bound, random_upper_curv_bound)
        curv_width = random_upper_curv_bound - random_lower_curv_bound

        return dict(
            n_hits=n_hits,
            curvature_estimate=curv,
            curvature_variance=curv_var,
            abs_curvature_estimate=abs(curv),
            inv_curv=1.0 / abs(curv),
            cross_curv=cross_curv,
            cross_curv_var=cross_curv_var,
            basic_curv_precision=basic_curv_precision,
            origin_curv_precision=origin_curv_precision,
            non_origin_curv_precision=non_origin_curv_precision,
            max_curv_precision=max_curv_precision,
            pt=pt,
            curv_bin=bin_curv,
            curv_dense=curv_dense,
            curv_width=curv_width,
            impact_estimate=impact,
            phi0=phi0,
        )

    # Refiners to be executed at the end of the harvesting / termination of the module
    #: Save a tree of all collected variables in a sub folder
    save_tree = refiners.save_tree()
    #: Save histograms in a sub folder
    save_histograms = refiners.save_histograms(outlier_z_score=5.0, allow_discrete=True)

    #: Save profile histograms in a sub folder
    save_profiles = refiners.save_profiles(x=['curvature_estimate', 'phi0'],
                                           y='curvature_variance',
                                           outlier_z_score=5.0)

    #: Save cross-curvature profile histograms in a sub folder
    save_cross_curv_profile = refiners.save_profiles(x=['cross_curv'],
                                                     y=['cross_curv_var',
                                                        'curvature_estimate',
                                                        'basic_curv_precision',
                                                        'origin_curv_precision',
                                                        'non_origin_curv_precision',
                                                        'max_curv_precision',
                                                        ],
                                                     outlier_z_score=5.0)

    #: Save scatterplots in a sub folder
    save_scatter = refiners.save_scatters(x=['curvature_estimate'], y='n_hits')


def main():
    run = LegendreBinningValidationRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
