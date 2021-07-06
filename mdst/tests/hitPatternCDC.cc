/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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
    EXPECT_EQ(initValue, hitPatternCDC.getInteger());

    HitPatternCDC hitPatternCDC2;
    EXPECT_EQ(0,  hitPatternCDC2.getNHits());

    // Test setting and getting individual bits
    short layer = 5;
    hitPatternCDC2.setLayer(layer);
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

  /// This tests if the total number of hits is set and read correctly.
  TEST_F(HitPatternCDCTest, SetNGetTotalNumberOfHits)
  {
    unsigned long long int initValue = static_cast<unsigned long long int>(0xAAAAAAAAAAAAAAAAL); //101010...101010
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

  /// Test if the longest run in a superlayer is returned correctly.
  TEST_F(HitPatternCDCTest, getLongestContRunInSL)
  {
    unsigned long long int initValue = static_cast<unsigned long long int>(0xEEEEEEEEEEEEEEL); // 11101110...11101110
    HitPatternCDC myHitPatternCDC(initValue);
    EXPECT_EQ(3, myHitPatternCDC.getLongestContRunInSL(0));
    EXPECT_EQ(3, myHitPatternCDC.getLongestContRunInSL(1));
    EXPECT_EQ(3, myHitPatternCDC.getLongestContRunInSL(2));
    EXPECT_EQ(3, myHitPatternCDC.getLongestContRunInSL(3));
    EXPECT_EQ(3, myHitPatternCDC.getLongestContRunInSL(4));
    EXPECT_EQ(3, myHitPatternCDC.getLongestContRunInSL(5));
    EXPECT_EQ(3, myHitPatternCDC.getLongestContRunInSL(6));
    EXPECT_EQ(3, myHitPatternCDC.getLongestContRunInSL(7));
    EXPECT_EQ(3, myHitPatternCDC.getLongestContRunInSL(8));

  }

  /// Test if getFirst/LastLayer returns the proper value.
  TEST_F(HitPatternCDCTest, getFirstLastLayer)
  {
    HitPatternCDC myHitPatternCDC(0);
    myHitPatternCDC.setNHits(123); //arbitrary number to check if this layer is masked out correctly

    unsigned short firstLayer = 5;
    unsigned short lastLayer = 46;
    myHitPatternCDC.setLayer(firstLayer);
    myHitPatternCDC.setLayer(lastLayer);
    EXPECT_EQ(firstLayer, myHitPatternCDC.getFirstLayer());
    EXPECT_EQ(lastLayer, myHitPatternCDC.getLastLayer());

    myHitPatternCDC.resetPattern();
    EXPECT_EQ(-1, myHitPatternCDC.getFirstLayer());
    EXPECT_EQ(-1, myHitPatternCDC.getLastLayer());
  }

} // namespace Belle2
