/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giacomo De Pietro                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* KLM headers. */
#include <klm/dataobjects/KLMMuidLikelihood.h>
#include <klm/muid/MuidBuilder.h>
#include <klm/muid/MuidElementNumbers.h>

/* Belle2 headers. */
#include <framework/database/DBStore.h>
#include <framework/database/Configuration.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/TestHelpers.h>

/* C++ headers. */
#include <bitset>
#include <string>

using namespace std;

namespace Belle2 {

  /** Test class for the MuidBuilder class, using the payloads in the default (master) Global Tag. */
  class MuidBuilderTest : public ::testing::Test {

  protected:
    /** Register the necessary objects in the DataStore and setup the Database. */
    void SetUp() override
    {
      /* DataStore. */
      DataStore::Instance().setInitializeActive(true);
      StoreArray<KLMMuidLikelihood> muids;
      muids.registerInDataStore();
      StoreObjPtr<EventMetaData> eventMetaData;
      eventMetaData.registerInDataStore();
      DataStore::Instance().setInitializeActive(false);
      /* Database. */
      eventMetaData.construct(1, 0, 0);
      DBStore& dbStore = DBStore::Instance();
      dbStore.update();
      dbStore.updateEvent();
      Conditions::Configuration::getInstance();
    }

    /** Clear the DataStore */
    void TearDown() override
    {
      DataStore::Instance().reset();
    }

    /** Pointer to a Muid object. */
    KLMMuidLikelihood* m_muid = nullptr;

    /** Pointer to a MuidBuilder class. */
    MuidBuilder* m_muidBuilder = nullptr;

  };

