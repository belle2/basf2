#include <tracking/modules/trackFinderCDC/TrackFinderCDCLegendreDAFStereoAssigningModule.h>

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
REG_MODULE(CDCLegendreDAFStereoAssigning)

CDCLegendreDAFStereoAssigningModule::CDCLegendreDAFStereoAssigningModule()
{

}

CDCLegendreDAFStereoAssigningModule::~CDCLegendreDAFStereoAssigningModule()
{

}

void CDCLegendreDAFStereoAssigningModule::initialize()
{


}

void CDCLegendreDAFStereoAssigningModule::event()
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





  int m_failedFitCounter(0);
  int m_successfulGFTrackCandFitCounter(0);
  int m_failedGFTrackCandFitCounter(0);

  std::vector<std::pair<genfit::Track*, double>> gfTracksWithPVal;


  for (TrackCandidate * cand : m_trackList) {
    //there is different information from mctracks and 'real' pattern recognition tracks, e.g. for PR tracks the PDG is unknown

    double theta = 0;

    B2INFO(" Creating TrackCand");
    genfit::TrackCand* aTrackCandPointer = new genfit::TrackCand();

    TVector3 position;
//    position.SetXYZ(0.0, 0.0, 0.0);//at the moment there is no vertex determination in the ConformalFinder, but maybe the origin or the innermost hit are good enough as start values...
    position = cand->getReferencePoint();

    TVector3 momentum = cand->getMomentumEstimation(true);

    //Pattern recognition can determine only the charge, so here some dummy pdg value is set (with the correct charge), the pdg hypothesis can be then overwritten in the GenFitterModule

    int trackCharge;
    int vote_pos(0), vote_neg(0);

    //estimation of charge of the track candidate
    for (TrackHit * Hit : cand->getTrackHits()) {
      int curve_sign = Hit->getCurvatureSignWrt(cand->getXc(), cand->getYc());

      if (curve_sign == TrackCandidate::charge_positive)
        ++vote_pos;
      else if (curve_sign == TrackCandidate::charge_negative)
        ++vote_neg;
      else {
        B2ERROR(
          "Strange behaviour of TrackHit::getCurvatureSignWrt");
        exit(EXIT_FAILURE);
      }
    }

    if (vote_pos > vote_neg)
      trackCharge = TrackCandidate::charge_positive;

    else
      trackCharge = TrackCandidate::charge_negative;

    int pdg = trackCharge * (211);

    aTrackCandPointer->setPosMomSeedAndPdgCode(position, momentum, pdg);

    //find indices of the Hits
    std::vector<TrackHit*> trackHitVector;// = cand->getTrackHits();

    for (TrackHit * hit : cand->getTrackHits()) {
      trackHitVector.push_back(hit);
    }

    double Rcand = cand->getRadius();

    for (TrackHit * hit : m_StereoHitList) {


      if (hit->getHitUsage() == TrackHit::used_in_track) continue;

      std::pair<StereoHit, StereoHit> stereoHitPair;

      try {
        stereoHitPair = stereohitsProcesser.getDisplacements(cand, hit, trackCharge);
      } catch (const char* msg) {
        B2DEBUG(100, msg);
        continue;
      }

      double dist_1 = stereoHitPair.first.getDisplacement();
      double dist_2 = stereoHitPair.second.getDisplacement();

      if (std::isnan(dist_1)) continue;
      if (std::isnan(dist_2)) continue;


      double lWire = fabs(hit->getBackwardWirePosition().Z() - hit->getForwardWirePosition().Z());
      double rWire = sqrt(SQR(hit->getBackwardWirePosition().x() - hit->getForwardWirePosition().x()) + SQR(hit->getBackwardWirePosition().y() - hit->getForwardWirePosition().y()));

      if ((fabs(dist_1) > rWire / 3.) || (fabs(dist_2) > rWire / 3.)) continue;


      //---------------------*******************************************-----------------------------------------

      trackHitVector.push_back(hit);
    }

    TrackFindingCDC::TrackProcessor::sortHits(trackHitVector, cand->getChargeSign());


    for (TrackHit * trackHit : trackHitVector) {
      int hitID = trackHit->getStoreIndex();
      aTrackCandPointer->addHit(Const::CDC, hitID);
    }

    std::vector<int> m_pdgCodes;
    bool m_usePdgCodeFromTrackCand = true;

    if (m_usePdgCodeFromTrackCand == true) {
      // The following code assumes PDG code > 0, hence we take the absolute value.
      m_pdgCodes.clear(); //clear the pdg code from the last track
      m_pdgCodes.push_back(abs(aTrackCandPointer->getPdgCode()));
      B2DEBUG(100, "PDG code from track candidate will be used and it is: " << aTrackCandPointer->getPdgCode());
      if (m_pdgCodes[0] == 0) {
        B2FATAL("The current genfit::TrackCand has no valid PDG code (it is 0) AND you did not set any valid PDG Code in Genfit2Module module to use instead");
      }
    }

    const int nPdg = m_pdgCodes.size();  //number of pdg hypothesises
    bool candFitted = false;   //boolean to mark if the track candidates was fitted successfully with at least one PDG hypothesis

    for (int iPdg = 0; iPdg != nPdg; ++iPdg) {  // loop over all pdg hypothesises
      //make sure the track fit starts with the correct PDG code because the sign of the PDG code will also set the charge in the TrackRep
      TParticlePDG* part = TDatabasePDG::Instance()->GetParticle(m_pdgCodes[iPdg]);
      B2DEBUG(99, "GenFitter: current PDG code: " << m_pdgCodes[iPdg]);
      int currentPdgCode = boost::math::sign(aTrackCandPointer->getChargeSeed()) * m_pdgCodes[iPdg];
      if (currentPdgCode == 0) {
        B2FATAL("Either the charge of the current genfit::TrackCand is 0 or you set 0 as a PDG code");
      }
      if (part->Charge() < 0.0) {
        currentPdgCode *= -1; //swap sign
      }
      //std::cout << "fitting with pdg " << currentPdgCode << " for charge " << aTrackCandPointer->getChargeSeed() << std::endl;
      //Find the particle with the correct PDG Code;
      Const::ChargedStable chargedStable = Const::pion;
      try {
        chargedStable = Const::ChargedStable(abs(currentPdgCode));
      } catch (...) {
        //use pion as default
      }


      //get fit starting values from the from the track candidate
      const TVector3& posSeed = aTrackCandPointer->getPosSeed();
      const TVector3& momentumSeed = aTrackCandPointer->getMomSeed();

      B2DEBUG(99, "Fit track with start values: ");

      B2DEBUG(100, "Start values: momentum (x,y,z,abs): " << momentumSeed.x() << "  " << momentumSeed.y() << "  " << momentumSeed.z() << " " << momentumSeed.Mag());
      //B2DEBUG(100, "Start values: momentum std: " << sqrt(covSeed(3, 3)) << "  " << sqrt(covSeed(4, 4)) << "  " << sqrt(covSeed(5, 5)));
      B2DEBUG(100, "Start values: pos:   " << posSeed.x() << "  " << posSeed.y() << "  " << posSeed.z());
      //B2DEBUG(100, "Start values: pos std:   " << sqrt(covSeed(0, 0)) << "  " << sqrt(covSeed(1, 1)) << "  " << sqrt(covSeed(2, 2)));
      B2DEBUG(100, "Start values: pdg:      " << currentPdgCode);

      B2INFO(" Creating MeasurementFactory");

      genfit::MeasurementFactory<genfit::AbsMeasurement> factory;

      //create MeasurementProducer for CDC and add producer to the factory with correct detector Id

      if (cdcHits.getEntries()) {
        auto CDCProducer =  new genfit::MeasurementProducer <CDCHit, CDCRecoHit> (cdcHits.getPtr());
        factory.addProducer(Const::CDC, CDCProducer);
      }

      // The track fit needs an initial guess for the resolution.  The
      // values should roughly match the actual resolution (squared),
      // but it may be that different track-finders, regions of
      // phasespace and/or subdetectors perform better with different values.
      TMatrixDSym covSeed(6);
      covSeed(0, 0) = 1e-3;
      covSeed(1, 1) = 1e-3;
      covSeed(2, 2) = 4e-3;
      covSeed(3, 3) = 0.01e-3;
      covSeed(4, 4) = 0.01e-3;
      covSeed(5, 5) = 0.04e-3;
      aTrackCandPointer->setCovSeed(covSeed);

      B2INFO(" Creating RKTrackRep");

      genfit::RKTrackRep* trackRep = new genfit::RKTrackRep(currentPdgCode);

      B2INFO(" Creating Track");

      genfit::Track* gfTrack = new genfit::Track(*aTrackCandPointer, factory, trackRep); //create the track with the corresponding track representation

      B2INFO(" Checking Track");

      const int nHitsInTrack = gfTrack->getNumPointsWithMeasurement();
      B2DEBUG(99, "Total Nr of Hits assigned to the Track: " << nHitsInTrack);


      //Check which hits are contributing to the track
      int nCDC = 0;
      int nSVD = 0;
      int nPXD = 0;

      for (unsigned int hit = 0; hit < aTrackCandPointer->getNHits(); hit++) {
        int detId = 0;
        int hitId = 0;
        aTrackCandPointer->getHit(hit, detId, hitId);
        if (detId == Const::PXD) {
          nPXD++;
        } else if (detId == Const::SVD) {
          nSVD++;
        } else if (detId == Const::CDC) {
          nCDC++;
        } else {
          B2WARNING("Hit from unknown detectorID has contributed to this track! The unknown id is: " << detId);
        }
      }
      B2DEBUG(99, "            (CDC: " << nCDC << ", SVD: " << nSVD << ", PXD: " << nPXD << ")");

      if (aTrackCandPointer->getNHits() < 3) { // this should not be nessesary because track finder should only produce track candidates with enough hits to calculate a momentum
        B2WARNING("Genfit2Module: only " << aTrackCandPointer->getNHits() << " were assigned to the Track! This Track will not be fitted!");
        ++m_failedFitCounter;
        continue;
      }

      // Select the fitter.  scoped_ptr ensures that it's destructed at the right point.

      B2INFO(" Creating AbsKalmanFitter");

      boost::scoped_ptr<genfit::AbsKalmanFitter> fitter(0);

      /*
      if (m_filterId == "Kalman") {
        fitter.reset(new genfit::KalmanFitterRefTrack());
        fitter->setMultipleMeasurementHandling(genfit::unweightedClosestToPredictionWire);
      } else if (m_filterId == "DAF") {
        // FIXME ... testing
        //fitter.reset(new genfit::DAF(false));
        fitter.reset(new genfit::DAF(true));
        ((genfit::DAF*)fitter.get())->setProbCut(m_probCut);
      } else if (m_filterId == "simpleKalman") {
        fitter.reset(new genfit::KalmanFitter(4, 1e-3, 1e3, false));
        fitter->setMultipleMeasurementHandling(genfit::unweightedClosestToPredictionWire);
      } else {
        B2FATAL("Unknown filter id " << m_filterId << " requested.");
      }
      */
      /*
       fitter.reset(new genfit::KalmanFitterRefTrack());
       fitter->setMultipleMeasurementHandling(genfit::unweightedClosestToPredictionWire);
        */

      fitter.reset(new genfit::DAF(true));
      double m_probCut(0.001);
      ((genfit::DAF*)fitter.get())->setProbCut(m_probCut);


      int m_nMinIter(3);
      int m_nMaxIter(10);
      int m_nMaxFailed(5);

      string m_resolveWireHitAmbi("default");

      fitter->setMinIterations(m_nMinIter);
      fitter->setMaxIterations(m_nMaxIter);
      fitter->setMaxFailedHits(m_nMaxFailed);
      if (m_resolveWireHitAmbi != "default") {
        if (m_resolveWireHitAmbi == "weightedAverage") {
          fitter->setMultipleMeasurementHandling(genfit::weightedAverage);
        } else if (m_resolveWireHitAmbi == "unweightedClosestToReference") {
          fitter->setMultipleMeasurementHandling(genfit::unweightedClosestToReference);
        } else if (m_resolveWireHitAmbi == "unweightedClosestToPrediction") {
          fitter->setMultipleMeasurementHandling(genfit::unweightedClosestToPrediction);
        } else {
          B2FATAL("Unknown wire hit ambiguity handling " << m_resolveWireHitAmbi << " requested");
        }
      }

      //now fit the track
      try {

        fitter->processTrack(gfTrack);

        //gfTrack.Print();
        bool fitSuccess = gfTrack->hasFitStatus(trackRep);
        genfit::FitStatus* fs = 0;
        genfit::KalmanFitStatus* kfs = 0;
        if (fitSuccess) {
          fs = gfTrack->getFitStatus(trackRep);
          fitSuccess = fitSuccess && fs->isFitted();
          fitSuccess = fitSuccess && fs->isFitConverged();
          kfs = dynamic_cast<genfit::KalmanFitStatus*>(fs);
          fitSuccess = fitSuccess && kfs;
        }
        B2INFO("-----> Fit results:");
        B2INFO("       Fitted and converged: " << fitSuccess);
        if (fitSuccess) {
          B2INFO("       Chi2 of the fit: " << kfs->getChi2());
          //B2DEBUG(99,"       Forward Chi2: "<<gfTrack.getForwardChi2());
          B2INFO("       NDF of the fit: " << kfs->getBackwardNdf());
          //Calculate probability
          double pValue = gfTrack->getFitStatus()->getPVal();
          B2INFO("       pValue of the fit: " << pValue);

          candFitted = true;

          TVector3 pos(0, 0, 0 /*m_beamSpot.at(0), m_beamSpot.at(1), m_beamSpot.at(2)*/); //origin
          TVector3 lineDirection(0, 0, 1); // beam axis
          TVector3 poca(0., 0., 0.); //point of closest approach
          TVector3 dirInPoca(0., 0., 0.); //direction of the track at the point of closest approach
          TMatrixDSym cov(6);

          genfit::MeasuredStateOnPlane mop = gfTrack->getFittedState();
          mop.extrapolateToLine(pos, lineDirection);
          mop.getPosMomCov(poca, dirInPoca, cov);

          theta = dirInPoca.Theta();

          B2INFO("theta = " << theta * 180. / 3.1415);

          stereohitsProcesser.assignStereohitsByAngle(cand, theta, m_StereoHitList, poca.Z());

          gfTracksWithPVal.push_back(std::make_pair(gfTrack, pValue));
        }

      } catch (...) {
        B2WARNING("Something went wrong during the fit!");
        ++m_failedFitCounter;
      }

    } //end loop over all pdg hypothesis

    if (candFitted == true) m_successfulGFTrackCandFitCounter++;
    else m_failedGFTrackCandFitCounter++;
  }

  std::sort(gfTracksWithPVal.begin(), gfTracksWithPVal.end(),
  [](std::pair<genfit::Track*, double> const a, std::pair<genfit::Track*, double> const b) {return a.second < b.second;});

  for (auto entry : gfTracksWithPVal) {
    B2INFO("pVal form track: " << entry.first->getFitStatus()->getPVal() << "; pVal from vector: " << entry.second);
  }

  B2INFO(" m_failedFitCounter = " << m_failedFitCounter);
  B2INFO(" m_successfulGFTrackCandFitCounter = " << m_successfulGFTrackCandFitCounter);
  B2INFO(" m_failedGFTrackCandFitCounter = " << m_failedGFTrackCandFitCounter);




//      stereohitsProcesser.assignStereohitsByAngle(cand, thetaTrack, m_StereoHitList);











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

void CDCLegendreDAFStereoAssigningModule::clear_pointer_vectors()
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
