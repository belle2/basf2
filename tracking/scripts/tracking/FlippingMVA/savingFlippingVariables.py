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
import tracking.harvest.harvesting as harvesting
import tracking.harvest.refiners as refiners
import tracking.harvest.peelers as peelers
from tracking.validation.utilities import getObjectList
ROOT.gSystem.Load("libtracking")


class Saving1stMVAData(harvesting.HarvestingModule):
    """ A dedicated module to save the variables using in flipping steps"""

    def __init__(self, name, contact=None, checkObj='RecoTracks', output_file_name='flip-refit-MVA1.root'):
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
        d0_variance = nan
        seed_pz_estimate = nan
        n_hits = nan
        z0_estimate = nan
        seed_pz_variance = nan
        phi0_variance = nan
        seed_z_estimate = nan
        tan_lambda_estimate = nan
        omega_variance = nan
        seed_tan_lambda_estimate = nan
        d0_estimate = nan
        seed_pt_estimate = nan
        cdc_qualityindicator = nan
        omega_estimate = nan
        z0_variance = nan
        seed_x_estimate = nan
        seed_y_estimate = nan
        seed_pt_resolution = nan
        seed_py_variance = nan
        seed_d0_estimate = nan
        seed_omega_variance = nan
        tan_lambda_variance = nan
        svd_layer6_clsTime = nan
        seed_tan_lambda_variance = nan
        seed_z_variance = nan
        n_svd_hits = nan
        phi0_estimate = nan
        n_cdc_hits = nan
        svd_layer3_positionSigma = nan
        first_cdc_layer = nan
        last_cdc_layer = nan
        ndf_hits = nan
        isPrimary_misID = False
        ismatched = False
        isprimary = False
        charge_truth = nan
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
                    if mc_particle.getCharge() != track_charge:
                        isPrimary_misID = True

            cdc_track_cand = recoTrack.getRelated('CDCRecoTracks')
            if cdc_track_cand:
                cdc_qualityindicator = cdc_track_cand.getQualityIndicator()

            if fit_result:
                omega_estimate = fit_result.getOmega()
                z0_estimate = fit_result.getZ0()
                d0_estimate = fit_result.getD0()
                phi0_estimate = fit_result.getPhi() % (2.0 * math.pi)
                tan_lambda_estimate = fit_result.getCotTheta()

                d0_variance = fit_result.getCov()[0]
                z0_variance = fit_result.getCov()[12]
                phi0_variance = fit_result.getCov()[5]
                omega_variance = fit_result.getCov()[9]

                reco_svdcdc_track = recoTrack.getRelated("SVDCDCRecoTracks")

                seed_fit_result = peelers.get_seed_track_fit_result(reco_svdcdc_track)
                seed_mom = seed_fit_result.getMomentum()
                seed_pos = seed_fit_result.getPosition()
                seed_cov6 = seed_fit_result.getCovariance6()
                seed_tan_lambda_estimate = seed_fit_result.getCotTheta()

                seed_pz_estimate = seed_mom.Z()
                seed_pz_variance = seed_cov6(5, 5)
                seed_z_estimate = seed_pos.Z()
                seed_x_estimate = seed_pos.X()
                seed_y_estimate = seed_pos.Y()

                seed_pt_estimate = seed_mom.Rho()
                seed_py_variance = seed_cov6(4, 4)
                seed_d0_estimate = seed_fit_result.getD0()
                seed_omega_variance = seed_fit_result.getCov()[9]
                seed_tan_lambda_variance = seed_fit_result.getCov()[14]
                seed_z_variance = seed_cov6(2, 2)

                tan_lambda_variance = seed_fit_result.getCov()[14]
                for svd_hit in getObjectList(recoTrack.getSVDHitList()):
                    if svd_hit.getSensorID().getLayerNumber() == 3:
                        svd_layer3_positionSigma = svd_hit.getPositionSigma()
                    if svd_hit.getSensorID().getLayerNumber() == 6:
                        svd_layer6_clsTime = svd_hit.getClsTime()

                cdc_hits = [hit.getICLayer() for hit in getObjectList(recoTrack.getCDCHitList())]
                if cdc_hits:
                    first_cdc_layer = min(cdc_hits)
                    last_cdc_layer = max(cdc_hits)

                n_cdc_hits = recoTrack.getNumberOfCDCHits()
                n_svd_hits = recoTrack.getNumberOfSVDHits()
                n_pxd_hits = recoTrack.getNumberOfPXDHits()

                n_hits = n_pxd_hits + n_svd_hits + n_cdc_hits
                ndf_hits = 2 * n_pxd_hits + n_svd_hits + n_cdc_hits

        crops = dict(
            d0_variance=d0_variance,
            seed_pz_estimate=seed_pz_estimate,
            n_hits=n_hits,
            z0_estimate=z0_estimate,
            seed_pz_variance=seed_pz_variance,
            phi0_variance=phi0_variance,
            seed_z_estimate=seed_z_estimate,
            tan_lambda_estimate=tan_lambda_estimate,
            omega_variance=omega_variance,
            seed_tan_lambda_estimate=seed_tan_lambda_estimate,
            d0_estimate=d0_estimate,
            seed_pt_estimate=seed_pt_estimate,
            cdc_qualityindicator=cdc_qualityindicator,
            omega_estimate=omega_estimate,
            z0_variance=z0_variance,
            seed_x_estimate=seed_x_estimate,
            seed_y_estimate=seed_y_estimate,
            seed_pt_resolution=seed_pt_resolution,
            seed_py_variance=seed_py_variance,
            seed_d0_estimate=seed_d0_estimate,
            seed_omega_variance=seed_omega_variance,
            tan_lambda_variance=tan_lambda_variance,
            svd_layer6_clsTime=svd_layer6_clsTime,
            seed_tan_lambda_variance=seed_tan_lambda_variance,
            seed_z_variance=seed_z_variance,
            n_svd_hits=n_svd_hits,
            phi0_estimate=phi0_estimate,
            n_cdc_hits=n_cdc_hits,
            svd_layer3_positionSigma=svd_layer3_positionSigma,
            first_cdc_layer=first_cdc_layer,
            last_cdc_layer=last_cdc_layer,
            ndf_hits=ndf_hits,
            isPrimary_misID=isPrimary_misID,
            ismatched=ismatched,
            isprimary=isprimary,
            charge_truth=charge_truth,
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
