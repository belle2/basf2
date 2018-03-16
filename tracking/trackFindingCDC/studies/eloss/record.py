#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Fix TLS bug
import ctypes
# ctypes.cdll.LoadLibrary('/space/ofrost/basf2/release/lib/Linux_x86_64/opt/libtracking_trackFindingCDC.so')

import basf2

import ROOT
from ROOT import Belle2  # make Belle2 namespace available
from ROOT import std
from ROOT.Belle2 import TrackFindingCDC as TFCDC

import os
import sys
import math
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


def getNaiveBetheEnergyLoss(p, pdg_code, s):
    eloss = s * 0.00008  # Miriam's / Sasha's value
    eloss = s * 1 * 10**-4  # Miriam's / Sasha's value
    return eLoss


def getBetheStoppingPower(p, pdg_code):
    mass = 0.139  # Hard coded pions
    energy = np.sqrt(mass * mass + p * p)

    gamma = energy / mass
    beta = p / energy

    gamma2 = gamma * gamma
    beta2 = beta * beta

    eMass = 0.511 * Belle2.Unit.MeV
    m_I = 7.66932e-08
    m_eDensity = 0.000515726

    Wmax = 2 * eMass * beta2 * gamma2 / (1 + 2 * gamma * eMass / mass)
    I2 = m_I * m_I

    K = 0.307075 * Belle2.Unit.MeV * Belle2.Unit.cm2
    dEdx = (
        K * m_eDensity / beta2 *
        (1.0 / 2.0 * math.log(2 * eMass * beta2 * gamma2 * Wmax / I2) - beta2)
    )

    m_I = 4.54e-8  # Miriam's / Sasha's value
    # I2 = 2.06116e-15
    I2 = m_I * m_I

    factor = 178.6 * 0.00015e-04 / 2  # Miriam's / Sasha's value - to big...
    eDensity = factor / K
    dEdx = factor / beta2 * (math.log(2 * eMass * beta2 * gamma2 * Wmax / I2) - beta2)

    # Miriam's / Sasha's original
    # SR = (0.00015e-04*(2*(beta)**2)**(-1))*(math.log((0.0000010404*((beta)**4)*((gamma)**4))/
    #                                                  (2.06116e-15*(1+0.007331*gamma)))-beta**2)
    # dEdx = 178.6 * SR
    return dEdx


def getBetheEnergyLoss(p, pdg_code, path):
    dEdx = getBetheStoppingPower(p, pdg_code)
    eloss = dEdx * path
    return eloss


def getMomentumLossFactor(p, pdg_code, eloss):
    p_factor = (p - eloss) / p
    return p_factor


def DeltaR(path, particleID, P):
    eloss = getBetheEnergyLoss(P, particleID, path)
    return getMomentumLossFactor(P, particleID, eloss)


