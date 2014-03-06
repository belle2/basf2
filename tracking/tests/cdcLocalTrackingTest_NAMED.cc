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

#include <tracking/cdcLocalTracking/tempro/Named.h>

using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;


// This file contains only compile time test.

TEST(CDCLocalTrackingTestWithoutGearBox, NAMED)
{

  typedef StaticString < 't', 'e', 's', 't' > StaticString_test;

  typedef StaticString < "test"[0], "test"[1], "test"[2], "test"[3] > StaticString_test2;

  typedef StaticString < Named::at("test", 0), Named::at("test", 1), Named::at("test", 2), Named::at("test", 3) > StaticString_test3;

  constexpr const char test[5] = "test";
  typedef StaticString<Named::at(test, 0), Named::at(test, 1), Named::at(test, 2), Named::at(test, 3)> StaticString_test4;

  // Inserts the sting itself so it gets expanded to something like similar to StaticString_test3;
  typedef NAMED("test") StaticString_test5;

  // Inserts the constexp variable so it gets expanded to something like similar to StaticString_test4;
  typedef NAMED(test) StaticString_test6;

  // But all definitions are equal
  ::testing::StaticAssertTypeEq< StaticString_test, StaticString_test2>();
  ::testing::StaticAssertTypeEq< StaticString_test, StaticString_test3>();
  ::testing::StaticAssertTypeEq< StaticString_test, StaticString_test4>();
  ::testing::StaticAssertTypeEq< StaticString_test, StaticString_test5>();
  ::testing::StaticAssertTypeEq< StaticString_test, StaticString_test6>();

}


TEST(CDCLocalTrackingTestWithoutGearBox, NAMED_Misaligned)
{
  typedef StaticString < 't', 'e', 's', 't', 's' > StaticString_tests;
  typedef NAMED("tests") StaticString_tests2;
  ::testing::StaticAssertTypeEq< StaticString_tests, StaticString_tests2>();
}

TEST(CDCLocalTrackingTestWithoutGearBox, NAMED_ToLong)
{

  static_assert(NAMED_MAXLENGTH == 64, "");
  /*
  typedef NAMED("0123456789" \
    "0123456789" \
    "0123456789" \
    "0123456789" \
    "0123456789" \
    "0123456789" \
    "0123456789" \
    "0123456789" \
    "0123456789" \
    "0123456789" \
    "0123456789" \
    "0123456789" \
    "0123456" \
    ) StaticString_does_not_compile;

  */


}

