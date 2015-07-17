/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackProcessor.h>
#include <tracking/trackFindingCDC/legendre/tests_fixtures/CDCLegendreTestFixture.h>

#include <vector>
#include <gtest/gtest.h>

#include <genfit/TrackCand.h>

using namespace std;

using namespace Belle2;
using namespace TrackFindingCDC;

class CDCLegendreTrackProcessor : public CDCLegendreTestFixture  {
protected:
  void SetUp() override
  {
    CDCLegendreTestFixture::SetUp();

    StoreArray<genfit::TrackCand> trackCands(m_track_cands_store_array_name);
    DataStore::Instance().setInitializeActive(true);
    trackCands.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);
  }

  void TearDown() override
  {
    DataStore::Instance().reset();
  }

  const std::string m_track_cands_store_array_name = "GFTrackCands";
};
