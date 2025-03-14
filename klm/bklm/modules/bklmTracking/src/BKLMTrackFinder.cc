/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/bklm/modules/bklmTracking/BKLMTrackFinder.h>

/* KLM headers. */
#include <klm/dataobjects/KLMHit2d.h>

/* Basf2 headers. */
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace CLHEP;

//! Default constructor
BKLMTrackFinder::BKLMTrackFinder()
{
}

BKLMTrackFinder::BKLMTrackFinder(BKLMTrackFitter* fitter) :
  m_Fitter(fitter),
  m_globalFit(false)
{
}

//! Destructor
BKLMTrackFinder::~BKLMTrackFinder()
{
}

//! Register a fitter if not constructed with one
void BKLMTrackFinder::registerFitter(BKLMTrackFitter* fitter)
{
  m_Fitter = fitter;
  m_Fitter->setGlobalFit(m_globalFit);
}

//! find associated hits and do fit
bool BKLMTrackFinder::filter(const std::list<KLMHit2d*>& seed,
                             std::list<KLMHit2d*>& hits,
                             std::list<KLMHit2d*>& track)
{
  track = seed;

  if (m_Fitter == 0) {
    B2ERROR("BKLMTrackFinder: Fitter not registered");
    return (false);
  }

  m_Fitter->fit(track);//fit seed

  for (KLMHit2d* hit2d : hits) {
    if (hit2d->getSubdetector() != KLMElementNumbers::c_BKLM)
      continue;

    // Prevent duplicate hits or hits on same layer
    // no duplicate hit is already guaranteed and now we allow hits on same layer so the following is commented out
    // bool skip = false;
    // for (j = track.begin(); j != track.end(); ++j) {
    //  if ((*j)->getLayer() == (*i)->getLayer())
    //    skip = true;
    // }
    // if (skip == true)
    //   continue;

    if (hit2d->isOnStaTrack() == false) {
      double error, sigma;
      if (m_globalFit)
        m_Fitter->globalDistanceToHit(hit2d, error, sigma);
      else m_Fitter->distanceToHit(hit2d, error, sigma);
      //B2INFO("BKLMTrackFinder" << " Error: " << error << " Sigma: " << sigma);
      if (sigma < 5.0) {
        track.push_back(hit2d);
      }
    }
  }

  if (track.size() < 3)
    return false;

  // Fit with new hits
  double chisqr = m_Fitter->fit(track);
  B2DEBUG(20, "BKLMTrackFinder:" << "ChiSqr: " << chisqr);

  // Do this the hard way to count each layer separately.
  std::list<int> hitLayers;
  for (KLMHit2d* hit2d : track) {
    hitLayers.push_back(hit2d->getLayer());
  }
  hitLayers.sort();
  hitLayers.unique();

  int layers = (*(--hitLayers.end()) - * (hitLayers.begin()));
  int noHits = hitLayers.size();

  if (noHits >= 4 && double(noHits) / double(layers) >= 0.75) {
    return true;
  } else {
    return false;
  }
}
