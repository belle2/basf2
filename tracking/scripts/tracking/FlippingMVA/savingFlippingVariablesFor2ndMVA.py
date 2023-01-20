##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import ROOT
from ROOT import Belle2
import math
import numpy as np
import tracking.harvest.harvesting as harvesting
import tracking.harvest.refiners as refiners
ROOT.gSystem.Load("libtracking")


class Saving2ndMVAData(harvesting.HarvestingModule):
    """ A dedicated module to save the variables using in flipping steps"""

    def __init__(self, name, contact=None, checkObj='RecoTracks', output_file_name='flip-refit-MVA2.root'):
        """Constructor"""
        super().__init__(foreach=checkObj, name=name, contact=contact, output_file_name=output_file_name)

        #: Name of the Obj to be picked
        self.checkObj = checkObj

        #: Name of  the StoreArray of the mc tracks
        self.mcRecoTracks = "MCRecoTracks"

        #: Reference to the track match lookup object reading the relation information
        self.track_match_look_up = None

        #: Name of the output file
        self.outputname = output_file_name

    def initialize(self):
        """Initialization at the start of the event processing"""
        super().initialize()
        self.track_match_look_up = Belle2.TrackMatchLookUp(self.mcRecoTracks, self.checkObj)
        output_tfile = ROOT.TFile(self.outputname, "RECREATE")
        self.outputname = output_tfile

    def prepare(self):
        """preparation at the start of each event.
           make sure the checkObj exist
        """
        super().prepare()
        checkDatas = Belle2.PyStoreArray(self.checkObj)
        if (not checkDatas):
            return False

    def pick(self, recoTrack):
        """pick every recoTrack"""
        return True

    def peel(self, recoTrack):
        """store the information for each recoTrack"""
        track_match_look_up = self.track_match_look_up
        nan = float('nan')
        flipped_pz_estimate = nan
        y_variance = nan
        tan_lambda_estimate = nan
        d0_variance = nan
        x_variance = nan
        z_estimate = nan
        phi0_variance = nan
        px_variance = nan
        pz_estimate = nan
        p_value = nan
        pt_estimate = nan
        y_estimate = nan
        d0_estimate = nan
        x_estimate = nan
        py_variance = nan
        pz_variance = nan
        omega_variance = nan
        tan_lambda_variance = nan
        z_variance = nan
        omega_estimate = nan
        pt_resolution = nan
        px_estimate = nan
        pt_variance = nan
        phi0_estimate = nan
        flipped_z_estimate = nan
        py_estimate = nan
        flipped_z_variance = nan
        flipped_pz_variance = nan
        flipped_pt_variance = nan
        flipped_py_estimate = nan
        z0_variance = nan
        flipped_p_value = nan
        flipped_px_variance = nan
        flipped_py_variance = nan
        flipped_x_estimate = nan
        quality_flip_indicator = nan
        quality_2ndflip_indicator = nan
        isPrimary_misID = False
        ismatched = False
        isprimary = False
        charge_truth = nan
        track_charge = nan
        inGoingArmTime = nan
        inGoingArmTimeError = nan
        outGoingArmTime = nan
        outGoingArmTimeError = nan
        InOutArmTimeDifference = nan
        InOutArmTimeDifferenceError = nan

        if (recoTrack):
            mc_particle = track_match_look_up.getRelatedMCParticle(recoTrack)
            fit_result = track_match_look_up.getRelatedTrackFitResult(recoTrack)

            inGoingArmTime = recoTrack.getIngoingArmTime()
            inGoingArmTimeError = recoTrack.getIngoingArmTimeError()
            outGoingArmTime = recoTrack.getOutgoingArmTime()
            outGoingArmTimeError = recoTrack.getOutgoingArmTimeError()
            InOutArmTimeDifference = recoTrack.getInOutArmTimeDifference()
            InOutArmTimeDifferenceError = recoTrack.getInOutArmTimeDifferenceError()

            ismatched = track_match_look_up.isMatchedPRRecoTrack(recoTrack)
            if mc_particle and fit_result:
                isprimary = bool(mc_particle.hasStatus(Belle2.MCParticle.c_PrimaryParticle))
                charge_truth = mc_particle.getCharge()
                if isprimary:
                    track_charge = fit_result.getChargeSign()
                    if charge_truth != track_charge:
                        isPrimary_misID = True

            recoTrack_flipped = recoTrack.getRelated("RecoTracks_flipped")
            if recoTrack_flipped:
                track_flipped = recoTrack_flipped.getRelated("Tracks_flipped")
                if track_flipped:
                    fit_result_flipped = track_flipped.getTrackFitResultWithClosestMassByName(
                        Belle2.Const.pion, "TrackFitResults_flipped")
                    if (fit_result and fit_result_flipped):
                        cov6 = fit_result.getCovariance6()
                        mom = fit_result.getMomentum()
                        pos = fit_result.getPosition()

                        pt_estimate = mom.Rho()
                        pt_variance = np.divide(mom.X() ** 2 * cov6(3, 3) + mom.Y() ** 2 * cov6(4, 4) -
                                                2 * mom.X() * mom.Y() * cov6(3, 4), mom.Perp2())
                        pt_resolution = np.divide(pt_variance, pt_estimate)

                        tan_lambda_estimate = fit_result.getCotTheta()
                        omega_estimate = fit_result.getOmega()
                        phi0_estimate = fit_result.getPhi() % (2.0 * math.pi)
                        d0_estimate = fit_result.getD0()
                        d0_variance = fit_result.getCov()[0]
                        z0_variance = fit_result.getCov()[12]
                        phi0_variance = fit_result.getCov()[5]
                        omega_variance = fit_result.getCov()[9]
                        tan_lambda_variance = fit_result.getCov()[14]
                        x_estimate = pos.X()
                        y_estimate = pos.Y()
                        z_estimate = pos.Z()
                        x_variance = cov6(0, 0)
                        y_variance = cov6(1, 1)
                        z_variance = cov6(2, 2)
                        px_estimate = mom.X()
                        py_estimate = mom.Y()
                        pz_estimate = cov6(5, 5)
                        px_variance = cov6(3, 3)
                        py_variance = cov6(4, 4)
                        pz_variance = cov6(5, 5)
                        p_value = fit_result.getPValue()

                        cov6_flipped = fit_result_flipped.getCovariance6()
                        mom_flipped = fit_result_flipped.getMomentum()
                        pos_flipped = fit_result_flipped.getPosition()

                        flipped_pt_variance = np.divide(
                            mom_flipped.X() ** 2 * cov6_flipped(3, 3) +
                            mom_flipped.Y() ** 2 * cov6_flipped(4, 4) -
                            2 * mom_flipped.X() * mom_flipped.Y() * cov6_flipped(3, 4),
                            mom_flipped.Perp2())

                        flipped_z_estimate = pos_flipped.Z()
                        flipped_pz_estimate = mom_flipped.Z()
                        flipped_py_estimate = mom_flipped.Y()

                        flipped_x_estimate = pos_flipped.X()
                        flipped_z_variance = cov6_flipped(2, 2)
                        flipped_pz_variance = cov6_flipped(5, 5)
                        flipped_px_variance = cov6_flipped(3, 3)
                        flipped_py_variance = cov6_flipped(4, 4)
                        flipped_p_value = fit_result_flipped.getPValue()

                        quality_flip_indicator = recoTrack.getFlipQualityIndicator()
                        quality_2ndflip_indicator = recoTrack.get2ndFlipQualityIndicator()

        crops = dict(
            flipped_pz_estimate=flipped_pz_estimate,
            y_variance=y_variance,
            tan_lambda_estimate=tan_lambda_estimate,
            d0_variance=d0_variance,
            x_variance=x_variance,
            z_estimate=z_estimate,
            phi0_variance=phi0_variance,
            px_variance=px_variance,
            pz_estimate=pz_estimate,
            p_value=p_value,
            pt_estimate=pt_estimate,
            y_estimate=y_estimate,
            d0_estimate=d0_estimate,
            x_estimate=x_estimate,
            py_variance=py_variance,
            pz_variance=pz_variance,
            omega_variance=omega_variance,
            tan_lambda_variance=tan_lambda_variance,
            z_variance=z_variance,
            omega_estimate=omega_estimate,
            pt_resolution=pt_resolution,
            px_estimate=px_estimate,
            pt_variance=pt_variance,
            phi0_estimate=phi0_estimate,
            flipped_z_estimate=flipped_z_estimate,
            py_estimate=py_estimate,
            flipped_z_variance=flipped_z_variance,
            flipped_pz_variance=flipped_pz_variance,
            flipped_pt_variance=flipped_pt_variance,
            flipped_py_estimate=flipped_py_estimate,
            z0_variance=z0_variance,
            flipped_p_value=flipped_p_value,
            flipped_px_variance=flipped_px_variance,
            flipped_py_variance=flipped_py_variance,
            flipped_x_estimate=flipped_x_estimate,
            quality_flip_indicator=quality_flip_indicator,
            quality_2ndflip_indicator=quality_2ndflip_indicator,
            isPrimary_misID=isPrimary_misID,
            ismatched=ismatched,
            isprimary=isprimary,
            charge_truth=charge_truth,
            track_charge=track_charge,
            inGoingArmTime=inGoingArmTime,
            inGoingArmTimeError=inGoingArmTimeError,
            outGoingArmTime=outGoingArmTime,
            outGoingArmTimeError=outGoingArmTimeError,
            InOutArmTimeDifference=InOutArmTimeDifference,
            InOutArmTimeDifferenceError=InOutArmTimeDifferenceError,
            )
        return crops

    #: save a tree of all the collected variables
    save_tree = refiners.save_tree(name="data")
