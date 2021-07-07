/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <mdst/dataobjects/HitPatternVXD.h>
#include <gtest/gtest.h>

#include <bitset>
#include <string>

namespace Belle2 {
  /** */
  class HitPatternVXDTest : public ::testing::Test {
  protected:
  };

  /** Test the numbering scheme in the static variables. */
  TEST_F(HitPatternVXDTest, NumberingScheme)
  {
    const std::list<unsigned short> pxdNumbering = {1, 2};
    const std::list<unsigned short> svdNumbering = {3, 4, 5, 6};
    EXPECT_EQ(pxdNumbering, HitPatternVXD::s_PXDLayerNumbers);
    EXPECT_EQ(svdNumbering, HitPatternVXD::s_SVDLayerNumbers);
  }

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

    for (const auto layerId : HitPatternVXD::s_SVDLayerNumbers) {
      unsigned short uHits = 2;
      unsigned short vHits = 1;
      myHitPattern.setSVDLayer(layerId, uHits, vHits);
      EXPECT_EQ(uHits, myHitPattern.getSVDLayer(layerId).first);
      EXPECT_EQ(vHits, myHitPattern.getSVDLayer(layerId).second);
      myHitPattern.resetSVDLayer(layerId);
      EXPECT_EQ(0, myHitPattern.getSVDLayer(layerId).first);
      EXPECT_EQ(0, myHitPattern.getSVDLayer(layerId).second);
      uHits = 4;
      vHits = 4;
      myHitPattern.setSVDLayer(layerId, uHits, vHits);
      EXPECT_EQ(3, myHitPattern.getSVDLayer(layerId).first);
      EXPECT_EQ(3, myHitPattern.getSVDLayer(layerId).second);
    }
  }

  /** Test simple Setters and Getters for the PXD. */
  TEST_F(HitPatternVXDTest, PXDSetterAndGetter)
  {
    HitPatternVXD myHitPattern(0);

    for (const auto layerId : HitPatternVXD::s_PXDLayerNumbers) {
      for (unsigned int nHits = 0; nHits <= 3; ++nHits) {
        myHitPattern.setPXDLayer(layerId, nHits, HitPatternVXD::PXDMode::normal);
        EXPECT_EQ(nHits, myHitPattern.getPXDLayer(layerId, HitPatternVXD::PXDMode::normal));
        myHitPattern.resetPXDLayer(layerId, HitPatternVXD::PXDMode::normal);
        EXPECT_EQ(0, myHitPattern.getPXDLayer(layerId, HitPatternVXD::PXDMode::normal));

        myHitPattern.setPXDLayer(layerId, nHits, HitPatternVXD::PXDMode::gated);
        EXPECT_EQ(nHits, myHitPattern.getPXDLayer(layerId, HitPatternVXD::PXDMode::gated));
        myHitPattern.resetPXDLayer(layerId, HitPatternVXD::PXDMode::gated);
        EXPECT_EQ(0, myHitPattern.getPXDLayer(layerId, HitPatternVXD::PXDMode::gated));
      }
    }

    for (const auto layerId : HitPatternVXD::s_PXDLayerNumbers) {
      unsigned int nHits = 4;
      myHitPattern.setPXDLayer(layerId, nHits, HitPatternVXD::PXDMode::normal);
      EXPECT_EQ(3, myHitPattern.getPXDLayer(layerId, HitPatternVXD::PXDMode::normal));
      myHitPattern.resetPXDLayer(layerId, HitPatternVXD::PXDMode::normal);
      EXPECT_EQ(0, myHitPattern.getPXDLayer(layerId, HitPatternVXD::PXDMode::normal));

      myHitPattern.setPXDLayer(layerId, nHits, HitPatternVXD::PXDMode::gated);
      EXPECT_EQ(3, myHitPattern.getPXDLayer(layerId, HitPatternVXD::PXDMode::gated));
      myHitPattern.resetPXDLayer(layerId, HitPatternVXD::PXDMode::gated);
      EXPECT_EQ(0, myHitPattern.getPXDLayer(layerId, HitPatternVXD::PXDMode::gated));
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

    // set SVD
    myHitPattern.setSVDLayer(3, 1, 3);
    myHitPattern.setSVDLayer(4, 0, 0);
    myHitPattern.setSVDLayer(5, 1, 1);
    myHitPattern.setSVDLayer(6, 0, 1);
    // set PXD normal
    myHitPattern.setPXDLayer(1, 1, HitPatternVXD::PXDMode::normal);
    myHitPattern.setPXDLayer(2, 2, HitPatternVXD::PXDMode::normal);
    // set PXD gated
    myHitPattern.setPXDLayer(1, 0, HitPatternVXD::PXDMode::gated);
    myHitPattern.setPXDLayer(2, 2, HitPatternVXD::PXDMode::gated);

    EXPECT_EQ(nSVD, myHitPattern.getNSVDLayers());
    EXPECT_EQ(nPXDnormal, myHitPattern.getNPXDLayers(HitPatternVXD::PXDMode::normal));
    EXPECT_EQ(nPXDgated, myHitPattern.getNPXDLayers(HitPatternVXD::PXDMode::gated));
    EXPECT_EQ(nSVD + nPXDnormal, myHitPattern.getNVXDLayers(HitPatternVXD::PXDMode::normal));
    EXPECT_EQ(nSVD + nPXDgated, myHitPattern.getNVXDLayers(HitPatternVXD::PXDMode::gated));

  }

  /** Test the getters for First/Last Layer. */
  TEST_F(HitPatternVXDTest, getFirstLastLayer)
  {
    HitPatternVXD myHitPattern(0);
    // random choices
    const unsigned short firstLayerPXD = 1;
    const unsigned short lastLayerPXD = 2;
    const unsigned short firstLayerSVD = 4;
    const unsigned short lastLayerSVD = 6;

    // set SVD
    myHitPattern.setSVDLayer(3, 0, 0);
    myHitPattern.setSVDLayer(4, 3, 0);
    myHitPattern.setSVDLayer(5, 0, 1);
    myHitPattern.setSVDLayer(6, 1, 1);
    // set PXD normal
    myHitPattern.setPXDLayer(1, 1, HitPatternVXD::PXDMode::normal);
    myHitPattern.setPXDLayer(2, 2, HitPatternVXD::PXDMode::normal);
    // set PXD gated
    myHitPattern.setPXDLayer(1, 1, HitPatternVXD::PXDMode::gated);
    myHitPattern.setPXDLayer(2, 2, HitPatternVXD::PXDMode::gated);

    EXPECT_EQ(firstLayerSVD, myHitPattern.getFirstSVDLayer());
    EXPECT_EQ(lastLayerSVD, myHitPattern.getLastSVDLayer());
    EXPECT_EQ(firstLayerPXD, myHitPattern.getFirstPXDLayer(HitPatternVXD::PXDMode::normal));
    EXPECT_EQ(lastLayerPXD, myHitPattern.getLastPXDLayer(HitPatternVXD::PXDMode::normal));
    EXPECT_EQ(firstLayerPXD, myHitPattern.getFirstPXDLayer(HitPatternVXD::PXDMode::gated));
    EXPECT_EQ(lastLayerPXD, myHitPattern.getLastPXDLayer(HitPatternVXD::PXDMode::gated));
  }
}