class ElossHarvestingRun(HarvestingRun):
    n_events = 10000
    generator_module = "eloss_gun"
    detector_setup = "TrackingDetectorConstB"

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
        self.eloss_estimator = TFCDC.EnergyLossEstimator.forCDC()
        self.bfield = TFCDC.CDCBFieldUtil

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

        # rl_drift_circle = 1
        # unit_variance = 0
        # observations2D = TFCDC.CDCObservations2D(rl_drift_circle, unit_variance)

        # for recoHit3D in track:
        #    observations2D.append(recoHit3D)

        # trajectory2D = track_fitter.fit(observations2D)
        # trajectory2D.setLocalOrigin(TFCDC.Vector2D(0, 0))

        mc_particle = mc_track_lookup.getMCParticle(track)
        pdg_code = mc_particle.getPDG()
        t_vertex = mc_particle.getVertex()
        t_mom = mc_particle.getMomentum()
        charge = mc_particle.getCharge()
        mass = mc_particle.getMass()
        mc_energy = mc_particle.getEnergy()

        mc_vertex2D = TFCDC.Vector2D(t_vertex.XYvector())
        mc_mom2D = TFCDC.Vector2D(t_mom.XYvector())
        mc_trajectory2D = TFCDC.CDCTrajectory2D(mc_vertex2D, 0, mc_mom2D, charge)
        mc_pt = mc_mom2D.norm()

        first_hit = self.mc_track_lookup.getFirstHit(track)
        first_sim_hit = first_hit.getRelated("CDCSimHits")
        if first_sim_hit is None:
            return
        # Make sure we start the track in the first layer to avoid some confusion
        if first_sim_hit.getWireID().getICLayer() != 0:
            return

        last_hit = self.mc_track_lookup.getLastHit(track)
        last_sim_hit = last_hit.getRelated("CDCSimHits")
        if last_sim_hit is None:
            return
        # Make sure we start the track in the last layer to avoid some confusion
        # if last_sim_hit.getWireID().getICLayer() != 55: return
        last_sim_mom3D = TFCDC.Vector3D(last_sim_hit.getMomentum())

        first_sim_pos3D = TFCDC.Vector3D(first_sim_hit.getPosTrack())
        first_sim_mom3D = TFCDC.Vector3D(first_sim_hit.getMomentum())
        first_sim_tof = first_sim_hit.getFlightTime()
        first_sim_energy = np.sqrt(first_sim_mom3D.norm() ** 2 + mass ** 2)
        first_sim_pt = first_sim_mom3D.cylindricalR()
        first_sim_pz = first_sim_mom3D.z()

        first_sim_mom2D = first_sim_mom3D.xy()

        # first_sim_mom2D.normalizeTo(last_sim_mom3D.cylindricalR())
        first_sim_trajectory2D = TFCDC.CDCTrajectory2D(first_sim_pos3D.xy(), first_sim_tof, first_sim_mom2D, charge)

        # mc_trajectory3D = mc_track_lookup.getTrajectory3D(track)
        # mc_trajectory2D = mc_trajectory3D.getTrajectory2D()

        for reco_hit3D in track:
            sim_hit = self.mc_hit_lookup.getSimHit(reco_hit3D.getWireHit().getHit())
            if sim_hit is None:
                continue

            sim_mom3D = TFCDC.Vector3D(sim_hit.getMomentum())
            sim_energy = np.sqrt(sim_mom3D.norm() ** 2 + mass ** 2)
            sim_pt = sim_mom3D.cylindricalR()
            sim_pz = sim_mom3D.z()

            mc_eloss_truth = mc_energy - sim_energy
            first_eloss_truth = first_sim_energy - sim_energy

            sim_pos3D = TFCDC.Vector3D(sim_hit.getPosTrack())
            sim_pos2D = sim_pos3D.xy()

            layer_cid = reco_hit3D.getWire().getICLayer()
            bz = self.bfield.getBFieldZ(sim_pos3D)
            r = sim_pos3D.cylindricalR()

            # recoPos3D = reco_hit3D.getRecoPos3D()
            # recoPos3D = TFCDC.Vector3D(sim_hit.getPosTrack())
            # recoPos2D = recoPos3D.xy()
            # refPos2D = reco_hit3D.getRefPos2D()
            # l = reco_hit3D.getSignedRecoDriftLength()

            mc_s2D = mc_trajectory2D.calcArcLength2D(sim_pos2D)
            first_s2D = first_sim_trajectory2D.calcArcLength2D(sim_pos2D)

            if mc_s2D < 0:
                # mc_s2D += mc_trajectory2D.getArcLength2DPeriod()
                continue

            mc_eloss_estimate = self.eloss_estimator.getEnergyLoss(mc_pt, pdg_code, mc_s2D)
            first_eloss_estimate = self.eloss_estimator.getEnergyLoss(first_sim_pt, pdg_code, first_s2D)
            first_ploss_factor = self.eloss_estimator.getMomentumLossFactor(first_sim_pt, pdg_code, first_s2D)

            sasha_eloss_estimate = getBetheEnergyLoss(first_sim_pt, pdg_code, first_s2D)
            sasha_ploss_factor = DeltaR(first_s2D, pdg_code, first_sim_pt)

            # first_residual2D = first_trajectory2D.getDist2D(refPos2D) - l
            first_residual2D = first_sim_trajectory2D.getDist2D(sim_pos2D)
            first_disp2D = charge * first_residual2D

            first_loss_disp2D_estimate = abs(self.eloss_estimator.getLossDist2D(first_sim_pt, pdg_code, first_s2D))
            first_delossed_disp2D = first_disp2D - first_loss_disp2D_estimate

            # mc_residual2D = mc_trajectory2D.getDist2D(refPos2D) - l
            mc_residual2D = mc_trajectory2D.getDist2D(sim_pos2D)
            mc_disp2D = charge * mc_residual2D

            mc_loss_disp2D_estimate = abs(self.eloss_estimator.getLossDist2D(mc_pt, pdg_code, mc_s2D))
            mc_delossed_disp2D = mc_disp2D - mc_loss_disp2D_estimate

            # s2D = trajectory2D.calcArcLength2D(recoPos2D)
            # if s2D < 0: s2D += trajectory2D.getArcLength2DPeriod()

            # residual2D = trajectory2D.getDist2D(recoPos2D)
            # disp2D = charge * residual2D

            # # Original approach
            # center2D = trajectory2D.getGlobalCircle().center()
            # radius2D = recoPos2D - center2D;

            # eLoss = self.eloss_estimator.getEnergyLoss(mc_pt, pdg_code, mc_s2D)
            # dx = self.eloss_estimator.getMomentumLossFactor(mc_pt, pdg_code, mc_s2D)
            # deloss_radius2D = recoPos2D - center2D;
            # deloss_radius2D.scale (1.0 /  dx)
            # loss_disp2D_estimate2 = (deloss_radius2D - radius2D).norm()

            # loss_disp2D_estimate3 = radius2D.norm() * (1.0 /dx - 1.0)
            # loss_disp2D_estimate3 = radius2D.norm() * ((1.0 - dx) /dx)
            # loss_disp2D_estimate3 = radius2D.norm() * (eLoss / (mc_pt - eLoss))

            if abs(mc_residual2D) > 6:
                continue

            if abs(first_residual2D) > 6:
                continue

            yield dict(
                layer_cid=layer_cid,
                r=r,
                bz=bz,
                # pt=trajectory2D.getAbsMom2D(),
                pdg_code=abs(pdg_code),
                mass=mass,
                charge=charge,
                mc_pt=mc_pt,
                first_sim_pt=first_sim_pt,
                sim_pt=sim_pt,

                diff_pt=first_sim_pt - sim_pt,
                diff_pz=first_sim_pz - sim_pz,

                mc_eloss_truth=mc_eloss_truth,
                mc_eloss_estimate=mc_eloss_estimate,

                first_eloss_truth=first_eloss_truth,

                first_eloss_estimate=first_eloss_estimate,
                sasha_eloss_estimate=sasha_eloss_estimate,

                first_ploss_factor=first_ploss_factor,
                sasha_ploss_factor=sasha_ploss_factor,

                # l=l,

                first_s2D=first_s2D,
                first_residual2D=first_residual2D,
                first_disp2D=first_disp2D,
                first_loss_disp2D_estimate=first_loss_disp2D_estimate,
                first_delossed_disp2D=first_delossed_disp2D,

                mc_s2D=mc_s2D,
                mc_residual2D=mc_residual2D,
                mc_disp2D=mc_disp2D,
                mc_loss_disp2D_estimate=mc_loss_disp2D_estimate,
                mc_delossed_disp2D=mc_delossed_disp2D,

                # s2D=s2D,
                # disp2D=disp2D,
                # residual2D=residual2D,

                # loss_disp2D_estimate2=loss_disp2D_estimate2,
                # loss_disp2D_estimate3=loss_disp2D_estimate3,
            )

    # Refiners to be executed at the end of the harvesting / termination of the module
    save_tree = refiners.save_tree()
    save_histograms = refiners.save_histograms(
        outlier_z_score=5.0,
        allow_discrete=True,
    )

    save_histograms_stackby_charge = refiners.save_histograms(
        select=[
            # "mc_disp2D",
            "first_disp2D",
            "charge",
        ],
        outlier_z_score=5.0,
        allow_discrete=True,
        fit="gaus",
        fit_z_score=1,
        groupby="charge",
    )

    save_scatter = refiners.save_scatters(
        x=['mc_s2D'],
        y=[
            # 'mc_disp2D',
            'disp2D',
        ],
        groupby=[None, "charge"],
        filter=lambda x: x == 211,
        filter_on="pdg_code",
    )

    save_profiles = refiners.save_profiles(
        x=['mc_s2D'],
        y=[
            # 'mc_disp2D',
            'first_disp2D',
            'disp2D',
        ],
        groupby=[None, "charge"],
    )

    save_bz_profiles = refiners.save_profiles(
        x='r',
        y='bz',
    )

    # Loss displacement #
    # ################# #

    save_cid_histogram = refiners.save_histograms(
        select=[
            # 'mc_disp2D',
            # 'mc_delossed_disp2D',
            'first_disp2D',
            'first_delossed_disp2D',
            'bz',
            # 'disp2D',
        ],
        outlier_z_score=5.0,
        groupby=["layer_cid"],
        # stackby="pdg_code",
    )

    save_cid_profiles = refiners.save_profiles(
        x=["mc_pt"],
        y=[
            # 'mc_disp2D',
            # 'mc_delossed_disp2D',
            'first_disp2D',
            'first_delossed_disp2D',
        ],
        outlier_z_score=5.0,
        groupby=["layer_cid"],
        stackby="pdg_code",
    )

    save_cid_scatters = refiners.save_scatters(
        x=["mc_pt"],
        y=[
            # 'mc_disp2D',
            # 'mc_delossed_disp2D',
            'first_disp2D',
            'first_delossed_disp2D',
        ],
        outlier_z_score=5.0,
        groupby=["layer_cid"],
        stackby="pdg_code",
    )

    # Energy loss #
    # ########### #
    save_energy_cid_histogram = refiners.save_histograms(
        select=[
            'pdg_code',
            'first_eloss_estimate',
            'first_eloss_truth',
        ],
        outlier_z_score=5.0,
        groupby=["layer_cid"],
        stackby="pdg_code",
        folder_name='energy/{groupby_addition}',
    )

    save_energy_cid_profiles = refiners.save_profiles(
        x=["mc_pt"],
        y=[
            'first_eloss_truth',
            'first_eloss_estimate',
        ],
        outlier_z_score=5.0,
        groupby=["layer_cid"],
        stackby="pdg_code",
        folder_name='energy/{groupby_addition}',
    )

    save_energy_cid_scatters = refiners.save_scatters(
        x=["mc_pt"],
        y=[
            'first_eloss_truth',
            'first_eloss_estimate',
        ],
        outlier_z_score=5.0,
        groupby=["layer_cid"],
        stackby="pdg_code",
        folder_name='energy/{groupby_addition}',
    )


def main():
    run = ElossHarvestingRun()
    run.configure_and_execute_from_commandline()

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
