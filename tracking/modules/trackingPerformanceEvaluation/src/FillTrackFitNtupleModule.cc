/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefano Spataro                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackingPerformanceEvaluation/FillTrackFitNtupleModule.h>
#include <tracking/modules/trackingPerformanceEvaluation/PerformanceEvaluationBaseClass.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <framework/geometry/BFieldManager.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/HitPatternVXD.h>

#include <tracking/dataobjects/RecoTrack.h>

#include <genfit/KalmanFitterInfo.h>

#include <root/TTree.h>
#include <root/TObject.h>

#include <boost/foreach.hpp>

#include <typeinfo>
#include <cxxabi.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(FillTrackFitNtuple)

FillTrackFitNtupleModule::FillTrackFitNtupleModule() :
  Module()
{

  setDescription("This module fills a ntuple with tracking variables under different hypotheses");

  addParam("outputFileName", m_rootFileName, "Name of output root file.",
           std::string("TrackingPerformanceEvaluation_output.root"));
  addParam("TracksName", m_TracksName, "Name of Track collection.", std::string(""));
  addParam("RecoTracksName", m_RecoTracksName, "Name of RecoTrack collection.", std::string("RecoTracks"));
  addParam("ParticleHypothesis", m_ParticleHypothesis, "Particle Hypothesis used in the track fit.", int(211));

}

FillTrackFitNtupleModule::~FillTrackFitNtupleModule()
{

}

void FillTrackFitNtupleModule::initialize()
{
  // Tracks, RecoTracks needed for this module
  StoreArray<RecoTrack>::required(m_RecoTracksName);
  StoreArray<Track>::required(m_TracksName);

  //set the ROOT File
  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  //now create ntuples

  m_n_MultiParticle = new TNtuple("nMultiParticle", "ntuple for multi hyp particle",
                                  "evt:run:exp:prod:seed_x:seed_y:seed_z:seed_px:seed_py:seed_pz:seed_p:seed_pt:seed_theta:seed_phi:seed_charge:nhits_pi:ncdc_pi:npxd_pi:nsvd_pi:nhits_k:ncdc_k:npxd_k:nsvd_k:nhits_p:ncdc_p:npxd_p:nsvd_p:nhits_d:ncdc_d:npxd_d:nsvd_d:flag_pi:flag_k:flag_p:flag_d:trk_x_pi:trk_y_pi:trk_z_pi:trk_px_pi:trk_py_pi:trk_pz_pi:trk_p_pi:trk_pt_pi:trk_theta_pi:trk_phi_pi:trk_charge_pi:trk_chi2_pi:trk_ndf_pi:trk_pvalue_pi:nfailed_pi:trk_x_k:trk_y_k:trk_z_k:trk_px_k:trk_py_k:trk_pz_k:trk_p_k:trk_pt_k:trk_theta_k:trk_phi_k:trk_charge_k:trk_chi2_k:trk_ndf_k:trk_pvalue_k:nfailed_k:trk_x_p:trk_y_p:trk_z_p:trk_px_p:trk_py_p:trk_pz_p:trk_p_p:trk_pt_p:trk_theta_p:trk_phi_p:trk_charge_p:trk_chi2_p:trk_ndf_p:trk_pvalue_p:nfailed_p:trk_x_d:trk_y_d:trk_z_d:trk_px_d:trk_py_d:trk_pz_d:trk_p_d:trk_pt_d:trk_theta_d:trk_phi_d:trk_charge_d:trk_chi2_d:trk_ndf_d:trk_pvalue_d:nfailed_d:cdcf_pi:cdcl_pi:svdf_pi:svdl_pi:cdcf_k:cdcl_k:svdf_k:svdl_k:cdcf_p:cdcl_p:svdf_p:svdl_p:cdcf_d:cdcl_d:svdf_d:svdl_d");
}

void FillTrackFitNtupleModule::beginRun()
{

}

