#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Fix TLS bug
import ctypes
ctypes.cdll.LoadLibrary('/space/ofrost/basf2/release/lib/Linux_x86_64/opt/libtracking_trackFindingCDC.so')

import basf2

import ROOT
from ROOT import Belle2  # make Belle2 namespace available
from ROOT import std
from ROOT.Belle2 import TrackFindingCDC as TFCDC

import os
import sys
import random
import numpy as np

from tracking.validation.utilities import is_primary

import tracking.harvest.harvesting as harvesting
import tracking.harvest.refiners as refiners
from tracking.harvest.run import HarvestingRun

import argparse

import logging


def get_logger():
    return logging.getLogger(__name__)

CONTACT = "oliver.frost@desy.de"


class ElossHarvestingRun(HarvestingRun):
    n_events = 40000
    generator_module = "eloss_gun"
    detector_setup = "TrackingDetector"

    @property
    def output_file_name(self):
        return 'eloss.root'

    def harvesting_module(self, path=None):
        harvesting_module = ElossHarvestingModule(self.output_file_name)
        if path:
            path.add_module(harvesting_module)
        return harvesting_module

    def create_argument_parser(self, **kwds):
        argument_parser = super().create_argument_parser(**kwds)
        return argument_parser

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
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


class ElossHarvestingModule(harvesting.HarvestingModule):

    """Module to collect information about the generated segments and
    compose validation plots on terminate."""

    def __init__(self, output_file_name):
        super().__init__(foreach='CDCTrackVector',
                         output_file_name=output_file_name)

        self.mc_track_lookup = None

        origin_track_fitter = TFCDC.CDCRiemannFitter()
        origin_track_fitter.setOriginConstrained()
        self.track_fitter = origin_track_fitter

    def initialize(self):
        super().initialize()
        self.mc_track_lookup = TFCDC.CDCMCTrackLookUp.getInstance()
        self.mc_hit_lookup = TFCDC.CDCMCHitLookUp.getInstance()

    def prepare(self):
        TFCDC.CDCMCHitLookUp.getInstance().fill()

    def pick(self, track):
        if track.size() < 4:
            return False

        mc_track_lookup = self.mc_track_lookup
        mc_particle = mc_track_lookup.getMCParticle(track)

        # Check that mc_particle is not a nullptr
        if mc_particle is None:
            return False

        if mc_particle.getMomentum().Perp() < 0.250:
            return False

        return is_primary(mc_particle)

    def peel(self, track):
        mc_track_lookup = self.mc_track_lookup
        mc_hit_lookup = self.mc_hit_lookup

        track_fitter = self.track_fitter

        rl_drift_circle = 1
        unit_variance = 0
        observations2D = TFCDC.CDCObservations2D(rl_drift_circle, unit_variance)

        for recoHit3D in track:
            observations2D.append(recoHit3D)

        trajectory2D = track_fitter.fit(observations2D)
        trajectory2D.setLocalOrigin(TFCDC.Vector2D(0, 0))

        mc_particle = mc_track_lookup.getMCParticle(track)
        pdg_code = abs(mc_particle.getPDG())
        t_vertex = mc_particle.getVertex()
        t_mom = mc_particle.getMomentum()
        charge = mc_particle.getCharge()

        vertex2D = TFCDC.Vector2D(t_vertex.XYvector())
        mom2D = TFCDC.Vector2D(t_mom.XYvector())

        mc_trajectory2D = TFCDC.CDCTrajectory2D(vertex2D, 0, mom2D, charge)

        l = recoHit3D.getSignedRecoDriftLength()

        for recoHit3D in track:
            layer_cid = recoHit3D.getWire().getICLayer()

            recoPos3D = recoHit3D.getRecoPos3D()
            r = recoHit3D.getRefPos2D().cylindricalR()

            mc_s2D = mc_trajectory2D.calcArcLength2D(recoPos3D.xy())
            if mc_s2D < 0:
                continue
                mc_s2D += mc_trajectory2D.getArcLength2DPeriod()

            mc_residual2D = mc_trajectory2D.getDist2D(recoPos3D.xy())
            mc_loss_disp2D = charge * mc_residual2D

            s2D = trajectory2D.calcArcLength2D(recoPos3D.xy())
            residual2D = trajectory2D.getDist2D(recoPos3D.xy())
            loss_disp2D = charge * residual2D

            if abs(mc_residual2D) > 6:
                continue

            yield dict(
                pt=trajectory2D.getAbsMom2D(),
                mc_pt=t_mom.Perp(),
                pdg_code=pdg_code,
                charge=charge,

                layer_cid=layer_cid,

                l=l,

                mc_s2D=mc_s2D,
                mc_loss_disp2D=mc_loss_disp2D,
                mc_residual2D=mc_residual2D,

                s2D=s2D,
                loss_disp2D=loss_disp2D,
                residual2D=residual2D,
            )

    # Refiners to be executed at the end of the harvesting / termination of the module
    save_tree = refiners.save_tree()
    save_histograms = refiners.save_histograms(
        outlier_z_score=5.0,
        allow_discrete=True,
    )

    save_histograms_stackby_charge = refiners.save_histograms(
        select=["residual2D", "loss_disp2D", "mc_loss_disp2D", "charge"],
        outlier_z_score=5.0,
        allow_discrete=True,
        fit="gaus",
        fit_z_score=1,
        groupby="charge",
    )

    save_scatter = refiners.save_scatters(
        x=['mc_s2D'],
        y=['mc_loss_disp2D', 'loss_disp2D'],
        groupby=[None, "charge"],
        filter=lambda x: x == 211,
        filter_on="pdg_code",
    )

    save_profiles = refiners.save_profiles(
        x=['mc_s2D'],
        y=['mc_loss_disp2D', 'loss_disp2D'],
        groupby=[None, "charge"],
    )

    save_cid_histogram = refiners.save_histograms(
        select=[
            'mc_loss_disp2D',
            'loss_disp2D',
        ],
        outlier_z_score=5.0,
        groupby=["layer_cid"],
    )

    save_cid_profiles = refiners.save_profiles(
        x=["mc_pt"],
        y=['mc_loss_disp2D'],
        outlier_z_score=5.0,
        groupby=["layer_cid"],
        stackby="pdg_code",
    )


def main():
    run = ElossHarvestingRun()
    run.configure_and_execute_from_commandline()

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
