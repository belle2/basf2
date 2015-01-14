#include <tracking/modules/trackFinderCDC/TrackFinderCDCLegendreVXDStereoMergerModule.h>

#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

#include <genfit/Track.h>
#include <genfit/TrackCand.h>
#include <genfit/RKTrackRep.h>
#include <genfit/AbsKalmanFitter.h>
#include <genfit/KalmanFitter.h>
#include <genfit/KalmanFitterRefTrack.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/DAF.h>
#include <genfit/Exception.h>
#include <genfit/MeasurementFactory.h>
#include <cdc/dataobjects/CDCRecoHit.h>
#include <TDatabasePDG.h>


#include <tracking/trackFindingCDC/legendre/CDCLegendreSimpleFilter.h>


using namespace std;
using namespace Belle2;
using namespace CDC;
using namespace TrackFindingCDC;

#define SQR(x) ((x)*(x)) //we will use it in least squares fit

//ROOT macro
REG_MODULE(CDCLegendreVXDStereoMerger)

CDCLegendreVXDStereoMergerModule::CDCLegendreVXDStereoMergerModule()
{

}

CDCLegendreVXDStereoMergerModule::~CDCLegendreVXDStereoMergerModule()
{

}

void CDCLegendreVXDStereoMergerModule::initialize()
{


}

