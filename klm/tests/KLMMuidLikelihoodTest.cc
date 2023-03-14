/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* KLM headers. */
#include <klm/dataobjects/KLMMuidLikelihood.h>

/* Basf2 headers. */
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>

/* C++ headers. */
#include <bitset>
#include <string>

/* Google Test headers. */
#include <gtest/gtest.h>

namespace Belle2 {

  /** Test class for the KLMMuidLikelihood object. */
  class KLMMuidLikelihoodTest : public ::testing::Test {

  protected:
    /** Register KLMMuidLikelihood in the DataStore. */
    void SetUp() override
    {
      DataStore::Instance().setInitializeActive(true);
      StoreArray<KLMMuidLikelihood> muids;
      muids.registerInDataStore();
      DataStore::Instance().setInitializeActive(false);
    }

    /** Clear the DataStore */
    void TearDown() override
    {
      DataStore::Instance().reset();
    }

    /** Pointer to a Muid object. */
    KLMMuidLikelihood* m_muid = nullptr;
  };

  /** Test some setters and getters. */
  TEST_F(KLMMuidLikelihoodTest, KLMMuidLikelihood)
  {
    StoreArray<KLMMuidLikelihood> muids;

    // Test the non-default constructor
    int pdg = Const::muon.getPDGCode();
    m_muid = muids.appendNew();
    m_muid->setPDGCode(pdg);
    EXPECT_EQ(m_muid->getPDGCode(), pdg);
    EXPECT_EQ(m_muid->getOutcome(), 0);
    EXPECT_EQ(m_muid->getHitLayer(), -1);
    EXPECT_FLOAT_EQ(m_muid->getLogL_mu(), -1.0E20);

    // Test the efficiency methods
    int layer = 4;
    float efficiency = 0.2;
    m_muid->setExtBKLMEfficiencyValue(layer, efficiency);
    EXPECT_FLOAT_EQ(m_muid->getExtBKLMEfficiencyValue(layer), efficiency);
    m_muid->setExtEKLMEfficiencyValue(layer, efficiency);
    EXPECT_FLOAT_EQ(m_muid->getExtEKLMEfficiencyValue(layer), efficiency);

    // Test the methods to count the hits and to check the crossed layer
    // 111111
    // it means 6 hits in BKLM and 0 in EKLM
    std::bitset<30> bitPattern(std::string("111111"));
    unsigned int pattern = static_cast<unsigned int>(bitPattern.to_ulong());
    m_muid->setHitLayerPattern(pattern);
    EXPECT_EQ(m_muid->getHitLayerPattern(), pattern);
    EXPECT_EQ(m_muid->getTotalBarrelHits(), 6);
    EXPECT_EQ(m_muid->getTotalEndcapHits(), 0);
    m_muid->setExtLayerPattern(pattern);
    EXPECT_TRUE(m_muid->isExtrapolatedBarrelLayerCrossed(5));
    EXPECT_FALSE(m_muid->isExtrapolatedBarrelLayerCrossed(6));
    EXPECT_FALSE(m_muid->isExtrapolatedEndcapLayerCrossed(0));
    // 1000000000000000
    // it means 0 hits in BKLM and 1 in EKLM
    bitPattern = std::bitset<30>(std::string("1000000000000000"));
    pattern = static_cast<unsigned int>(bitPattern.to_ulong());
    m_muid->setHitLayerPattern(pattern);
    EXPECT_EQ(m_muid->getHitLayerPattern(), pattern);
    EXPECT_EQ(m_muid->getTotalBarrelHits(), 0);
    EXPECT_EQ(m_muid->getTotalEndcapHits(), 1);
    m_muid->setExtLayerPattern(pattern);
    EXPECT_FALSE(m_muid->isExtrapolatedBarrelLayerCrossed(0));
    EXPECT_TRUE(m_muid->isExtrapolatedEndcapLayerCrossed(0));
    EXPECT_FALSE(m_muid->isExtrapolatedEndcapLayerCrossed(1));
    // 10101000000000111000
    // it means 3 hits in BKLM and 3 in EKLM
    bitPattern = std::bitset<30>(std::string("10101000000000111000"));
    pattern = static_cast<unsigned int>(bitPattern.to_ulong());
    m_muid->setHitLayerPattern(pattern);
    EXPECT_EQ(m_muid->getHitLayerPattern(), pattern);
    EXPECT_EQ(m_muid->getTotalBarrelHits(), 3);
    EXPECT_EQ(m_muid->getTotalEndcapHits(), 3);
    m_muid->setExtLayerPattern(pattern);
    EXPECT_TRUE(m_muid->isExtrapolatedBarrelLayerCrossed(4));
    EXPECT_TRUE(m_muid->isExtrapolatedEndcapLayerCrossed(0));
    EXPECT_FALSE(m_muid->isExtrapolatedEndcapLayerCrossed(10));
    // 11111111111111111111111111111
    // it means 15 hits in BKLM, 14 in EKLM and 1 "fake" hit
    bitPattern = std::bitset<30>(std::string("11111111111111111111111111111"));
    pattern = static_cast<unsigned int>(bitPattern.to_ulong());
    m_muid->setHitLayerPattern(pattern);
    EXPECT_EQ(m_muid->getHitLayerPattern(), pattern);
    EXPECT_EQ(m_muid->getTotalBarrelHits(), 15);
    EXPECT_EQ(m_muid->getTotalEndcapHits(), 14);
    m_muid->setExtLayerPattern(pattern);
    EXPECT_TRUE(m_muid->isExtrapolatedBarrelLayerCrossed(14));
    EXPECT_FALSE(m_muid->isExtrapolatedBarrelLayerCrossed(15));
    EXPECT_TRUE(m_muid->isExtrapolatedEndcapLayerCrossed(13));
  }

}
