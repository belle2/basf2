/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <gtest/gtest.h>

#include <tracking/trackFindingCDC/testFixtures/TrackFindingCDCTestWithTopology.h>

#include <tracking/trackFindingCDC/filters/facet/FeasibleRLFacetFilter.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <framework/utilities/TestHelpers.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TEST_F(TrackFindingCDCTestWithTopology, filter_facet_FeasibleRLFacetFilter_accepts_long_para)
{
  FeasibleRLFacetFilter filter;

  filter.initialize();
  filter.beginRun();

  const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();

  const CDCWire& aWire = wireTopology.getWire(0, 0, 0);
  const CDCWire& bWire = wireTopology.getWire(0, 0, 1);
  const CDCWire& cWire = wireTopology.getWire(0, 0, 3);

  const CDCWireHit aWireHit(aWire.getWireID(), 0.1);
  const CDCWireHit bWireHit(bWire.getWireID(), 0.1);
  const CDCWireHit cWireHit(cWire.getWireID(), 0.1);

  filter.beginEvent();

  std::vector<std::array<ERightLeft, 3> > acceptableRLs = {
    { ERightLeft::c_Right, ERightLeft::c_Right, ERightLeft::c_Right,},
    { ERightLeft::c_Right, ERightLeft::c_Right, ERightLeft::c_Left,},
    { ERightLeft::c_Left, ERightLeft::c_Right, ERightLeft::c_Right,},
    { ERightLeft::c_Left, ERightLeft::c_Left, ERightLeft::c_Left,},
    { ERightLeft::c_Left, ERightLeft::c_Left, ERightLeft::c_Right,},
    { ERightLeft::c_Right, ERightLeft::c_Left, ERightLeft::c_Left,},
  };

  for (const std::array<ERightLeft, 3>& acceptableRL : acceptableRLs) {

    TEST_CONTEXT("For rl combination " << static_cast<int>(acceptableRL[0]) << ", "
                 << static_cast<int>(acceptableRL[1]) << ", "
                 << static_cast<int>(acceptableRL[2]));
    CDCRLWireHit aRLWireHit(&aWireHit, acceptableRL[0]);
    CDCRLWireHit bRLWireHit(&bWireHit, acceptableRL[1]);
    CDCRLWireHit cRLWireHit(&cWireHit, acceptableRL[2]);
    CDCFacet facet(aRLWireHit, bRLWireHit, cRLWireHit);
    Weight weight = filter(facet);
    EXPECT_TRUE(not std::isnan(weight));
  }

  filter.endRun();
  filter.terminate();
}