void FillTrackFitNtupleModule::event()
{

  StoreObjPtr<EventMetaData> eventMetaData("EventMetaData", DataStore::c_Event);
  int event_num = eventMetaData->getEvent();
  int event_run = eventMetaData->getRun();
  int event_exp = eventMetaData->getExperiment();
  int event_prod = eventMetaData->getProduction();

  B2DEBUG(99, "+++++ Loop on Tracks");
  StoreArray<Track> tracks(m_TracksName);

  BOOST_FOREACH(Track & track, tracks) {

    //RelationVector<RecoTrack> recoTrack_fromTrack = DataStore::getRelationsWithObj<RecoTrack>(&track);
    const RecoTrack* recoTrack = track.getRelationsTo<RecoTrack>()[0];
    if (recoTrack == NULL) std::cout << "pippo!!!!" << std::endl;

    const TrackFitResult* fitResult_pi = track.getTrackFitResult(Const::ChargedStable(211));
    const TrackFitResult* fitResult_k = track.getTrackFitResult(Const::ChargedStable(321));
    const TrackFitResult* fitResult_p = track.getTrackFitResult(Const::ChargedStable(2212));
    const TrackFitResult* fitResult_d = track.getTrackFitResult(Const::ChargedStable(1000010020));

    Bool_t flag_pi = kTRUE, flag_k = kTRUE, flag_p = kTRUE, flag_d = kTRUE;
    if ((fitResult_pi == NULL) || (fitResult_pi->getParticleType() != Const::ChargedStable(211))) flag_pi = kFALSE;
    if ((fitResult_k == NULL) || (fitResult_k->getParticleType() != Const::ChargedStable(321))) flag_k = kFALSE;
    if ((fitResult_p == NULL) || (fitResult_p->getParticleType() != Const::ChargedStable(2212))) flag_p = kFALSE;
    if ((fitResult_d == NULL) || (fitResult_d->getParticleType() != Const::ChargedStable(1000010020))) flag_d = kFALSE;

    Float_t trk_x_pi = 0, trk_y_pi = 0, trk_z_pi = 0, trk_px_pi = 0, trk_py_pi = 0, trk_pz_pi = 0, trk_p_pi = 0, trk_pt_pi = 0,
            trk_theta_pi = 0, trk_phi_pi = 0,
            trk_charge_pi = 0, trk_chi2_pi = 0, trk_ndf_pi = 0, trk_pvalue_pi = 0, nfailed_pi = 0;
    Float_t trk_x_k = 0, trk_y_k = 0, trk_z_k = 0, trk_px_k = 0, trk_py_k = 0, trk_pz_k = 0, trk_p_k = 0, trk_pt_k = 0, trk_theta_k = 0,
            trk_phi_k = 0, trk_charge_k = 0,
            trk_chi2_k = 0, trk_ndf_k = 0, trk_pvalue_k = 0, nfailed_k = 0;
    Float_t trk_x_p = 0, trk_y_p = 0, trk_z_p = 0, trk_px_p = 0, trk_py_p = 0, trk_pz_p = 0, trk_p_p = 0, trk_pt_p = 0, trk_theta_p = 0,
            trk_phi_p = 0, trk_charge_p = 0,
            trk_chi2_p = 0, trk_ndf_p = 0, trk_pvalue_p = 0, nfailed_p = 0;
    Float_t trk_x_d = 0, trk_y_d = 0, trk_z_d = 0, trk_px_d = 0, trk_py_d = 0, trk_pz_d = 0, trk_p_d = 0, trk_pt_d = 0, trk_theta_d = 0,
            trk_phi_d = 0, trk_charge_d = 0,
            trk_chi2_d = 0, trk_ndf_d = 0, trk_pvalue_d = 0, nfailed_d = 0;

    Float_t nhits_pi = 0, ncdc_pi = 0, npxd_pi = 0, nsvd_pi = 0;
    Float_t nhits_k = 0, ncdc_k = 0, npxd_k = 0, nsvd_k = 0;
    // nhits = recoTrack->getNumberOfTrackingHits();
    // ncdc = recoTrack->getNumberOfCDCHits();
    // npxd = recoTrack->getNumberOfPXDHits();
    // nsvd = recoTrack->getNumberOfSVDHits();
    Float_t nhits_p = 0, ncdc_p = 0, npxd_p = 0, nsvd_p = 0;
    Float_t nhits_d = 0, ncdc_d = 0, npxd_d = 0, nsvd_d = 0;
    Int_t first_cdc_pi = -100, last_cdc_pi = -100, first_svd_pi = -100, last_svd_pi;
    Int_t first_cdc_k = -100, last_cdc_k = -100, first_svd_k = -100, last_svd_k;
    Int_t first_cdc_p = -100, last_cdc_p = -100, first_svd_p = -100, last_svd_p;
    Int_t first_cdc_d = -100, last_cdc_d = -100, first_svd_d = -100, last_svd_d;
    const auto& trackReps = recoTrack->getRepresentations();
    for (const auto& trackRep : trackReps) {
      int PDG = std::abs(trackRep->getPDG());

      switch (PDG) {
        case 211:
          if (flag_pi) {
            trk_x_pi = fitResult_pi->getPosition().X();
            trk_y_pi = fitResult_pi->getPosition().Y();
            trk_z_pi = fitResult_pi->getPosition().Z();
            trk_px_pi = fitResult_pi->getMomentum().X();
            trk_py_pi = fitResult_pi->getMomentum().Y();
            trk_pz_pi = fitResult_pi->getMomentum().Z();
            trk_p_pi = fitResult_pi->getMomentum().Mag();
            trk_pt_pi = fitResult_pi->getMomentum().Pt();
            trk_theta_pi = fitResult_pi->getMomentum().Theta() * TMath::RadToDeg();
            trk_phi_pi = fitResult_pi->getMomentum().Phi() * TMath::RadToDeg();
            trk_charge_pi = fitResult_pi->getChargeSign();
            double chi2 = recoTrack->getTrackFitStatus(trackRep)->getChi2();
            if (isnan(chi2)) chi2 = -10;
            if (isinf(chi2)) chi2 = -20;
            trk_chi2_pi =  chi2;
            trk_ndf_pi =  recoTrack->getTrackFitStatus(trackRep)->getNdf();;
            trk_pvalue_pi =  fitResult_pi->getPValue();
            nfailed_pi = recoTrack->getTrackFitStatus(trackRep)->getNFailedPoints();
            ncdc_pi = fitResult_pi->getHitPatternCDC().getNHits();
            npxd_pi = fitResult_pi->getHitPatternVXD().getNPXDHits();
            nsvd_pi = fitResult_pi->getHitPatternVXD().getNSVDHits();
            nhits_pi = ncdc_pi + npxd_pi + nsvd_pi;
            first_cdc_pi = fitResult_pi->getHitPatternCDC().getFirstLayer();
            last_cdc_pi = fitResult_pi->getHitPatternCDC().getLastLayer();
            first_svd_pi = fitResult_pi->getHitPatternVXD().getFirstSVDLayer();
            last_svd_pi = fitResult_pi->getHitPatternVXD().getLastSVDLayer();
          }
          break;

        case 321:
          if (flag_k) {
            trk_x_k = fitResult_k->getPosition().X();
            trk_y_k = fitResult_k->getPosition().Y();
            trk_z_k = fitResult_k->getPosition().Z();
            trk_px_k = fitResult_k->getMomentum().X();
            trk_py_k = fitResult_k->getMomentum().Y();
            trk_pz_k = fitResult_k->getMomentum().Z();
            trk_p_k = fitResult_k->getMomentum().Mag();
            trk_pt_k = fitResult_k->getMomentum().Pt();
            trk_theta_k = fitResult_k->getMomentum().Theta() * TMath::RadToDeg();
            trk_phi_k = fitResult_k->getMomentum().Phi() * TMath::RadToDeg();
            trk_charge_k = fitResult_k->getChargeSign();
            double chi2 = recoTrack->getTrackFitStatus(trackRep)->getChi2();
            if (isnan(chi2)) chi2 = -10;
            if (isinf(chi2)) chi2 = -20;
            trk_chi2_k =  chi2;
            trk_ndf_k =  recoTrack->getTrackFitStatus(trackRep)->getNdf();;
            trk_pvalue_k =  fitResult_k->getPValue();
            nfailed_k = recoTrack->getTrackFitStatus(trackRep)->getNFailedPoints();
            ncdc_k = fitResult_k->getHitPatternCDC().getNHits();
            npxd_k = fitResult_k->getHitPatternVXD().getNPXDHits();
            nsvd_k = fitResult_k->getHitPatternVXD().getNSVDHits();
            nhits_k = ncdc_k + npxd_k + nsvd_k;
            first_cdc_k = fitResult_k->getHitPatternCDC().getFirstLayer();
            last_cdc_k = fitResult_k->getHitPatternCDC().getLastLayer();
            first_svd_k = fitResult_k->getHitPatternVXD().getFirstSVDLayer();
            last_svd_k = fitResult_k->getHitPatternVXD().getLastSVDLayer();
          }
          break;

        case 2212:
          if (flag_p) {
            trk_x_p = fitResult_p->getPosition().X();
            trk_y_p = fitResult_p->getPosition().Y();
            trk_z_p = fitResult_p->getPosition().Z();
            trk_px_p = fitResult_p->getMomentum().X();
            trk_py_p = fitResult_p->getMomentum().Y();
            trk_pz_p = fitResult_p->getMomentum().Z();
            trk_p_p = fitResult_p->getMomentum().Mag();
            trk_pt_p = fitResult_p->getMomentum().Pt();
            trk_theta_p = fitResult_p->getMomentum().Theta() * TMath::RadToDeg();
            trk_phi_p = fitResult_p->getMomentum().Phi() * TMath::RadToDeg();
            trk_charge_p = fitResult_p->getChargeSign();
            double chi2 = recoTrack->getTrackFitStatus(trackRep)->getChi2();
            if (isnan(chi2)) chi2 = -10;
            if (isinf(chi2)) chi2 = -20;
            trk_chi2_p =  chi2;
            trk_ndf_p =  recoTrack->getTrackFitStatus(trackRep)->getNdf();;
            trk_pvalue_p =  fitResult_p->getPValue();
            nfailed_p = recoTrack->getTrackFitStatus(trackRep)->getNFailedPoints();
            ncdc_p = fitResult_p->getHitPatternCDC().getNHits();
            npxd_p = fitResult_p->getHitPatternVXD().getNPXDHits();
            nsvd_p = fitResult_p->getHitPatternVXD().getNSVDHits();
            nhits_p = ncdc_p + npxd_p + nsvd_p;
            first_cdc_p = fitResult_p->getHitPatternCDC().getFirstLayer();
            last_cdc_p = fitResult_p->getHitPatternCDC().getLastLayer();
            first_svd_p = fitResult_p->getHitPatternVXD().getFirstSVDLayer();
            last_svd_p = fitResult_p->getHitPatternVXD().getLastSVDLayer();
          }
          break;

        case 1000010020:
          if (flag_d) {
            trk_x_d = fitResult_d->getPosition().X();
            trk_y_d = fitResult_d->getPosition().Y();
            trk_z_d = fitResult_d->getPosition().Z();
            trk_px_d = fitResult_d->getMomentum().X();
            trk_py_d = fitResult_d->getMomentum().Y();
            trk_pz_d = fitResult_d->getMomentum().Z();
            trk_p_d = fitResult_d->getMomentum().Mag();
            trk_pt_d = fitResult_d->getMomentum().Pt();
            trk_theta_d = fitResult_d->getMomentum().Theta() * TMath::RadToDeg();
            trk_phi_d = fitResult_d->getMomentum().Phi() * TMath::RadToDeg();
            trk_charge_d = fitResult_d->getChargeSign();
            double chi2 = recoTrack->getTrackFitStatus(trackRep)->getChi2();
            if (isnan(chi2)) chi2 = -10;
            if (isinf(chi2)) chi2 = -20;
            trk_chi2_d =  chi2;
            trk_ndf_d =  recoTrack->getTrackFitStatus(trackRep)->getNdf();;
            trk_pvalue_d =  fitResult_d->getPValue();
            nfailed_d = recoTrack->getTrackFitStatus(trackRep)->getNFailedPoints();
            ncdc_d = fitResult_d->getHitPatternCDC().getNHits();
            npxd_d = fitResult_d->getHitPatternVXD().getNPXDHits();
            nsvd_d = fitResult_d->getHitPatternVXD().getNSVDHits();
            nhits_d = ncdc_d + npxd_d + nsvd_d;
            first_cdc_d = fitResult_d->getHitPatternCDC().getFirstLayer();
            last_cdc_d = fitResult_d->getHitPatternCDC().getLastLayer();
            first_svd_d = fitResult_d->getHitPatternVXD().getFirstSVDLayer();
            last_svd_d = fitResult_d->getHitPatternVXD().getLastSVDLayer();
          }
      }
    }
    Float_t buffer[] = {event_num, event_run, event_exp, event_prod,
                        recoTrack->getPositionSeed().X(), recoTrack->getPositionSeed().Y(), recoTrack->getPositionSeed().Z(),
                        recoTrack->getMomentumSeed().X(), recoTrack->getMomentumSeed().Y(), recoTrack->getMomentumSeed().Z(), recoTrack->getMomentumSeed().Mag(), recoTrack->getMomentumSeed().Perp(),
                        recoTrack->getMomentumSeed().Theta()* TMath::RadToDeg(), recoTrack->getMomentumSeed().Phi()* TMath::RadToDeg(), recoTrack->getChargeSeed(),
                        nhits_pi, ncdc_pi, npxd_pi, nsvd_pi, nhits_k, ncdc_k, npxd_k, nsvd_k, nhits_p, ncdc_p, npxd_p, nsvd_p, nhits_d, ncdc_d, npxd_d, nsvd_d,
                        flag_pi, flag_k, flag_p, flag_d,
                        trk_x_pi, trk_y_pi, trk_z_pi,
                        trk_px_pi, trk_py_pi, trk_pz_pi, trk_p_pi, trk_pt_pi, trk_theta_pi,
                        trk_phi_pi, trk_charge_pi, trk_chi2_pi, trk_ndf_pi, trk_pvalue_pi, nfailed_pi,
                        trk_x_k, trk_y_k, trk_z_k,
                        trk_px_k, trk_py_k, trk_pz_k, trk_p_k, trk_pt_k, trk_theta_k,
                        trk_phi_k, trk_charge_k, trk_chi2_k, trk_ndf_k, trk_pvalue_k, nfailed_k,
                        trk_x_p, trk_y_p, trk_z_p,
                        trk_px_p, trk_py_p, trk_pz_p, trk_p_p, trk_pt_p, trk_theta_p,
                        trk_phi_p, trk_charge_p, trk_chi2_p, trk_ndf_p, trk_pvalue_p, nfailed_p,
                        trk_x_d, trk_y_d, trk_z_d,
                        trk_px_d, trk_py_d, trk_pz_d, trk_p_d, trk_pt_d, trk_theta_d,
                        trk_phi_d, trk_charge_d, trk_chi2_d, trk_ndf_d, trk_pvalue_d, nfailed_d,
                        first_cdc_pi, last_cdc_pi, first_svd_pi, last_svd_pi,
                        first_cdc_k, last_cdc_k, first_svd_k, last_svd_k,
                        first_cdc_p, last_cdc_p, first_svd_p, last_svd_p,
                        first_cdc_d, last_cdc_d, first_svd_d, last_svd_d
                       };
    m_n_MultiParticle->Fill(buffer);
  }



}


void FillTrackFitNtupleModule::terminate()
{

  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();
    m_n_MultiParticle->Write();

    m_rootFilePtr->Close();
  }

}
