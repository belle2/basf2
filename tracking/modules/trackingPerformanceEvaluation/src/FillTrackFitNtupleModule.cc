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

#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/HitPatternVXD.h>

#include <map>

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

void FillTrackFitNtupleModule::initialize()
{
  // Tracks, RecoTracks needed for this module
  m_RecoTracks.isRequired();
  m_Tracks.isRequired();

  //set the ROOT File
  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  //now create ntuples
  TString var_list("evt:run:exp:prod:nhits:ncdc:npxd:nsvd:seed_x:");
  var_list += ("seed_y:seed_z:seed_px:seed_py:seed_pz:seed_p:seed_pt:seed_theta:seed_phi:seed_charge:");
  var_list += ("nhits_pi:ncdc_pi:npxd_pi:nsvd_pi:nhits_k:ncdc_k:npxd_k:nsvd_k:nhits_p:ncdc_p:npxd_p:nsvd_p:nhits_d:ncdc_d:npxd_d:nsvd_d:");
  var_list += ("flag_pi:flag_k:flag_p:flag_d:");
  var_list += ("trk_x_pi:trk_y_pi:trk_z_pi:trk_px_pi:trk_py_pi:trk_pz_pi:trk_p_pi:trk_pt_pi:trk_theta_pi:trk_phi_pi:");
  var_list += ("trk_charge_pi:trk_chi2_pi:trk_ndf_pi:trk_pvalue_pi:nfailed_pi:");
  var_list += ("trk_x_k:trk_y_k:trk_z_k:trk_px_k:trk_py_k:trk_pz_k:trk_p_k:trk_pt_k:trk_theta_k:trk_phi_k:");
  var_list += ("trk_charge_k:trk_chi2_k:trk_ndf_k:trk_pvalue_k:nfailed_k:");
  var_list += ("trk_x_p:trk_y_p:trk_z_p:trk_px_p:trk_py_p:trk_pz_p:trk_p_p:trk_pt_p:trk_theta_p:trk_phi_p:");
  var_list += ("trk_charge_p:trk_chi2_p:trk_ndf_p:trk_pvalue_p:nfailed_p:");
  var_list += ("trk_x_d:trk_y_d:trk_z_d:trk_px_d:trk_py_d:trk_pz_d:trk_p_d:trk_pt_d:trk_theta_d:trk_phi_d:");
  var_list += ("trk_charge_d:trk_chi2_d:trk_ndf_d:trk_pvalue_d:nfailed_d:");
  var_list += ("cdcf_pi:cdcl_pi:svdf_pi:svdl_pi:");
  var_list += ("cdcf_k:cdcl_k:svdf_k:svdl_k:");
  var_list += ("cdcf_p:cdcl_p:svdf_p:svdl_p:");
  var_list += ("cdcf_d:cdcl_d:svdf_d:svdl_d");
  m_n_MultiParticle = new TNtuple("nMultiParticle", "ntuple for multi hyp particle", var_list);
}

