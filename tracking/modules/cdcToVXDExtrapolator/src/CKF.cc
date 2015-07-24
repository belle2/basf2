/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ian J. Watson                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <tracking/modules/cdcToVXDExtrapolator/CKF.h>

#include <genfit/Track.h>
#include <genfit/TrackCand.h>
#include <genfit/RKTrackRep.h>
#include <genfit/AbsKalmanFitter.h>
#include <genfit/KalmanFitter.h>
#include <genfit/DAF.h>
#include <genfit/KalmanFitterInfo.h>
#include <genfit/KalmanFitterRefTrack.h>
#include <genfit/FitStatus.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/Exception.h>

using namespace Belle2;

CKF::CKF(genfit::Track* _track, bool (*_findHits)(genfit::Track*, unsigned, std::vector<genfit::AbsMeasurement*>&, void*),
         void* _data, double _maxChi2Increment, int _maxHoles) :
  seedTrack(_track),
  findHits(_findHits),
  data(_data),
  step(0),
  maxHoles(_maxHoles),
  maxChi2Increment(_maxChi2Increment)
{
}

genfit::Track* CKF::processTrack()
{
  auto tracks = new std::vector<genfit::Track*>;
  tracks->push_back(new genfit::Track(*seedTrack));

  auto fitter = new genfit::KalmanFitter();
  fitter->setMinIterations(3);
  fitter->setMaxIterations(10);
  fitter->setMultipleMeasurementHandling(genfit::unweightedClosestToPredictionWire);
  fitter->setMaxFailedHits(5);

  B2DEBUG(90, "- In CKF::processTrack(): new track");
  std::vector<genfit::AbsMeasurement*> allHits;
  for (step = 0; true; ++step) {
    // only process steps where we find new hits, take it from the
    // seed track because we assume we have a very wide window.
    std::vector<genfit::AbsMeasurement*> newHits;
    if (!findHits(seedTrack, step, newHits, data))
      break;

    if (newHits.size() == 0) {
      B2DEBUG(90, "-- In CKF::processTrack(): At step " << step << " no hits found, continuing search");
      continue;
    }
    if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 90, PACKAGENAME()) == true) {
      std::cerr << "-- In CKF::processTrack(): At step " << step << " given " << newHits.size() << " hits" << " ";
    }
    for (unsigned i = 0; i < newHits.size(); ++i) {
      allHits.push_back(newHits[i]);
      if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 90, PACKAGENAME()) == true) {
        std::cerr << newHits[i]->getHitId() << " ";
      }
    }
    if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 90, PACKAGENAME()) == true) {
      std::cerr << std::endl;
    }

    bool any = false;
    std::vector<genfit::Track*>* newtracks = new std::vector<genfit::Track*>;

    for (auto& track : *tracks) {
      // copy the no-added-hit case
      newtracks->push_back(track);
      /// if the track is bad, don't try to add hits
      if (!passPreUpdateTrim(track, step)) {
        continue;
      }
      any = true;
      if (newHits.size() != 0) {
        for (auto& hit : newHits) {
          genfit::Track* newtrack = new genfit::Track(*track);
          auto newhit = hit->clone();
          newtrack->insertMeasurement(newhit, 0);
          try {
            fitter->processTrackPartially(newtrack, newtrack->getCardinalRep(), 1, 0);
            auto up0 = newtrack->getPointWithMeasurementAndFitterInfo(0, newtrack->getCardinalRep())->getKalmanFitterInfo(0)->getUpdate(-1);
            //auto up1 = newtrack->getPointWithMeasurementAndFitterInfo(1, newtrack->getCardinalRep())->getKalmanFitterInfo(0)->getUpdate(-1);

            if (passPostUpdateTrim(newtrack, step)) {
              TMatrixDSym cov(6, 6), cov2(6, 6);
              TVector3 pos, mom, pos2, mom2;
              track->getFittedState().getPosMomCov(pos, mom, cov);
              up0->getPosMomCov(pos2, mom2, cov2);

              chi2Map[hit->getHitId()] = up0->getChiSquareIncrement();
              B2DEBUG(70, "--- In CKF::processTrack(): Added partial track step " << newtrack->getFitStatus(
                        newtrack->getCardinalRep())->getChi2() <<
                      "[" << newtrack->getFitStatus(newtrack->getCardinalRep())->getNdf() << "] " << up0->getChiSquareIncrement() << "{" << up0->getNdf()
                      << "}" << //" " << up1->getChiSquareIncrement() <<
                      " - " << pos2.X() << " " << pos2.Y() << " " << pos2.Z() << " " << " - " << pos.X() << " " << pos.Y() << " " << pos.Z() << " ");
              newtracks->push_back(newtrack);
            } else
              delete newtrack;
          } catch (...) {
            B2DEBUG(80, "--- In CKF::processTrack(): Track extrapolation failed");
            delete newtrack;
          }
        }
      }
    }

    if (!any) {
      delete newtracks;
      break;
    }

    B2DEBUG(90, "-- In CKF::processTrack(): At step " << step << " have " << newtracks->size() << " track extrapolations");
    // for (unsigned i = 0; i < tracks->size(); ++i)
    //   delete tracks->at(i);
    delete tracks;
    tracks = newtracks;
  }

  B2DEBUG(90, "- In CKF::processTrack(): Finished search, have " << tracks->size() << " track extrapolations");
  genfit::Track* best = bestTrack(tracks);
  genfit::Track* ret = 0;
  if (best)
    ret = new genfit::Track(*best);
  for (unsigned i = 0; i < tracks->size(); ++i)
    delete tracks->at(i);
  delete tracks;
  delete fitter;

  // everything is deleted so i hope we can delete the hits!
  for (unsigned i = 0; i < allHits.size(); ++i)
    delete allHits[i];


  if (ret) {
    B2DEBUG(90, "- In CKF::processTrack(): Found a best");
    if (refitTrack(ret)) {
      return ret;
    } else {
      B2DEBUG(90, "- In CKF::processTrack(): Refit failed. Returning seed track.");
      delete ret;
      return seedTrack;
    }
  } else {
    B2DEBUG(90, "- In CKF::processTrack(): No best");
    return seedTrack;
  }
}

