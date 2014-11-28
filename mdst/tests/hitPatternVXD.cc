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

  /** Test the getNLayer functions */
  TEST_F(HitPatternVXDTest, getNLayers)
  {
    HitPatternVXD myHitPattern(0);
    const unsigned short nSVD = 3;
    const unsigned short nPXDnormal = 2;
    const unsigned short nPXDgated = 1;
    const unsigned short normalMode = 0;
    const unsigned short gatedMode = 1;
    // set SVD
    myHitPattern.setSVDLayer(0, 1, 3);
    myHitPattern.setSVDLayer(1, 0, 0);
    myHitPattern.setSVDLayer(2, 1, 1);
    myHitPattern.setSVDLayer(3, 0, 1);
    // set PXD normal
    myHitPattern.setPXDLayer(0, 1, normalMode);
    myHitPattern.setPXDLayer(1, 2, normalMode);
    // set PXD gated
    myHitPattern.setPXDLayer(0, 0, gatedMode);
    myHitPattern.setPXDLayer(1, 2, gatedMode);

    EXPECT_EQ(nSVD, myHitPattern.getNSVDLayers());
    EXPECT_EQ(nPXDnormal, myHitPattern.getNPXDLayers(normalMode));
    EXPECT_EQ(nPXDgated, myHitPattern.getNPXDLayers(gatedMode));
    EXPECT_EQ(nSVD + nPXDnormal, myHitPattern.getNVXDLayers(normalMode));
    EXPECT_EQ(nSVD + nPXDgated, myHitPattern.getNVXDLayers(gatedMode));

  }

  /** Test the getters for First/Last Layer. */
  TEST_F(HitPatternVXDTest, getFirstLastLayer)
  {
    HitPatternVXD myHitPattern(0);
    // random choices
    const unsigned short firstLayerPXD = 0;
    const unsigned short lastLayerPXD = 1;
    const unsigned short firstLayerSVD = 1;
    const unsigned short lastLayerSVD = 3;

    const unsigned short normalMode = 0;
    const unsigned short gatedMode = 1;
    // set SVD
    myHitPattern.setSVDLayer(0, 0, 0);
    myHitPattern.setSVDLayer(1, 3, 0);
    myHitPattern.setSVDLayer(2, 0, 1);
    myHitPattern.setSVDLayer(3, 1, 1);
    // set PXD normal
    myHitPattern.setPXDLayer(0, 1, normalMode);
    myHitPattern.setPXDLayer(1, 2, normalMode);
    // set PXD gated
    myHitPattern.setPXDLayer(0, 1, gatedMode);
    myHitPattern.setPXDLayer(1, 2, gatedMode);

    EXPECT_EQ(firstLayerSVD, myHitPattern.getFirstSVDLayer());
    EXPECT_EQ(lastLayerSVD, myHitPattern.getLastSVDLayer());
    EXPECT_EQ(firstLayerPXD, myHitPattern.getFirstPXDLayer(normalMode));
    EXPECT_EQ(lastLayerPXD, myHitPattern.getLastPXDLayer(normalMode));
    EXPECT_EQ(firstLayerPXD, myHitPattern.getFirstPXDLayer(gatedMode));
    EXPECT_EQ(lastLayerPXD, myHitPattern.getLastPXDLayer(gatedMode));
  }
}
