#include <gtest/gtest.h>
#include <tracking/dataobjects/HitPatternCDC.h>

namespace Belle2 {
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** Set up a few arrays and objects in the datastore */
  class BitPatternCDCTest : public ::testing::Test {
  protected:
  };

  /** Test simple Setters and Getters. */
  TEST_F(BitPatternCDCTest, settersNGetters)
  {
    // Test proper initialization. ---------------------------------------------------------
    unsigned long initValue = 66560; // python >>> 2 ** 16 + 2 ** 10
    HitPatternCDC myPattern(initValue);
    EXPECT_EQ(2,  myPattern.getNHits());

    HitPatternCDC myPattern2;
    EXPECT_EQ(0,  myPattern2.getNHits());

    // Test setting and getting individual bits
    short layer = 5;
    myPattern2.setLayer(layer);
    EXPECT_EQ(1,  myPattern2.getNHits());
    EXPECT_EQ(1,  myPattern2.getLayer(layer));

    // Test of SuperLayer getters; setters don't make sense.
    HitPatternCDC bitPatternCDC;
    bitPatternCDC.setLayer(9);   // SL 2
    bitPatternCDC.setLayer(15);  // SL 3
    bitPatternCDC.setLayer(21);  // SL 4

    EXPECT_FALSE(bitPatternCDC.getSLayer(0));
    EXPECT_TRUE(bitPatternCDC.getSLayer(1));

  }
}