  /** Test the MuidBuilder class. */
  TEST_F(MuidBuilderTest, MuidBuilder)
  {
    StoreArray<KLMMuidLikelihood> muids;

    /* Test for a muon with 14 hits in the barrel. */
    int pdg = 13;
    std::bitset<30> bitExtPattern(std::string("11111111111111"));
    unsigned int extPattern = static_cast<unsigned int>(bitExtPattern.to_ulong());
    std::bitset<30> bitHitPattern(std::string("11111111111111"));
    unsigned int hitPattern = static_cast<unsigned int>(bitHitPattern.to_ulong());
    bool isForward = true;
    bool escaped = false;
    int lastBarrelLayer = 13;
    int lastEndcapLayer = -1;
    unsigned int outcome = MuidElementNumbers::calculateExtrapolationOutcome(isForward, escaped, lastBarrelLayer, lastEndcapLayer);
    double chiSquared = 29.5;
    int degreesOfFreedom = 28;
    m_muid = muids.appendNew();
    m_muid->setPDGCode(pdg);
    m_muid->setExtLayerPattern(extPattern);
    m_muid->setHitLayerPattern(hitPattern);
    m_muid->setIsForward(isForward);
    m_muid->setBarrelExtLayer(lastBarrelLayer);
    m_muid->setEndcapExtLayer(lastBarrelLayer);
    m_muid->setOutcome(outcome);
    m_muid->setChiSquared(chiSquared);
    m_muid->setDegreesOfFreedom(degreesOfFreedom);
    std::vector<int> pdgVector = MuidElementNumbers::getPDGVector(-1);
    std::vector<float> logLVector = { -169.215, -0.288937, -46.5124, -49.7292, -72.7715, -96.2517};
    for (size_t i = 0; i < pdgVector.size(); ++i) {
      m_muidBuilder = new MuidBuilder(pdgVector.at(i));
      float logL = std::log(m_muidBuilder->getPDF(m_muid));
      EXPECT_LT(std::abs(logL - logLVector.at(i)), 10E-4);
      delete m_muidBuilder;
    }
    logLVector.clear();

    /* Test for the positive-charged hypotheses. */
    std::vector<int> pdgVectorPlus = MuidElementNumbers::getPDGVector(1);
    logLVector = { -169.215, -0.284881, -45.7914, -42.7717, -95.9839, -118.769};
    for (size_t i = 0; i < pdgVector.size(); ++i) {
      m_muidBuilder = new MuidBuilder(pdgVectorPlus.at(i));
      float logL = std::log(m_muidBuilder->getPDF(m_muid));
      EXPECT_LT(std::abs(logL - logLVector.at(i)), 10E-4);
      delete m_muidBuilder;
    }
    logLVector.clear();

    /* Test for a muon with some discrepancies in hit and ext. patterns. */
    bitExtPattern = std::bitset<30>(std::string("11111111111111"));
    extPattern = static_cast<unsigned int>(bitExtPattern.to_ulong());
    bitHitPattern = std::bitset<30>(std::string("11011111111101"));
    hitPattern = static_cast<unsigned int>(bitHitPattern.to_ulong());
    isForward = true;
    escaped = false;
    lastBarrelLayer = 13;
    lastEndcapLayer = -1;
    outcome = MuidElementNumbers::calculateExtrapolationOutcome(isForward, escaped, lastBarrelLayer, lastEndcapLayer);
    chiSquared = 23.5;
    degreesOfFreedom = 24;
    m_muid = muids.appendNew();
    m_muid->setPDGCode(pdg);
    m_muid->setExtLayerPattern(extPattern);
    m_muid->setHitLayerPattern(hitPattern);
    m_muid->setIsForward(isForward);
    m_muid->setBarrelExtLayer(lastBarrelLayer);
    m_muid->setEndcapExtLayer(lastBarrelLayer);
    m_muid->setOutcome(outcome);
    m_muid->setChiSquared(chiSquared);
    m_muid->setDegreesOfFreedom(degreesOfFreedom);
    logLVector = { -147.07, -6.37567, -40.9424, -43.8204, -63.8973, -84.6684};
    for (size_t i = 0; i < pdgVector.size(); ++i) {
      m_muidBuilder = new MuidBuilder(pdgVector.at(i));
      float logL = std::log(m_muidBuilder->getPDF(m_muid));
      EXPECT_LT(std::abs(logL - logLVector.at(i)), 10E-4);
      delete m_muidBuilder;
    }
    logLVector.clear();

    /* Test for a muon with large discrepancies in hit and ext. patterns. */
    bitExtPattern = std::bitset<30>(std::string("11111111111111"));
    extPattern = static_cast<unsigned int>(bitExtPattern.to_ulong());
    bitHitPattern = std::bitset<30>(std::string("11"));
    hitPattern = static_cast<unsigned int>(bitHitPattern.to_ulong());
    isForward = true;
    escaped = false;
    lastBarrelLayer = 1;
    lastEndcapLayer = -1;
    outcome = MuidElementNumbers::calculateExtrapolationOutcome(isForward, escaped, lastBarrelLayer, lastEndcapLayer);
    chiSquared = 7.5;
    degreesOfFreedom = 4;
    m_muid = muids.appendNew();
    m_muid->setPDGCode(pdg);
    m_muid->setExtLayerPattern(extPattern);
    m_muid->setHitLayerPattern(hitPattern);
    m_muid->setIsForward(isForward);
    m_muid->setBarrelExtLayer(lastBarrelLayer);
    m_muid->setEndcapExtLayer(lastBarrelLayer);
    m_muid->setOutcome(outcome);
    m_muid->setChiSquared(chiSquared);
    m_muid->setDegreesOfFreedom(degreesOfFreedom);
    logLVector = { -13.0068, -1.82853, -4.78739, -6.44048, -6.67272, -6.8342};
    for (size_t i = 0; i < pdgVector.size(); ++i) {
      m_muidBuilder = new MuidBuilder(pdgVector.at(i));
      float logL = std::log(m_muidBuilder->getPDF(m_muid));
      EXPECT_LT(std::abs(logL - logLVector.at(i)), 10E-4);
      delete m_muidBuilder;
    }
    logLVector.clear();

    /* Test for a muon with hits in both barrel and endcaps. */
    bitExtPattern = std::bitset<30>(std::string("1111100000000000000000111"));
    extPattern = static_cast<unsigned int>(bitExtPattern.to_ulong());
    bitHitPattern = std::bitset<30>(std::string("1111100000000000000000111"));
    hitPattern = static_cast<unsigned int>(bitHitPattern.to_ulong());
    isForward = true;
    escaped = false;
    lastBarrelLayer = 2;
    lastEndcapLayer = 9;
    outcome = MuidElementNumbers::calculateExtrapolationOutcome(isForward, escaped, lastBarrelLayer, lastEndcapLayer);
    chiSquared = 16.5;
    degreesOfFreedom = 16;
    m_muid = muids.appendNew();
    m_muid->setPDGCode(pdg);
    m_muid->setExtLayerPattern(extPattern);
    m_muid->setHitLayerPattern(hitPattern);
    m_muid->setIsForward(isForward);
    m_muid->setBarrelExtLayer(lastBarrelLayer);
    m_muid->setEndcapExtLayer(lastBarrelLayer);
    m_muid->setOutcome(outcome);
    m_muid->setChiSquared(chiSquared);
    m_muid->setDegreesOfFreedom(degreesOfFreedom);
    logLVector = { -18.3899, -0.303499, -5.48054, -6.21227, -7.09687, -9.839};
    for (size_t i = 0; i < pdgVector.size(); ++i) {
      m_muidBuilder = new MuidBuilder(pdgVector.at(i));
      float logL = std::log(m_muidBuilder->getPDF(m_muid));
      EXPECT_LT(std::abs(logL - logLVector.at(i)), 10E-4);
      delete m_muidBuilder;
    }

  }

}