void CDCLegendreVXDStereoMergerModule::event()
{

  B2INFO("**********   CDCLegendreHistogrammingModule  ************");

  StoreArray<CDCHit> cdcHits("");
  B2DEBUG(100,
          "CDCTracking: Number of digitized Hits: " << cdcHits.getEntries());

  if (cdcHits.getEntries() == 0)
    B2WARNING("CDCTracking: cdcHitsCollection is empty!");


  StoreArray<genfit::TrackCand> gfTrackCands("TrackCands");

  if (gfTrackCands.getEntries() == 0) {
    B2WARNING("CDCTracking: gfTrackCands is empty!");
    return;
  }

//  if (cdcHits.getEntries() > 1500) {
//    B2INFO("** Skipping track finding due to too large number of hits **");
//    return;
//  }

  //Convert CDCHits to own Hit class
  for (int iHit = 0; iHit < cdcHits.getEntries(); iHit++) {
    TrackHit* trackHit = new TrackHit(cdcHits[iHit], iHit);
    if (trackHit->checkHitDriftLength()) {
      if (trackHit->getIsAxial())
        m_AxialHitList.push_back(trackHit);
      else
        m_StereoHitList.push_back(trackHit);
    } else {
      delete trackHit;
    }
  }





  int tracknr(0);

  for (int iCand = 0; iCand < gfTrackCands.getEntries(); iCand++) {
    std::vector<TrackHit*> trackHits;
    for (TrackHit * hit : m_AxialHitList) {
      for (int iHit = 0; iHit < gfTrackCands[iCand]->getNHits(); iHit++) {
        if (hit->getStoreIndex() == gfTrackCands[iCand]->getHit(iHit)->getHitId()) {
          trackHits.push_back(hit);
          continue;
        }
      }
    }


    std::pair<double, double> ref_point = std::make_pair(0., 0.);
    std::pair<double, double> track_par = std::make_pair(-999, -999);
    double chi2;
    double r = 1.5 * 0.00299792458 / gfTrackCands[iCand]->getMomSeed().Pt();
    double theta = gfTrackCands[iCand]->getMomSeed().Phi() - gfTrackCands[iCand]->getChargeSeed() * TMath::Pi() / 2.;

    if (theta < 0.) theta += TMath::Pi() * 2.;

    track_par.first = theta;
    track_par.second  = r;

    int charge = TrackCandidate::getChargeAssumption(track_par.first,
                                                     track_par.second, trackHits);

    if (gfTrackCands[iCand]->getChargeSeed() > 0) charge = TrackCandidate::charge_positive;
    else charge = TrackCandidate::charge_negative;

    TrackCandidateWithStereoHits* trackCandidate = new TrackCandidateWithStereoHits(track_par.first, track_par.second, charge, trackHits, gfTrackCands[iCand]);
    //  trackCandidate->clearBadHits(ref_point);
    //    appendNewHits(trackCandidate);
//    trackCandidate->setChi2(chi2);

    trackCandidate->setReferencePoint(gfTrackCands[iCand]->getPosSeed().X(), gfTrackCands[iCand]->getPosSeed().Y());

    B2DEBUG(100, "R value: " << trackCandidate->getR() << "; theta: " << trackCandidate->getTheta() << "; radius: " << trackCandidate->getRadius() << "; phi: " << trackCandidate->getPhi() << "; charge: " << trackCandidate->getChargeSign() << "; Xc = " << trackCandidate->getXc() << "; Yc = " << trackCandidate->getYc());

    //  for (TrackHit * hit : trackCandidate->getTrackHits()) {
    //    hit->setHitUsage(TrackHit::not_used);
    //  }

//    trackCandidate->setCandidateType(TrackCandidate::tracklet);

    m_trackList.push_back(trackCandidate);

  }


  //sort tracks by value of curvature
  m_trackList.sort([](const TrackCandidateWithStereoHits * a, const TrackCandidateWithStereoHits * b) {
    return static_cast <bool>(a->getRadius() > b->getRadius());
  });


  B2INFO("Number of track candidates: " << m_trackList.size());

  StereohitsProcesser stereohitsProcesser;




  //merging of CDC track with VXD tracks in XY projection allows to set polar angle of track canidate
  //basing on polar angle of VXD track we can assign stereohis to track candidate

  std::string m_VXDGFTracksColName("VXDTracks");
  StoreArray< genfit::Track > VXDGFTracks(m_VXDGFTracksColName);

//  StoreArray<genfit::TrackCand> CandArray(m_gfTrackCandsColName);
  genfit::KalmanFitter kalmanFilter; /**< kalman filter object to fit the track */

  // if (CandArray.getEntries() == 0)
  // return;
  double m_CDC_wall_radius = 16.25;
  TVector3 position(0., 0., 0.);
  TVector3 momentum(0., 0., 1.);

  B2INFO("Number of VXD tracks: " << VXDGFTracks.getEntries());

  for (TrackCandidate * cand : m_trackList) {

    TVector3 mom;
    TVector3 pos;
    TVector3 vxdmom;
    TVector3 vxdpos;

    //Here we choose innermost hit of track candidate and then extrapolate VXD track onto it
    double dist_hit_min = 999.;
    double driftLength_hit_min = 999;
    for (TrackHit * hit : cand->getTrackHits()) {
      if (not hit->getIsAxial()) continue;

      double dist_hit = hit->getOriginalWirePosition().Pt();

      if (dist_hit < dist_hit_min) {
        dist_hit_min = dist_hit;
        pos.SetXYZ(hit->getOriginalWirePosition().X(), hit->getOriginalWirePosition().Y(), 0);
        driftLength_hit_min = hit->getDriftLength();
      }

    }

    genfit::Track* bestVXDTrackToMerge = nullptr;
    int bestVXDTrackToMergeID = -1;
    double min_dist = 999.;
    TVector3 vxdmomBest;
    TVector3 vxdposBest;

    for (unsigned int iVxdTrack = 0; iVxdTrack < VXDGFTracks.getEntries(); iVxdTrack++) { //extrapolate to the innermost hit of track candidate
      double chi_2 = 999;
      double CHI2_MAX = 999;
      try {
        genfit::MeasuredStateOnPlane vxd_sop = VXDGFTracks[iVxdTrack]->getFittedState(-1);
        vxd_sop.extrapolateToCylinder(dist_hit_min/*m_CDC_wall_radius*/, position, momentum);
        vxdpos = vxd_sop.getPos();
        vxdmom = vxd_sop.getMom();
      } catch (...) {
        B2WARNING("VXDTrack extrapolation to cylinder failed!");
        continue;
      }


      pos.SetZ(0);
      vxdpos.SetZ(0);

      double merge_radius = 5.;
      double dist = TMath::Sqrt((pos - vxdpos) * (pos - vxdpos)) - driftLength_hit_min;

      // Extrapolate tracks to same plane & Match Tracks
      if (dist < merge_radius) {


        /*
        try {
          genfit::MeasuredStateOnPlane vxd_sop = VXDGFTracks[jtrack]->getFittedState(-1);
          genfit::MeasuredStateOnPlane cdc_sop = CDCGFTracks[itrack]->getFittedState();
          cdc_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
          vxd_sop.extrapolateToPlane(cdc_sop.getPlane());
        } catch (...) {
          B2WARNING("VXDTrack extrapolation to plane failed!");
          continue;
        }
        genfit::MeasuredStateOnPlane vxd_sop = VXDGFTracks[jtrack]->getFittedState(-1);
        genfit::MeasuredStateOnPlane cdc_sop = CDCGFTracks[itrack]->getFittedState();
        cdc_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
        vxd_sop.extrapolateToPlane(cdc_sop.getPlane());
        try {
          TMatrixDSym inv_covmtrx = (vxd_sop.getCov() + cdc_sop.getCov()).Invert();
        } catch (...) {
          B2WARNING("Covariance matrix is singular!");
          continue;
        }
        TMatrixDSym inv_covmtrx = (vxd_sop.getCov() + cdc_sop.getCov()).Invert();
        TVectorD state_diff = cdc_sop.getState() - vxd_sop.getState();
        state_diff *= inv_covmtrx;
        chi_2 = state_diff * (cdc_sop.getState() - vxd_sop.getState());

        if ((chi_2 < CHI2_MAX) && (chi_2 > 0)) {
          matched_track = 1;
          CHI2_MAX = chi_2;
          vxd_match = jtrack;
          vxd_xmin = vxd_sop.getPos();
          vxd_pmin = vxd_sop.getMom();
          matches++;
        }
          */


        if (min_dist > dist) {
          bestVXDTrackToMerge = VXDGFTracks[iVxdTrack];
          bestVXDTrackToMergeID = iVxdTrack;
          min_dist = dist;
          vxdmomBest = vxdmom;
          vxdposBest = vxdpos;
        }
      }
    }//end loop on VXD tracks

    if (bestVXDTrackToMerge) {
      if (bestVXDTrackToMergeID < 0)continue;
      vxdmomBest.SetZ(0);
      TVector3 candMom = cand->getMomentumEstimation();
      candMom.SetZ(0);
      double momDiff(0);
      momDiff = (vxdmomBest - candMom).Pt();
      B2INFO("momDiff = " << momDiff << "; momDiff/candMom = " << momDiff / candMom.Pt());
      if (momDiff > 1.) continue;

//      B2INFO("Getting FitResult object using DataStore::getRelationsToObj()!");
//      TrackFitResult* fitResult = DataStore::getRelatedFromObj<TrackFitResult>(bestVXDTrackToMerge);

//      if(not fitResult) continue;

//      B2INFO("Getting genfit::TrackCand object using TrackFitResult::getRelatedFrom()!");


      std::string m_VXDGFTracksCandColName("VXDTracksCand");
//      genfit::TrackCand* vxdTrackCandRel = fitResult->getRelatedFrom<genfit::TrackCand>(m_VXDGFTracksCandColName);

      const genfit::TrackCand* vxdTrackCandRel = DataStore::getRelatedToObj<genfit::TrackCand>(bestVXDTrackToMerge, m_VXDGFTracksCandColName);
      B2INFO("Done!");

//      if(not vxdTrackCandRel)continue;

      bool areTracksConsistent = false;

//      if (vxdTrackCandRel) areTracksConsistent = m_cdcLegendreTrackFitter->fitTrackCandWithSVD(cand, vxdTrackCandRel);

//      if (not areTracksConsistent) continue;

//      delete vxdTrackCand;

//      m_cdcLegendreTrackFitter->fitTrackCandWithTrack(cand, VXDGFTracks[bestVXDTrackToMergeID]);

      B2INFO("Tracks could be merged! dist = " << min_dist);
      genfit::MeasuredStateOnPlane vxd_sop = bestVXDTrackToMerge->getFittedState(-1);
      vxd_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
      vxdposBest = vxd_sop.getPos();
      vxdmomBest = vxd_sop.getMom();
      //TODO: extract theta angle from vxd track!
      double thetaTrack = vxdmom.Theta();



      stereohitsProcesser.assignStereohitsByAngle(cand, thetaTrack, m_StereoHitList);


      vxdTrackCandRel = nullptr;
      delete vxdTrackCandRel;

    }

    bestVXDTrackToMerge = nullptr;


//    delete cdcProducer;
//    delete trackRep;
  }


  for (TrackCandidateWithStereoHits * trackCand : m_trackList) {
    for (int iCand = 0; iCand < gfTrackCands.getEntries(); iCand++) {
      if (trackCand->getGfTrackCand() != gfTrackCands[iCand]) continue;

      std::pair<double, double> ref_point_temp = std::make_pair(0., 0.);
      TVector3 position;
      position = trackCand->getReferencePoint();

      TVector3 momentum = trackCand->getMomentumEstimation(true);

      int pdg = trackCand->getChargeSign() * (211);

      gfTrackCands[iCand]->setPosMomSeedAndPdgCode(position, momentum, pdg);

      std::vector<TrackHit*>& trackHitVector = trackCand->getTrackHits();

      for (TrackHit * trackHit : trackHitVector) {
        if (trackHit->getIsAxial()) continue;
        int hitID = trackHit->getStoreIndex();
        gfTrackCands[iCand]->addHit(Const::CDC, hitID);
      }
      gfTrackCands[iCand]->sortHits();
    }

  }

  clear_pointer_vectors();

}

void CDCLegendreVXDStereoMergerModule::clear_pointer_vectors()
{

  for (TrackHit * hit : m_AxialHitList) {
    delete hit;
  }
  m_AxialHitList.clear();

  for (TrackHit * hit : m_StereoHitList) {
    delete hit;
  }
  m_StereoHitList.clear();

  for (TrackCandidateWithStereoHits * track : m_trackList) {
    delete track;
  }
  m_trackList.clear();


}
