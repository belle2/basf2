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
#include <klm/dataobjects/bklm/BKLMHit2d.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;
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
bool BKLMTrackFinder::filter(const std::list<BKLMHit2d*>& seed,
                             std::list<BKLMHit2d*>& hits,
                             std::list<BKLMHit2d*>& track)
{

  std::list<BKLMHit2d*>::iterator i;

  track = seed;

  if (m_Fitter == 0) {
    B2ERROR("BKLMTrackFinder: Fitter not registered");
    return (false);
  }

  m_Fitter->fit(track);//fit seed

  for (i = hits.begin(); i != hits.end(); ++i) {

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
      double error, sigma;
      if (m_globalFit)
        m_Fitter->globalDistanceToHit(*i, error, sigma);
      else m_Fitter->distanceToHit(*i, error, sigma);
      //B2INFO("BKLMTrackFinder" << " Error: " << error << " Sigma: " << sigma);
      if (sigma < 5.0) {
        track.push_back(*i);
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
