/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <gtest/gtest.h>
#include <framework/utilities/TestHelpers.h>
#include <tracking/trackFindingCDC/testFixtures/TrackFindingCDCTestWithTopology.h>

#include <tracking/trackFindingCDC/filters/track/BasicTrackVarSet.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/WireID.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>

#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;

TEST(BasicTrackVarSetCalculation, test_empty_track)
{
  // Define mean variables to be 0 for empty track. Default of boost accumulator library is NAN, but
  // that the BDT cannot train on. Should be converted to 0 via the toFinite function.

  // create an empty track containing no hits
  const CDCTrack emptyTrack;
  // extract variables from emtpy track
  BasicTrackVarSet trackVarSet;

  // returnvalue should be false
  bool returnvalue = trackVarSet.extract(&emptyTrack);
  EXPECT_EQ(false, returnvalue);

  // for empty tracks the extraction calculations are not performed
  // thus all variables should be 0
  EXPECT_EQ(0, *trackVarSet.find("size"));
  EXPECT_EQ(0, *trackVarSet.find("pt"));

  EXPECT_EQ(0, *trackVarSet.find("drift_length_mean"));
  EXPECT_EQ(0, *trackVarSet.find("drift_length_sum"));
  EXPECT_EQ(0, *trackVarSet.find("drift_length_min"));
  EXPECT_EQ(0, *trackVarSet.find("drift_length_max"));
  EXPECT_EQ(0, *trackVarSet.find("drift_length_variance"));

  EXPECT_EQ(0, *trackVarSet.find("adc_mean"));
  EXPECT_EQ(0, *trackVarSet.find("adc_sum"));
  EXPECT_EQ(0, *trackVarSet.find("adc_min"));
  EXPECT_EQ(0, *trackVarSet.find("adc_max"));
  EXPECT_EQ(0, *trackVarSet.find("adc_variance"));

  EXPECT_EQ(0, *trackVarSet.find("empty_s_mean"));
  EXPECT_EQ(0, *trackVarSet.find("empty_s_sum"));
  EXPECT_EQ(0, *trackVarSet.find("empty_s_min"));
  EXPECT_EQ(0, *trackVarSet.find("empty_s_max"));
  EXPECT_EQ(0, *trackVarSet.find("empty_s_variance"));
  EXPECT_EQ(0, *trackVarSet.find("s_range"));
}

TEST_F(TrackFindingCDCTestWithTopology, basicTrackVarSet_test_one_hit_track)
{
  // create a dummy track with only one hit
  const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();

  const WireID& aWireID = wireTopology.getWire(0, 0, 0).getWireID();
  double driftLength = 2.0;
  unsigned short adcCount = 512.0;
  double arcLength2D = 0.3;
  const CDCHit aHit(128, adcCount, aWireID);
  const CDCWireHit aWireHit(&aHit, driftLength);
  const CDCRLWireHit aRLWireHit(&aWireHit, ERightLeft::c_Unknown);
  double some_arbitrary_z_coord = 0.0;
  const Vector3D aRecoPos(aRLWireHit.getRefPos2D(), some_arbitrary_z_coord);
  const CDCRecoHit3D aCDCHit(aRLWireHit, aRecoPos, arcLength2D);
  const std::vector<CDCRecoHit3D> aHitVector{aCDCHit};
  const CDCTrack track(aHitVector);

  // extrack variables from track
  BasicTrackVarSet trackVarSet;

  trackVarSet.extract(&track);

  EXPECT_EQ(1, *trackVarSet.find("size"));

  EXPECT_EQ(driftLength, *trackVarSet.find("drift_length_mean"));
  EXPECT_EQ(driftLength, *trackVarSet.find("drift_length_sum"));
  EXPECT_EQ(driftLength, *trackVarSet.find("drift_length_min"));
  EXPECT_EQ(driftLength, *trackVarSet.find("drift_length_max"));
  // variance not calculatable, define it to be -1
  EXPECT_EQ(-1, *trackVarSet.find("drift_length_variance"));

  EXPECT_EQ(adcCount, *trackVarSet.find("adc_mean"));
  EXPECT_EQ(adcCount, *trackVarSet.find("adc_sum"));
  EXPECT_EQ(adcCount, *trackVarSet.find("adc_min"));
  EXPECT_EQ(adcCount, *trackVarSet.find("adc_max"));
  // variance not calculatable, define it to be -1
  EXPECT_EQ(-1, *trackVarSet.find("adc_variance"));

  // with only one arc length (s), there are no gaps in s, thus empty_s variables empty
  EXPECT_EQ(0, *trackVarSet.find("empty_s_mean"));
  EXPECT_EQ(0, *trackVarSet.find("empty_s_sum"));
  EXPECT_EQ(0, *trackVarSet.find("empty_s_min"));
  EXPECT_EQ(0, *trackVarSet.find("empty_s_max"));
  EXPECT_EQ(0, *trackVarSet.find("s_range"));
  // variance not calculatable, define it to be -1
  EXPECT_EQ(-1, *trackVarSet.find("empty_s_variance"));
}

