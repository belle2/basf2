#include <mdst/dataobjects/HitPatternVXD.h>
#include <gtest/gtest.h>

#include <bitset>
#include <string>

namespace Belle2 {
  /** */
  class HitPatternVXDTest : public ::testing::Test {
  protected:
  };

  /** Constructor and some general getters. */
  TEST_F(HitPatternVXDTest, General)
  {
    unsigned int initValue = 16776960;
    std::bitset<32> initValueAsBitset(static_cast<std::string>("00000000111111111111111100000000"));

    HitPatternVXD myHitPattern(initValue);
    EXPECT_EQ(initValueAsBitset, myHitPattern.getBitset());
    EXPECT_EQ(initValue, myHitPattern.getInteger());

  }

  /** Test simple Setters and Getters for SVD. */
  TEST_F(HitPatternVXDTest, SVDSetterAndGetter)
  {
    HitPatternVXD myHitPattern(0);

    for (unsigned int svdLayer = 0; svdLayer <= 3; ++svdLayer) {
      unsigned short uHits = 2;
      unsigned short vHits = 1;
      myHitPattern.setSVDLayer(svdLayer, uHits, vHits);
      EXPECT_EQ(uHits, myHitPattern.getSVDLayer(svdLayer).first);
      EXPECT_EQ(vHits, myHitPattern.getSVDLayer(svdLayer).second);
      myHitPattern.resetSVDLayer(svdLayer);
      EXPECT_EQ(0, myHitPattern.getSVDLayer(svdLayer).first);
      EXPECT_EQ(0, myHitPattern.getSVDLayer(svdLayer).second);
      uHits = 4;
      vHits = 4;
      myHitPattern.setSVDLayer(svdLayer, uHits, vHits);
      EXPECT_EQ(3, myHitPattern.getSVDLayer(svdLayer).first);
      EXPECT_EQ(3, myHitPattern.getSVDLayer(svdLayer).second);
    }
  }

  /** Test simple Setters and Getters for the PXD. */
  TEST_F(HitPatternVXDTest, PXDSetterAndGetter)
  {
    HitPatternVXD myHitPattern(0);

    for (unsigned int pxdLayer = 0; pxdLayer <= 1; ++pxdLayer) {
      for (unsigned int mode = 0; mode <= 1; ++mode) {
        for (unsigned int nHits = 0; nHits <= 3; ++nHits) {
          myHitPattern.setPXDLayer(pxdLayer, nHits, mode);
          EXPECT_EQ(nHits, myHitPattern.getPXDLayer(pxdLayer, mode));
          myHitPattern.resetPXDLayer(pxdLayer, mode);
          EXPECT_EQ(0, myHitPattern.getPXDLayer(pxdLayer, mode));
        }
      }
    }

    for (unsigned int pxdLayer = 0; pxdLayer <= 1; ++pxdLayer) {
      for (unsigned int mode = 0; mode <= 1; ++mode) {
        unsigned int nHits = 4;
        myHitPattern.setPXDLayer(pxdLayer, nHits, mode);
        EXPECT_EQ(3, myHitPattern.getPXDLayer(pxdLayer, mode));
        myHitPattern.resetPXDLayer(pxdLayer, mode);
        EXPECT_EQ(0, myHitPattern.getPXDLayer(pxdLayer, mode));
      }
    }

  }

  /** Test total number of hit getters. */
  TEST_F(HitPatternVXDTest, NGetters)
  {
    HitPatternVXD myHitPattern(16777215);
    const unsigned short svdHits = 24;
    const unsigned short pxdHits = 12;
    const unsigned short ndf = 2 * pxdHits + svdHits;
    EXPECT_EQ(svdHits, myHitPattern.getNSVDHits());
    EXPECT_EQ(pxdHits, myHitPattern.getNPXDHits());
    EXPECT_EQ(ndf, myHitPattern.getNdf());

  }

}