bool CKF::passPreUpdateTrim(genfit::Track* track, unsigned step)
{
  // Check for holes
  int nadded = track->getNumPointsWithMeasurement() - seedTrack->getNumPointsWithMeasurement();
  int nholes = step - nadded;
  if (nholes > maxHoles) {
    B2DEBUG(70, "--- In CKF::preUpdateTrim(): Filtering track with nholes " << nholes << "{" << step << " " <<
            track->getNumPointsWithMeasurement() << " " << seedTrack->getNumPointsWithMeasurement() << " " << nadded << "}");
    return false;
  }
  return true;
}

bool CKF::passPostUpdateTrim(genfit::Track* track, unsigned)
{
  auto up0 = track->getPointWithMeasurementAndFitterInfo(0, track->getCardinalRep())->getKalmanFitterInfo(0)->getUpdate(-1);
  if ((up0->getChiSquareIncrement() / up0->getNdf()) > maxChi2Increment) {
    B2DEBUG(70, "--- In CKF::postUpdateTrim(): Filtering track with chi2 " << up0->getChiSquareIncrement() << "{" << up0->getNdf() <<
            "}");
    return false;
  }
  return true;
}

void chiSqNdf(genfit::Track* track, double& chi2, int& ndf)
{
  chi2 = 0; ndf = 0;
  for (unsigned i = 0; i < track->getNumPointsWithMeasurement(); ++i) {
    auto fi = track->getPointWithMeasurementAndFitterInfo(i, track->getCardinalRep());
    if (fi) {
      auto up = fi->getKalmanFitterInfo(0)->getUpdate(-1);
      chi2 += up->getChiSquareIncrement();
      ndf += up->getNdf();
    } else {
      B2WARNING("FitterInfo missing in track!");
    }
  }
}

genfit::Track* CKF::bestTrack(std::vector<genfit::Track*>* tracks)
{
  if (!tracks || tracks->size() == 0)
    return 0;
  unsigned iBest = 0;
  double chi2Best = tracks->at(0)->getFitStatus(tracks->at(0)->getCardinalRep())->getChi2();
  for (unsigned i = 0; i < tracks->size(); ++i) {
    double thisChi2(0); int ndf(0); // tracks->at(i)->getFitStatus(tracks->at(i)->getCardinalRep())->getChi2();
    chiSqNdf(tracks->at(i), thisChi2, ndf);
    B2DEBUG(90, "-- In CKF::bestTrack(): @ " << i << " " << thisChi2 << " " << ndf << " " << thisChi2 / ndf);
    thisChi2 = thisChi2 / ndf;
    if (thisChi2 < chi2Best) {
      chi2Best = thisChi2;
      iBest = i;
    }
  }
  B2DEBUG(90, "-- In CKF::bestTrack(): Best track @ " << iBest);

  return tracks->at(iBest);
}

bool CKF::refitTrack(genfit::Track* track)
{
  bool fitSuccess = false;
  // following the GenFitter.cc code
  // genfit::AbsKalmanFitter* fitter = 0;
  // if (m_useKalman) {
  genfit::KalmanFitterRefTrack* fitter = new genfit::KalmanFitterRefTrack();
  fitter->setMinIterations(3);
  fitter->setMaxIterations(10);
  fitter->setMultipleMeasurementHandling(genfit::unweightedClosestToPredictionWire);
  fitter->setMaxFailedHits(5);
  // } else {
  //   fitter = new genfit::DAF(true);
  //   ((genfit::DAF*) fitter)->setProbCut(0.001);
  //   fitter->setMaxFailedHits(5);
  // }
  try {
    if (!track->checkConsistency()) {
      B2DEBUG(50, "Inconsistent track found, attempting to sort!")
      bool sorted = track->sort();
      if (!sorted) {
        B2DEBUG(50, "Track points NOT SORTED! Still inconsistent, I can't be held responsible for assertion failure!");
      } else {
        B2DEBUG(50, "Track points SORTED!! Hopefully this works now!");
      }
    }
    fitter->processTrack(track);

    genfit::FitStatus* fs = 0;
    genfit::KalmanFitStatus* kfs = 0;
    fitSuccess = track->hasFitStatus();
    if (fitSuccess) {
      fs = track->getFitStatus();
      fitSuccess = fitSuccess && fs->isFitted();
      fitSuccess = fitSuccess && fs->isFitConverged();
      kfs = dynamic_cast<genfit::KalmanFitStatus*>(fs);
      fitSuccess = fitSuccess && kfs;
      B2DEBUG(100, "> Track fit. isFitted: " << fs->isFitted() << " isFitConverged:" << fs->isFitConverged() << " isFitConverged(false):"
              << fs->isFitConverged(false) << " nFailedPoints:" << fs->getNFailedPoints() << " numPointsWithMeasurement (from track): " <<
              track->getNumPointsWithMeasurement());
    } else {
      B2WARNING("Bad Fit in CKFCdcToVxdModule");
    }
  } catch (...) {
    B2WARNING("Track fitting has failed.");
  }
  delete fitter;
  return fitSuccess;
}