TEST_F(TrackFindingCDCTestWithTopology, basicTrackVarSet_test_two_hit_track)
{

  // create a dummy track with only one hit
  const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();

  const WireID& aWireID = wireTopology.getWire(0, 0, 0).getWireID();
  const WireID& bWireID = wireTopology.getWire(0, 0, 1).getWireID();
  std::vector<WireID> wireIDs = {aWireID, bWireID};

  std::vector<double> drift_lengths = {2.0, 3.0};
  // large ADC counts might cause errors in calculation due to short data type
  // for example when squaring to get the variance
  std::vector<unsigned short> adc_counts = {30, 500};
  std::vector<double> arc_length_2Ds = {2.0, -3.0};

  std::vector<CDCHit> cdcHits;
  std::vector<CDCWireHit> cdcWireHits;
  std::vector<CDCRecoHit3D> cdcRecoHits;

  for (size_t i = 0; i < drift_lengths.size(); i++) {
    cdcHits.emplace_back(128, adc_counts.at(i), wireIDs.at(i)); // store in vector to avoid nullptrs
    cdcWireHits.emplace_back(&cdcHits.at(i), drift_lengths.at(i));
    CDCRLWireHit aRLWireHit(&cdcWireHits.at(i), ERightLeft::c_Unknown);
    Vector3D aRecoPos(aRLWireHit.getRefPos2D(), 0.0);
    cdcRecoHits.emplace_back(aRLWireHit, aRecoPos, arc_length_2Ds.at(i));
  }

  const CDCTrack track(cdcRecoHits);

  // extrack variables from track
  BasicTrackVarSet trackVarSet;
  trackVarSet.extract(&track);

  EXPECT_EQ(2, *trackVarSet.find("size"));

  EXPECT_EQ(2.5, *trackVarSet.find("drift_length_mean"));
  EXPECT_EQ(5.0, *trackVarSet.find("drift_length_sum"));
  EXPECT_EQ(2.0, *trackVarSet.find("drift_length_min"));
  EXPECT_EQ(3.0, *trackVarSet.find("drift_length_max"));
  // variance from python: np.sqrt(np.var([2, 3])*2/(2-1))
  EXPECT_NEAR(0.7071, *trackVarSet.find("drift_length_variance"), 1e-4);

  EXPECT_EQ(265, *trackVarSet.find("adc_mean"));
  EXPECT_EQ(530, *trackVarSet.find("adc_sum"));
  EXPECT_EQ(30, *trackVarSet.find("adc_min"));
  EXPECT_EQ(500, *trackVarSet.find("adc_max"));
  EXPECT_NEAR(332.34, *trackVarSet.find("adc_variance"), 1e-2);

  // for empty_s, there is only one hit gap with two hits
  double only_empty_s = -5.0;
  EXPECT_EQ(only_empty_s, *trackVarSet.find("empty_s_mean"));
  EXPECT_EQ(only_empty_s, *trackVarSet.find("empty_s_sum"));
  EXPECT_EQ(only_empty_s, *trackVarSet.find("empty_s_min"));
  EXPECT_EQ(only_empty_s, *trackVarSet.find("empty_s_max"));
  EXPECT_EQ(only_empty_s, *trackVarSet.find("s_range"));
  // variance not calculatable
  EXPECT_EQ(-1, *trackVarSet.find("empty_s_variance"));
}

TEST_F(TrackFindingCDCTestWithTopology, basicTrackVarSet_test_empty_s_for_three_hit_track)
{
  // Just test the empty_s calculations also with three hits, which result in two empty_s values
  const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();

  const WireID& aWireID = wireTopology.getWire(0, 0, 0).getWireID();
  const WireID& bWireID = wireTopology.getWire(0, 0, 1).getWireID();
  const WireID& cWireID = wireTopology.getWire(0, 0, 2).getWireID();
  std::vector<WireID> wireIDs = {aWireID, bWireID, cWireID};

  // -> empty_s hit gaps = {2, 3}
  std::vector<double> arc_length_2Ds = { -2.0, 0.0, 3.0};

  std::vector<CDCHit> cdcHits;
  cdcHits.reserve(wireIDs.size());
  std::vector<CDCWireHit> cdcWireHits;
  cdcWireHits.reserve(wireIDs.size());
  std::vector<CDCRecoHit3D> cdcRecoHits;
  cdcRecoHits.reserve(wireIDs.size());

  for (const WireID& wireID : wireIDs) {
    cdcHits.emplace_back(128, 0, wireID);
  }
  for (const CDCHit& cdcHit : cdcHits) {
    cdcWireHits.emplace_back(&cdcHit, 0);
  }
  for (std::size_t i = 0; i < cdcWireHits.size(); i++) {
    const CDCRLWireHit aRLWireHit(&cdcWireHits.at(i), ERightLeft::c_Unknown);
    const Vector3D aRecoPos(aRLWireHit.getRefPos2D(), 0.0);
    cdcRecoHits.emplace_back(aRLWireHit, aRecoPos, arc_length_2Ds.at(i));
  }

  const CDCTrack track(cdcRecoHits);

  // extrack variables from track
  BasicTrackVarSet trackVarSet;
  trackVarSet.extract(&track);

  EXPECT_EQ(3, *trackVarSet.find("size"));

  EXPECT_EQ(2.5, *trackVarSet.find("empty_s_mean"));
  EXPECT_EQ(5.0, *trackVarSet.find("empty_s_sum"));
  EXPECT_EQ(2.0, *trackVarSet.find("empty_s_min"));
  EXPECT_EQ(3.0, *trackVarSet.find("empty_s_max"));
  EXPECT_NEAR(0.7071, *trackVarSet.find("empty_s_variance"), 1e-4);
}
