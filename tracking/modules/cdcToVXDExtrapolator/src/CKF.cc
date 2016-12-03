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
#include <tracking/modules/cdcToVXDExtrapolator/CKFPartialTrack.h>

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

#include<algorithm>

using namespace Belle2;

CKF::CKF(genfit::Track* _track, bool (*_findHits)(genfit::Track*, std::vector<CKFPartialTrack*>&, unsigned,
                                                  std::vector<genfit::AbsMeasurement*>&, void*),
         void* _data, double _maxChi2Increment, int _maxHoles, double _holePenalty, int _Nmax, double _hitMultiplier) :
  seedTrack(_track),
  findHits(_findHits),
  data(_data),
  step(0),
  maxHoles(_maxHoles),
  maxChi2Increment(_maxChi2Increment),
  hitMultiplier(_hitMultiplier),
  holePenalty(_holePenalty),
  Nmax(_Nmax),
  tracks(0)
{
}

genfit::Track* CKF::processTrack()
{
  tracks = new std::vector<CKFPartialTrack*>;
  tracks->push_back(new CKFPartialTrack(*seedTrack));

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
    if (!findHits(seedTrack, *tracks, step, newHits, data))
      break;

    if (newHits.size() == 0) {
      B2DEBUG(90, "-- In CKF::processTrack(): At step " << step << " no hits found, continuing search");
      for (auto& track : *tracks) {
        track->addHole();
      }
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
    std::vector<CKFPartialTrack*>* newtracks = new std::vector<CKFPartialTrack*>;

    for (auto& track : *tracks) {
      /// if the track is bad, don't try to add hits
      if (!passPreUpdateTrim(track, step)) {
        // copy the no-added-hit case
        track->addHole();
        newtracks->push_back(track);
        continue;
      }
      any = true;
      if (newHits.size() != 0) {
        for (auto& hit : newHits) {
          CKFPartialTrack* newtrack = new CKFPartialTrack(*track);
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
              newtrack->addHit(up0->getChiSquareIncrement() / up0->getNdf());

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

      // copy the no-added-hit case
      track->addHole();
      newtracks->push_back(track);
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
    orderTracksAndTrim(tracks);
  }

  B2DEBUG(90, "- In CKF::processTrack(): Finished search, have " << tracks->size() << " track extrapolations");
  CKFPartialTrack* best = bestTrack(tracks);
  genfit::Track* ret = 0;
  if (best)
    ret = new genfit::Track(*best);
  for (unsigned i = 0; i < tracks->size(); ++i)
    delete tracks->at(i);
  delete tracks;
  tracks = 0;
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

bool CKF::passPreUpdateTrim(CKFPartialTrack* track, unsigned step)
{
  // Check for holes
  int nadded = track->getNumPointsWithMeasurement() - seedTrack->getNumPointsWithMeasurement();
  int nholes = step - nadded;
  if (nholes != (int) track->totalHoles())
    B2WARNING("Number of holes from CKFPartialTrack: " << track->totalHoles() << ", differs from what I think it should be: " <<
              nholes);
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

void chiSqNdf(genfit::Track* track, double& chi2, int& ndf, double& chi2_ndof)
{
  chi2 = 0;
  ndf = 0;
  chi2_ndof = 0;
  for (unsigned i = 0; i < track->getNumPointsWithMeasurement(); ++i) {
    auto fi = track->getPointWithMeasurementAndFitterInfo(i, track->getCardinalRep());
    if (fi) {
      auto up = fi->getKalmanFitterInfo(0)->getUpdate(-1);
      chi2 += up->getChiSquareIncrement();
      ndf += up->getNdf();
      if (std::isnan(up->getChiSquareIncrement() / up->getNdf())) {
        B2DEBUG(40, "IsNAN: " << up->getChiSquareIncrement() << " " << up->getNdf());
        chi2_ndof += 1.;
      } else
        chi2_ndof += up->getChiSquareIncrement() / up->getNdf();
    } else {
      B2WARNING("FitterInfo missing in track!");
    }
  }
}

double CKF::quality(CKFPartialTrack* track)
{
  double thisChi2(0), thisChi2_Ndf(0); int ndf(0);
  chiSqNdf(track, thisChi2, ndf, thisChi2_Ndf);
  // thisChi2 += track->trueHoles() * holePenalty;
  // ndf += track->trueHoles();
  // thisChi2 = thisChi2 / ndf;
  // B2DEBUG(90, "----- In CKF::quailty(): @ " << " " << thisChi2);
  // return thisChi2;

  double Q = (hitMultiplier * (double) track->addedHits()) - (double) track->trueHoles() - (1.0f / holePenalty) * thisChi2_Ndf;
  B2DEBUG(90, "-- In CKF::quality(): @ " << " " << thisChi2 << " " << ndf << " " << thisChi2 / ndf <<
          " sum(chi2/ndf) " << thisChi2_Ndf <<
          " Hole Penalty: " << track->trueHoles() * holePenalty << " Hits: " << track->addedHits() << " Q " << Q);
  return Q;
}

bool CKF::preBestQuality(CKFPartialTrack* track)
{
  if (track->lowestChi2() > 1)
    return false;
  return true;
}

CKFPartialTrack* CKF::bestTrack(std::vector<CKFPartialTrack*>* tracks)
{
  if (!tracks || tracks->size() == 0) {
    B2DEBUG(90, "-- In CKF::bestTrack(): No tracks, no best.");
    return 0;
  }
  int iBest = 0;
  double QBest = quality(tracks->at(0));
  for (unsigned i = 1; i < tracks->size(); ++i) {
    double thisQ = quality(tracks->at(i));
    if (thisQ > QBest) {
      QBest = thisQ;
      iBest = i;
    }
  }
  B2DEBUG(90, "-- In CKF::bestTrack(): Best track @ " << iBest);

  return tracks->at(iBest);
}

void CKF::orderTracksAndTrim(std::vector<CKFPartialTrack*>*& tracks)
{
  std::sort(tracks->begin(), tracks->end(), [this](CKFPartialTrack * a, CKFPartialTrack * b) { return (quality(a) > quality(b)); });
  if ((int) tracks->size() <= Nmax) return;

  for (unsigned i = 0; i < tracks->size(); ++i) {
    double Q = quality(tracks->at(i));
    B2DEBUG(80, "---- orderingTracksAndTrimming: @ " << i << " " << Q);
    /// don't allow tracks without decent hits to continue
    if (!preBestQuality(tracks->at(i)))
      continue;
    if (Q > (hitMultiplier * tracks->at(i)->addedHits())) {
      double chi2(0), chi2_ndf(0); int ndf(0);
      chiSqNdf(tracks->at(i), chi2, ndf, chi2_ndf);
      B2WARNING("---- orderingTracksAndTrimming: BAD TRACK @ " << i << " " << Q << " " << chi2_ndf << " " <<
                tracks->at(i)->addedHits() << " " << chi2 << " " << ndf << " " << chi2 / ndf << " " <<
                tracks->at(i)->trueHoles() << " " << tracks->at(i)->totalHoles() << " ");
    }
  }
  for (unsigned i = Nmax; i < tracks->size(); ++i) {
    delete tracks->at(i);
  }
  tracks->erase(tracks->begin() + Nmax, tracks->end());
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
    try {
      track->checkConsistency();
    } catch (genfit::Exception& e) {
      B2DEBUG(50, e.getExcString());
      B2DEBUG(50, "Inconsistent track found, attempting to sort!");
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
