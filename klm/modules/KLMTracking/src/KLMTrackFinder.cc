/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/modules/KLMTracking/KLMTrackFinder.h>

/* KLM headers. */
#include <klm/dataobjects/KLMHit2d.h>

/* Basf2 headers. */
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace CLHEP;

//! Default constructor
KLMTrackFinder::KLMTrackFinder()
{
}

KLMTrackFinder::KLMTrackFinder(KLMTrackFitter* fitter) :
  m_Fitter(fitter),
{
}

//! Destructor
KLMTrackFinder::~KLMTrackFinder()
{
}

//! Register a fitter if not constructed with one
void KLMTrackFinder::registerFitter(KLMTrackFitter* fitter)
{
  m_Fitter = fitter;
}

//! find associated hits and do fit
bool KLMTrackFinder::filter(const std::list<KLMHit2d*>& seed,
                            std::list<KLMHit2d*>& hits,
                            std::list<KLMHit2d*>& track)
{

  std::list<KLMHit2d*>::iterator i;

  track = seed;

  if (m_Fitter == 0) {
    B2ERROR("KLMTrackFinder: Fitter not registered");
    return (false);
  }

  m_Fitter->fit(track);//fit seed

  //TODO: REMOVE ME AFTER TESTING B-KLMTrackFitter
  // Will also need to think about how to generalize this
  for (i = hits.begin(); i != hits.end(); ++i) {
    if ((*i)->getSubdetector() != KLMElementNumbers::c_BKLM) //should be removed
      continue;

    // Prevent duplicate hits or hits on same layer
    // no duplicate hit is alreday guaranteed and now we allow hits on same layer so the following is commented out
    // bool skip = false;
    // for (j = track.begin(); j != track.end(); ++j) {
    //  if ((*j)->getLayer() == (*i)->getLayer())
    //    skip = true;
    // }
    // if (skip == true)
    //   continue;

    if ((*i)->isOnStaTrack() == false) {
      double distance, error, sigma;
      distance = m_Fitter->globalDistanceToHit(*i, error, sigma);
      B2DEBUG("KLMTrackFinder" << " Error: " << error << " Sigma: " << sigma << " Distance: " << distance);
      if (sigma < 5.0) {
        track.push_back(*i);
      }
    }
  }

  if (track.size() < 3)
    return false;

  // Fit with new hits
  double chisqr = m_Fitter->fit(track);
  B2DEBUG(20, "KLMTrackFinder:" << "ChiSqr: " << chisqr);

  // Do this the hard way to count each layer separately.
  std::list<int> hitLayers;
  for (i = track.begin(); i != track.end(); ++i) {
    hitLayers.push_back((*i)->getLayer());
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
