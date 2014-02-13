/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "framework/datastore/StoreArray.h"
#include "skim/hlt/dataobjects/L3Tag.h"
//#include "skim/hlt/dataobjects/L3Track.h"
#include "skim/hlt/modules/level3/FTFinder.h"
#include "skim/hlt/modules/level3/FTTrack.h"
#include "skim/hlt/modules/level3/L3TrackTrigger.h"
#include <cmath>

using namespace Belle2;

L3TrackTrigger::L3TrackTrigger()
  : m_drCut(0.), m_dzCut(0.), m_ptCut(0.), m_minNGoodTrks(0)
{
  SetName("TrackTrigger");
}

bool
L3TrackTrigger::select(L3Tag* tag)
{
  int nGoodTrks(0);
  FTList<FTTrack*>& tracks = FTFinder::instance().getTracks();
  //StoreArray<L3Track> tracks;
  const int n = tracks.length();
  //const int n = tracks.getEntries();
  for (int i = 0; i < n; i++) {
    const FTTrack& t = *tracks[i];
    //L3Track& t = *tracks[i];
    const double dr = t.getDr();
    const double dz = t.getDz();
    const double pt = (t.getKappa() == 0.) ? 0. : 1. / fabs(t.getKappa());
    if (fabs(dr) < m_drCut && fabs(dz) < m_dzCut && pt > m_ptCut) nGoodTrks++;
  }
  if (tag) tag->setNTracks(nGoodTrks);

  return nGoodTrks >= m_minNGoodTrks;
}
