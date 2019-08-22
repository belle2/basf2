/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giacomo De Pietro                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <tracking/dataobjects/Muid.h>
#include <framework/datastore/StoreArray.h>
#include <framework/utilities/TestHelpers.h>

/* C++ headers. */
#include <bitset>
#include <string>

using namespace std;

namespace Belle2 {

  /** Test class for the Muid object. */
  class MuidTest : public ::testing::Test {

  protected:
    /** Register Muid in the DataStore. */
    void SetUp() override
    {
      DataStore::Instance().setInitializeActive(true);
      StoreArray<Muid> muids;
      muids.registerInDataStore();
      DataStore::Instance().setInitializeActive(false);
    }

    /** Clear the DataStore */
    void TearDown() override
    {
      DataStore::Instance().reset();
    }

    /** Pointer to a Muid object. */
    Muid* m_muid;
  };

  /** Test some setters and getters. */
  TEST_F(MuidTest, Muid)
  {
    StoreArray<Muid> muids;

    // Test the non-default constructor
    int pdg = 13;
    m_muid = muids.appendNew(pdg);
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

    // Test the methods to count the hits
    // 111111
    // it means 6 hits in BKLM and 0 in EKLM
    std::bitset<30> bitPattern(std::string("111111"));
    unsigned int pattern = static_cast<unsigned int>(bitPattern.to_ulong());
    m_muid->setHitLayerPattern(pattern);
    EXPECT_EQ(m_muid->getHitLayerPattern(), pattern);
    EXPECT_EQ(m_muid->getTotalBarrelHits(), 6);
    EXPECT_EQ(m_muid->getTotalEndcapHits(), 0);
    // 1000000000000000
    // it means 0 hits in BKLM and 1 in EKLM
    bitPattern = std::bitset<30>(std::string("1000000000000000"));
    pattern = static_cast<unsigned int>(bitPattern.to_ulong());
    m_muid->setHitLayerPattern(pattern);
    EXPECT_EQ(m_muid->getHitLayerPattern(), pattern);
    EXPECT_EQ(m_muid->getTotalBarrelHits(), 0);
    EXPECT_EQ(m_muid->getTotalEndcapHits(), 1);
    // 10101000000000111000
    // it means 3 hits in BKLM and 3 in EKLM
    bitPattern = std::bitset<30>(std::string("10101000000000111000"));
    pattern = static_cast<unsigned int>(bitPattern.to_ulong());
    m_muid->setHitLayerPattern(pattern);
    EXPECT_EQ(m_muid->getHitLayerPattern(), pattern);
    EXPECT_EQ(m_muid->getTotalBarrelHits(), 3);
    EXPECT_EQ(m_muid->getTotalEndcapHits(), 3);
    // 11111111111111111111111111111
    // it means 15 hits in BKLM, 14 in EKLM and 1 "fake" hit
    bitPattern = std::bitset<30>(std::string("11111111111111111111111111111"));
    pattern = static_cast<unsigned int>(bitPattern.to_ulong());
    m_muid->setHitLayerPattern(pattern);
    EXPECT_EQ(m_muid->getHitLayerPattern(), pattern);
    EXPECT_EQ(m_muid->getTotalBarrelHits(), 15);
    EXPECT_EQ(m_muid->getTotalEndcapHits(), 14);
  }

}
