#include <gtest/gtest.h>
#include <mdst/dataobjects/HitPatternCDC.h>

namespace Belle2 {
  /** Unit tests for the CDC hit Patterns.*/
  class HitPatternCDCTest : public ::testing::Test {
  protected:
  };

  /** Test simple Setters, Getters, and Hassers. */
  TEST_F(HitPatternCDCTest, settersNGetters)
  {
    // Test proper initialization.
    unsigned long initValue = 66560; // python >>> 2 ** 16 + 2 ** 10
    HitPatternCDC hitPatternCDC(initValue);
    EXPECT_EQ(2,  hitPatternCDC.getNHits());

    HitPatternCDC hitPatternCDC2;
    EXPECT_EQ(0,  hitPatternCDC2.getNHits());

    // Test setting and getting individual bits
    short layer = 5;
    hitPatternCDC2.setLayer(layer);
    EXPECT_EQ(1,    hitPatternCDC2.getNHits());
    EXPECT_EQ(true, hitPatternCDC2.hasLayer(5));

    // Test of SuperLayer getters; setters don't make sense.
    // Reminder: Start counting from 0 here.
    // Even -> Axial Layer; Odd -> Stereo Layer
    HitPatternCDC hitPatternCDC3;
    hitPatternCDC3.setLayer(9);   // SL 1
    hitPatternCDC3.setLayer(15);  // SL 2
    hitPatternCDC3.setLayer(21);  // SL 3

    EXPECT_FALSE(hitPatternCDC3.hasSLayer(0));
    EXPECT_TRUE(hitPatternCDC3.hasSLayer(1));

    // Test Axial and Stereo requests.
    EXPECT_TRUE(hitPatternCDC3.hasAxialLayer());
    EXPECT_TRUE(hitPatternCDC3.hasStereoLayer());
    EXPECT_FALSE(hitPatternCDC2.hasStereoLayer());

    hitPatternCDC3.resetLayer(15);
    EXPECT_FALSE(hitPatternCDC3.hasAxialLayer());

    // Test resetting of super layers.
    hitPatternCDC3.resetSLayer(1);
    hitPatternCDC3.resetSLayer(3);
    EXPECT_EQ(0, hitPatternCDC3.getNHits());
  } // settersNGetters

  /// This tests if the
  TEST_F(HitPatternCDCTest, SetNGetTotalNumberOfHits)
  {
    unsigned long int initValue = 12297829382473034410U; //101010...101010
    HitPatternCDC myHitPatternCDC(initValue);
    EXPECT_EQ(170, myHitPatternCDC.getNHits());
    unsigned short int nHits1 = 255;
    myHitPatternCDC.setNHits(nHits1);
    EXPECT_EQ(nHits1, myHitPatternCDC.getNHits());
    unsigned short int nHits2 = 0;
    myHitPatternCDC.setNHits(nHits2);
    EXPECT_EQ(nHits2, myHitPatternCDC.getNHits());
    unsigned short int nHits3 = 93;
    myHitPatternCDC.setNHits(nHits3);
    EXPECT_EQ(nHits3, myHitPatternCDC.getNHits());
  }

} // namespace Belle2
