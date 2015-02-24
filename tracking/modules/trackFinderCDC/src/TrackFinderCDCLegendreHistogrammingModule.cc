#include <tracking/modules/trackFinderCDC/TrackFinderCDCLegendreHistogrammingModule.h>

#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>


#include <tracking/trackFindingCDC/legendre/CDCLegendreSimpleFilter.h>


using namespace std;
using namespace Belle2;
using namespace CDC;
using namespace TrackFindingCDC;

#define SQR(x) ((x)*(x)) //we will use it in least squares fit

//ROOT macro
REG_MODULE(CDCLegendreHistogramming)

CDCLegendreHistogrammingModule::CDCLegendreHistogrammingModule()
{
  addParam("CDCHitsColName", m_param_cdcHitsColumnName,
           "Input CDCHits collection (should be created by CDCDigi module)",
           string("CDCHits"));

  addParam("GFTrackCandidatesColName", m_param_trackCandidatesColumnName,
           "Output GFTrackCandidates collection",
           string("TrackCands"));
}

CDCLegendreHistogrammingModule::~CDCLegendreHistogrammingModule()
{

}

void CDCLegendreHistogrammingModule::initialize()
{


}

void CDCLegendreHistogrammingModule::event()
{

  B2INFO("**********   CDCLegendreHistogrammingModule  ************");

  StoreArray<CDCHit> cdcHits(m_param_cdcHitsColumnName);
  B2DEBUG(100,
          "CDCTracking: Number of digitized Hits: " << cdcHits.getEntries());

  if (cdcHits.getEntries() == 0)
    B2WARNING("CDCTracking: cdcHitsCollection is empty!");

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


  //get genfit candidates from StoreArray
  StoreArray<genfit::TrackCand> gfTrackCands(m_param_trackCandidatesColumnName);

  if (gfTrackCands.getEntries() == 0) {
    B2WARNING("CDCTracking: gfTrackCands is empty!");
    return;
  }


  int tracknr(0);

  //Restore LegendreTrackCandidates using information from genfit candidates
  for (int iCand = 0; iCand < gfTrackCands.getEntries(); iCand++) {
    //vector with TrackHit objects belonging to the same track
    std::vector<TrackHit*> trackHits;
    for (TrackHit * hit : m_AxialHitList) {
      for (unsigned int iHit = 0; iHit < gfTrackCands[iCand]->getNHits(); iHit++) {
        if (hit->getStoreIndex() == gfTrackCands[iCand]->getHit(iHit)->getHitId()) {
          trackHits.push_back(hit);
          continue;
        }
      }
    }

    //set parameters of the track
//    std::pair<double, double> ref_point = std::make_pair(0., 0.);
    std::pair<double, double> track_par = std::make_pair(-999, -999);
//    double chi2;
    double r = 1.5 * 0.00299792458 / gfTrackCands[iCand]->getMomSeed().Pt();
    //here: theta is azimuthal angle of the candidate (in XY plane)
    double theta = gfTrackCands[iCand]->getMomSeed().Phi() - gfTrackCands[iCand]->getChargeSeed() * TMath::Pi() / 2.;

    if (theta < 0.) theta += TMath::Pi() * 2.;

    track_par.first = theta;
    track_par.second  = r;

    //estimate charge of the candidate
    int charge = TrackCandidate::getChargeAssumption(track_par.first,
                                                     track_par.second, trackHits);

    if (gfTrackCands[iCand]->getChargeSeed() > 0) charge = TrackCandidate::charge_positive;
    else charge = TrackCandidate::charge_negative;

    //create legendre track candidate; this object holds axial and stereo hits
    TrackCandidateWithStereoHits* trackCandidate = new TrackCandidateWithStereoHits(track_par.first, track_par.second, charge, trackHits, gfTrackCands[iCand]);
    //  trackCandidate->clearBadHits(ref_point);
    //    appendNewHits(trackCandidate);
//    trackCandidate->setChi2(chi2);

    //set reference point of the track
    trackCandidate->setReferencePoint(gfTrackCands[iCand]->getPosSeed().X(), gfTrackCands[iCand]->getPosSeed().Y());

    B2DEBUG(100, "R value: " << trackCandidate->getR() << "; theta: " << trackCandidate->getTheta() << "; radius: " << trackCandidate->getRadius() << "; phi: " << trackCandidate->getPhi() << "; charge: " << trackCandidate->getChargeSign() << "; Xc = " << trackCandidate->getXc() << "; Yc = " << trackCandidate->getYc());

    //  for (TrackHit * hit : trackCandidate->getTrackHits()) {
    //    hit->setHitUsage(TrackHit::not_used);
    //  }

//    trackCandidate->setCandidateType(TrackCandidate::tracklet);

    //store pointer to created object in a list for further usage
    m_trackList.push_back(trackCandidate);

  }


  //sort tracks by value of curvature
  m_trackList.sort([](const TrackCandidateWithStereoHits * a, const TrackCandidateWithStereoHits * b) {
    return static_cast <bool>(a->getRadius() > b->getRadius());
  });


  B2INFO("Number of track candidates: " << m_trackList.size());

  //create object which will add stereohits to tracks
  StereohitsProcesser stereohitsProcesser;

  for (TrackCandidate * cand : m_trackList) {
    tracknr++;
    B2INFO("Processing new track; assigning stereohits.");

    //assign stereohits to the track
    stereohitsProcesser.makeHistogramming(cand, m_StereoHitList);
  }


  //extend genfit candidates (from StoreArray) with new stereohits
  for (TrackCandidateWithStereoHits * trackCand : m_trackList) {
    for (int iCand = 0; iCand < gfTrackCands.getEntries(); iCand++) {
      if (trackCand->getGfTrackCand() != gfTrackCands[iCand]) continue;

//     std::pair<double, double> ref_point_temp = std::make_pair(0., 0.);
      TVector3 position;
      position = trackCand->getReferencePoint();

      TVector3 momentum = trackCand->getMomentumEstimation(true);

      int pdg = trackCand->getChargeSign() * (211);

      gfTrackCands[iCand]->setPosMomSeedAndPdgCode(position, momentum, pdg);

      std::vector<TrackHit*>& trackHitVector = trackCand->getTrackHits();

      for (TrackHit * trackHit : trackHitVector) {
        //add only stereohits
        if (trackHit->getIsAxial()) continue;
        int hitID = trackHit->getStoreIndex();
        gfTrackCands[iCand]->addHit(Const::CDC, hitID);
      }
      gfTrackCands[iCand]->sortHits();
    }

  }

  //clear pointers
  clear_pointer_vectors();

}

void CDCLegendreHistogrammingModule::clear_pointer_vectors()
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
