/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/hits/CDCDerivedGenHit.h>

#include <tracking/trackFindingCDC/test_fixtures/TrackFindingCDCTestWithTopology.h>

#include <type_traits>
#include <gtest/gtest.h>

using namespace std;

using namespace Belle2;
using namespace TrackFindingCDC;

// Test of the restrained base class idiom
namespace {
  template<class T>
  void unused(const T&) {}

  TEST_F(TrackFindingCDCTestWithTopology, eventdata_hits_CDCDerivedGenHit_methodAccess)
  {
    // CDCDerivedGenHit has all methods of its restrained base accessable with ->.
    CDCDerivedGenHit derivedGenHit;
    const Vector2D pos = derivedGenHit->getDummyPos2D();

    // Also all of its own methods are accessable with ->.
    derivedGenHit->setDummyPos2D(pos);
    derivedGenHit->setFlag(true);
    EXPECT_TRUE(derivedGenHit->getFlag());
  }

  TEST_F(TrackFindingCDCTestWithTopology, eventdata_hits_CDCDerivedGenHit_hiddenBase)
  {
    // But unlike normal inheritance you may never obtain a non constant reference of the base.
    CDCDerivedGenHit derivedGenHit;
    // Would not compile
    // CDCGenHit& genHit = derivedGenHit;

    // If it is okay the obtain a constant reference is under full controll of the developer.
    const CDCGenHit& genHit = derivedGenHit;
    unused(genHit);

    static_assert(std::is_constructible<const CDCGenHit&, CDCDerivedGenHit>::value,
                  "Can assign to the constant restrained base");

    static_assert(not std::is_constructible<CDCGenHit&, CDCDerivedGenHit&>::value,
                  "Cannot assign to the non constant restrained base");
  }
}