void FillTrackFitNtupleModule::event()
{

  StoreObjPtr<EventMetaData> eventMetaData("EventMetaData", DataStore::c_Event);
  Float_t event_num = eventMetaData->getEvent();
  Float_t event_run = eventMetaData->getRun();
  Float_t event_exp = eventMetaData->getExperiment();
  Float_t event_prod = eventMetaData->getProduction();

  B2DEBUG(29, "+++++ Loop on Tracks");
  StoreArray<Track> tracks(m_Tracks);

  for (Track& track : tracks) {

    RecoTrack* recoTrack = track.getRelationsTo<RecoTrack>()[0];
    if (recoTrack == nullptr) {
      // if no recoTrack is associated to Track, we skip the track
      B2WARNING(" the RecoTrack associated to Track is nullptr!");
      continue;
    }

    Float_t nhits = recoTrack->getNumberOfTrackingHits();
    Float_t ncdc = recoTrack->getNumberOfCDCHits();
    Float_t npxd = recoTrack->getNumberOfPXDHits();
    Float_t nsvd =  recoTrack->getNumberOfSVDHits();

    const Const::ChargedStable pdg_list[4] = {Const::pion, Const::kaon, Const::proton, Const::deuteron}; // loop only on these hypotheses

    std::map <Const::ChargedStable, Float_t> flag; // is the particle hypothesis existing in the track?
    std::map <Const::ChargedStable, Float_t> trk_x, trk_y, trk_z, trk_px, trk_py, trk_pz, trk_p, trk_pt, trk_theta, trk_phi;
    std::map <Const::ChargedStable, Float_t> trk_charge, trk_chi2, trk_ndf, trk_pvalue, trk_nfailed;
    std::map <Const::ChargedStable, Float_t> nhits_pid, ncdc_pid, nsvd_pid, npxd_pid;
    std::map <Const::ChargedStable, Float_t> first_cdc, last_cdc, first_svd, last_svd;

    for (const Const::ChargedStable& pdgIter : pdg_list) {
      trk_x[pdgIter] = 0.; trk_y[pdgIter] = 0.; trk_z[pdgIter] = 0.;
      trk_px[pdgIter] = 0.; trk_py[pdgIter] = 0.; trk_pz[pdgIter] = 0.;
      trk_p[pdgIter] = 0.; trk_pt[pdgIter] = 0.; trk_theta[pdgIter] = 0.; trk_phi[pdgIter] = 0.;
      trk_charge[pdgIter] = 0.;
      trk_chi2[pdgIter] = 0.; trk_ndf[pdgIter] = 0.; trk_pvalue[pdgIter] = 0.; trk_nfailed[pdgIter] = 0.;
      nhits_pid[pdgIter] = 0.; ncdc_pid[pdgIter] = 0.; nsvd_pid[pdgIter] = 0.; npxd_pid[pdgIter] = 0.;
      first_cdc[pdgIter] = -100.; last_cdc [pdgIter] = -100.;
      first_svd[pdgIter] = -100.; last_svd [pdgIter] = -100.;

      const TrackFitResult* fitResult = track.getTrackFitResult(pdgIter);
      if ((fitResult != nullptr) && (fitResult->getParticleType() == pdgIter)) {
        flag[pdgIter] = kTRUE;
      } else {
        flag[pdgIter] = kFALSE;
        continue;
      }

      trk_x[pdgIter] = fitResult->getPosition().X();
      trk_y[pdgIter] = fitResult->getPosition().Y();
      trk_z[pdgIter] = fitResult->getPosition().Z();
      trk_px[pdgIter] = fitResult->getMomentum().X();
      trk_py[pdgIter] = fitResult->getMomentum().Y();
      trk_pz[pdgIter] = fitResult->getMomentum().Z();
      trk_p[pdgIter] = fitResult->getMomentum().Mag();
      trk_pt[pdgIter] = fitResult->getMomentum().Pt();
      trk_theta[pdgIter] = fitResult->getMomentum().Theta() * TMath::RadToDeg();
      trk_phi[pdgIter] = fitResult->getMomentum().Phi() * TMath::RadToDeg();
      trk_charge[pdgIter] = fitResult->getChargeSign();
      double chi2 = recoTrack->getTrackFitStatus(recoTrack->getTrackRepresentationForPDG(pdgIter.getPDGCode()))->getChi2();
      if (isnan(chi2)) chi2 = -10;
      if (isinf(chi2)) chi2 = -20;
      trk_chi2[pdgIter] =  chi2;
      trk_ndf[pdgIter] =  recoTrack->getTrackFitStatus(recoTrack->getTrackRepresentationForPDG(pdgIter.getPDGCode()))->getNdf();
      trk_pvalue[pdgIter] =  fitResult->getPValue();
      trk_nfailed[pdgIter] = recoTrack->getTrackFitStatus(recoTrack->getTrackRepresentationForPDG(
                                                            pdgIter.getPDGCode()))->getNFailedPoints();
      ncdc_pid[pdgIter] = fitResult->getHitPatternCDC().getNHits();
      npxd_pid[pdgIter] = fitResult->getHitPatternVXD().getNPXDHits();
      nsvd_pid[pdgIter] = fitResult->getHitPatternVXD().getNSVDHits();
      nhits_pid[pdgIter] = ncdc_pid[pdgIter] + npxd_pid[pdgIter] + nsvd_pid[pdgIter];
      first_cdc[pdgIter] = fitResult->getHitPatternCDC().getFirstLayer();
      last_cdc[pdgIter] = fitResult->getHitPatternCDC().getLastLayer();
      first_svd[pdgIter] = fitResult->getHitPatternVXD().getFirstSVDLayer();
      last_svd[pdgIter] = fitResult->getHitPatternVXD().getLastSVDLayer();
    }

    Float_t buffer[] = {event_num, event_run, event_exp, event_prod,
                        nhits, ncdc, npxd, nsvd,
                        (Float_t)recoTrack->getPositionSeed().X(), (Float_t)recoTrack->getPositionSeed().Y(), (Float_t)recoTrack->getPositionSeed().Z(),
                        (Float_t)recoTrack->getMomentumSeed().X(), (Float_t)recoTrack->getMomentumSeed().Y(), (Float_t)recoTrack->getMomentumSeed().Z(), (Float_t)recoTrack->getMomentumSeed().Mag(), (Float_t)recoTrack->getMomentumSeed().Perp(),
                        (Float_t)(recoTrack->getMomentumSeed().Theta()* TMath::RadToDeg()), (Float_t)(recoTrack->getMomentumSeed().Phi()* TMath::RadToDeg()), (Float_t)recoTrack->getChargeSeed(),
                        nhits_pid[Const::pion], ncdc_pid[Const::pion], npxd_pid[Const::pion], nsvd_pid[Const::pion],
                        nhits_pid[Const::kaon], ncdc_pid[Const::kaon], npxd_pid[Const::kaon], nsvd_pid[Const::kaon],
                        nhits_pid[Const::proton], ncdc_pid[Const::proton], npxd_pid[Const::proton], nsvd_pid[Const::proton],
                        nhits_pid[Const::deuteron], ncdc_pid[Const::deuteron], npxd_pid[Const::deuteron], nsvd_pid[Const::deuteron],
                        flag[Const::pion], flag[Const::kaon], flag[Const::proton], flag[Const::deuteron],
                        trk_x[Const::pion], trk_y[Const::pion], trk_z[Const::pion],
                        trk_px[Const::pion], trk_py[Const::pion], trk_pz[Const::pion], trk_p[Const::pion], trk_pt[Const::pion], trk_theta[Const::pion],
                        trk_phi[Const::pion], trk_charge[Const::pion], trk_chi2[Const::pion], trk_ndf[Const::pion], trk_pvalue[Const::pion], trk_nfailed[Const::pion],
                        trk_x[Const::kaon], trk_y[Const::kaon], trk_z[Const::kaon],
                        trk_px[Const::kaon], trk_py[Const::kaon], trk_pz[Const::kaon], trk_p[Const::kaon], trk_pt[Const::kaon], trk_theta[Const::kaon],
                        trk_phi[Const::kaon], trk_charge[Const::kaon], trk_chi2[Const::kaon], trk_ndf[Const::kaon], trk_pvalue[Const::kaon], trk_nfailed[Const::kaon],
                        trk_x[Const::proton], trk_y[Const::proton], trk_z[Const::proton],
                        trk_px[Const::proton], trk_py[Const::proton], trk_pz[Const::proton], trk_p[Const::proton], trk_pt[Const::proton], trk_theta[Const::proton],
                        trk_phi[Const::proton], trk_charge[Const::proton], trk_chi2[Const::proton], trk_ndf[Const::proton], trk_pvalue[Const::proton], trk_nfailed[Const::proton],
                        trk_x[Const::deuteron], trk_y[Const::deuteron], trk_z[Const::deuteron],
                        trk_px[Const::deuteron], trk_py[Const::deuteron], trk_pz[Const::deuteron], trk_p[Const::deuteron], trk_pt[Const::deuteron], trk_theta[Const::deuteron],
                        trk_phi[Const::deuteron], trk_charge[Const::deuteron], trk_chi2[Const::deuteron], trk_ndf[Const::deuteron], trk_pvalue[Const::deuteron], trk_nfailed[Const::deuteron],
                        first_cdc[Const::pion], last_cdc[Const::pion], first_svd[Const::pion], last_svd[Const::pion],
                        first_cdc[Const::kaon], last_cdc[Const::kaon], first_svd[Const::kaon], last_svd[Const::kaon],
                        first_cdc[Const::proton], last_cdc[Const::proton], first_svd[Const::proton], last_svd[Const::proton],
                        first_cdc[Const::deuteron], last_cdc[Const::deuteron], first_svd[Const::deuteron], last_svd[Const::deuteron]
                       };
    m_n_MultiParticle->Fill(buffer);
  }



}


void FillTrackFitNtupleModule::terminate()
{

  if (m_rootFilePtr != nullptr) {
    m_rootFilePtr->cd();
    m_n_MultiParticle->Write();

    m_rootFilePtr->Close();
  }

}
