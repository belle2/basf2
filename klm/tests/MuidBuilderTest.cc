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
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>
#include <klm/dataobjects/KLMMuidLikelihood.h>
#include <klm/muid/MuidBuilder.h>
#include <klm/muid/MuidElementNumbers.h>

/* Belle 2 headers. */
#include <framework/database/Database.h>
#include <framework/database/DBStore.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Const.h>

/* C++ headers. */
#include <bitset>
#include <string>

/* Google Test headers. */
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {

  /** Test for the MuidBuilder class, using the payloads in the default Global Tag. */
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
    }

    /** Clear the DataStore and reset the Database. */
    void TearDown() override
    {
      DataStore::Instance().reset();
      Database::reset();
    }

    /** Pointer to a Muid object. */
    KLMMuidLikelihood* m_muid = nullptr;

    /** Pointer to a MuidBuilder class. */
    MuidBuilder* m_muidBuilder = nullptr;

    /** Vector of negative charged hypotheses. */
    std::vector<int> m_pdgVectorMinus = MuidElementNumbers::getPDGVector(-1);

    /** Vector of positive charged hypotheses. */
    std::vector<int> m_pdgVectorPlus = MuidElementNumbers::getPDGVector(1);
  };

  /** Test for a muon with 14 hits in the barrel. */
  TEST_F(MuidBuilderTest, MuidBuilder01)
  {
    StoreArray<KLMMuidLikelihood> muids;
    int pdg = Const::muon.getPDGCode();
    std::bitset<30> bitExtPattern(std::string("11111111111111"));
    unsigned int extPattern = static_cast<unsigned int>(bitExtPattern.to_ulong());
    unsigned int hitPattern = static_cast<unsigned int>(bitExtPattern.to_ulong());
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
    std::vector<float> logLVector = { -169.215, -0.288937, -46.5124, -49.7292, -72.7715, -96.2517};
    for (size_t i = 0; i < m_pdgVectorMinus.size(); ++i) {
      m_muidBuilder = new MuidBuilder(m_pdgVectorMinus.at(i));
      float logL = std::log(m_muidBuilder->getPDF(m_muid));
      EXPECT_LT(std::abs(logL - logLVector.at(i)), 10E-4);
      delete m_muidBuilder;
    }
    logLVector.clear();
    /* Test for the positive-charged hypotheses. */
    m_pdgVectorPlus = MuidElementNumbers::getPDGVector(1);
    logLVector = { -169.215, -0.284881, -45.7914, -42.7717, -95.9839, -118.769};
    for (size_t i = 0; i < m_pdgVectorPlus.size(); ++i) {
      m_muidBuilder = new MuidBuilder(m_pdgVectorPlus.at(i));
      float logL = std::log(m_muidBuilder->getPDF(m_muid));
      EXPECT_LT(std::abs(logL - logLVector.at(i)), 10E-4);
      delete m_muidBuilder;
    }
  }

  /** Test for a muon with some discrepancies between hit and ext. patterns. */
  TEST_F(MuidBuilderTest, MuidBuilder02)
  {
    StoreArray<KLMMuidLikelihood> muids;
    int pdg = Const::muon.getPDGCode();
    std::bitset<30> bitExtPattern(std::string("11111111111111"));
    unsigned int extPattern = static_cast<unsigned int>(bitExtPattern.to_ulong());
    std::bitset<30> bitHitPattern(std::string("11011111111101"));
    unsigned int hitPattern = static_cast<unsigned int>(bitHitPattern.to_ulong());
    bool isForward = true;
    bool escaped = false;
    int lastBarrelLayer = 13;
    int lastEndcapLayer = -1;
    unsigned int outcome = MuidElementNumbers::calculateExtrapolationOutcome(isForward, escaped, lastBarrelLayer, lastEndcapLayer);
    double chiSquared = 23.5;
    int degreesOfFreedom = 24;
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
    std::vector<float> logLVector = { -147.07, -6.37567, -40.9424, -43.8204, -63.8973, -84.6684};
    for (size_t i = 0; i < m_pdgVectorMinus.size(); ++i) {
      m_muidBuilder = new MuidBuilder(m_pdgVectorMinus.at(i));
      float logL = std::log(m_muidBuilder->getPDF(m_muid));
      EXPECT_LT(std::abs(logL - logLVector.at(i)), 10E-4);
      delete m_muidBuilder;
    }
  }

  /** Test for a muon with large discrepancies between hit and ext. patterns. */
  TEST_F(MuidBuilderTest, MuidBuilder03)
  {
    StoreArray<KLMMuidLikelihood> muids;
    int pdg = Const::muon.getPDGCode();
    std::bitset<30> bitExtPattern(std::string("11111111111111"));
    unsigned int extPattern = static_cast<unsigned int>(bitExtPattern.to_ulong());
    std::bitset<30> bitHitPattern(std::string("11"));
    unsigned int hitPattern = static_cast<unsigned int>(bitHitPattern.to_ulong());
    bool isForward = true;
    bool escaped = false;
    int lastBarrelLayer = 1;
    int lastEndcapLayer = -1;
    unsigned int outcome = MuidElementNumbers::calculateExtrapolationOutcome(isForward, escaped, lastBarrelLayer, lastEndcapLayer);
    double chiSquared = 7.5;
    int degreesOfFreedom = 4;
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
    std::vector<float> logLVector = { -13.0068, -1.82853, -4.78739, -6.44048, -6.67272, -6.8342};
    for (size_t i = 0; i < m_pdgVectorMinus.size(); ++i) {
      m_muidBuilder = new MuidBuilder(m_pdgVectorMinus.at(i));
      float logL = std::log(m_muidBuilder->getPDF(m_muid));
      EXPECT_LT(std::abs(logL - logLVector.at(i)), 10E-4);
      delete m_muidBuilder;
    }
  }

  /** Test for a muon with hits in both barrel and endcaps. */
  TEST_F(MuidBuilderTest, MuidBuilder04)
  {
    StoreArray<KLMMuidLikelihood> muids;
    int pdg = Const::muon.getPDGCode();
    std::bitset<30> bitExtPattern(std::string("1111111000000000000111"));
    unsigned int extPattern = static_cast<unsigned int>(bitExtPattern.to_ulong());
    unsigned int hitPattern = static_cast<unsigned int>(bitExtPattern.to_ulong());
    /* Test the forward endcap. */
    bool isForward = true;
    bool escaped = false;
    int lastBarrelLayer = 2;
    int lastEndcapLayer = 6;
    unsigned int outcome = MuidElementNumbers::calculateExtrapolationOutcome(isForward, escaped, lastBarrelLayer, lastEndcapLayer);
    double chiSquared = 19.5;
    int degreesOfFreedom = 20;
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
    std::vector<float> logLVector = { -44.4325, -0.746618, -17.3903, -18.7434, -25.4178, -34.7886};
    for (size_t i = 0; i < m_pdgVectorMinus.size(); ++i) {
      m_muidBuilder = new MuidBuilder(m_pdgVectorMinus.at(i));
      float logL = std::log(m_muidBuilder->getPDF(m_muid));
      EXPECT_LT(std::abs(logL - logLVector.at(i)), 10E-4);
      delete m_muidBuilder;
    }
    logLVector.clear();
    /* Test the backward endcap. */
    isForward = false;
    outcome = MuidElementNumbers::calculateExtrapolationOutcome(isForward, escaped, lastBarrelLayer, lastEndcapLayer);
    m_muid->setIsForward(isForward);
    m_muid->setOutcome(outcome);
    logLVector = { -50.7005, -1.11268, -18.3302, -20.7392, -26.5815, -33.6206};
    for (size_t i = 0; i < m_pdgVectorMinus.size(); ++i) {
      m_muidBuilder = new MuidBuilder(m_pdgVectorMinus.at(i));
      float logL = std::log(m_muidBuilder->getPDF(m_muid));
      EXPECT_LT(std::abs(logL - logLVector.at(i)), 10E-4);
      delete m_muidBuilder;
    }
  }

  /** Test for a muon with hits in both barrel and endcaps. */
  TEST_F(MuidBuilderTest, MuidBuilder05)
  {
    StoreArray<KLMMuidLikelihood> muids;
    int pdg = Const::muon.getPDGCode();
    std::bitset<30> bitExtPattern(std::string("1111111000000000000111"));
    unsigned int extPattern = static_cast<unsigned int>(bitExtPattern.to_ulong());
    std::bitset<30> bitHitPattern(std::string("1100111000000000000101"));
    unsigned int hitPattern = static_cast<unsigned int>(bitHitPattern.to_ulong());
    /* Test the forward endcap. */
    bool isForward = true;
    bool escaped = false;
    int lastBarrelLayer = 2;
    int lastEndcapLayer = 6;
    unsigned int outcome = MuidElementNumbers::calculateExtrapolationOutcome(isForward, escaped, lastBarrelLayer, lastEndcapLayer);
    double chiSquared = 15.5;
    int degreesOfFreedom = 14;
    float efficiency = 0.8437;
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
    for (int layer = 0; layer < BKLMElementNumbers::getMaximalLayerNumber(); ++layer)
      m_muid->setExtBKLMEfficiencyValue(layer, efficiency);
    for (int layer = 0; layer < EKLMElementNumbers::getMaximalLayerNumber(); ++layer)
      m_muid->setExtEKLMEfficiencyValue(layer, efficiency);
    std::vector<float> logLVector = { -38.1205, -2.43391, -16.0512, -17.094, -23.4617, -31.9516};
    for (size_t i = 0; i < m_pdgVectorMinus.size(); ++i) {
      m_muidBuilder = new MuidBuilder(m_pdgVectorMinus.at(i));
      float logL = std::log(m_muidBuilder->getPDF(m_muid));
      EXPECT_LT(std::abs(logL - logLVector.at(i)), 10E-4);
      delete m_muidBuilder;
    }
    logLVector.clear();
    /* Test the backward endcap. */
    isForward = false;
    outcome = MuidElementNumbers::calculateExtrapolationOutcome(isForward, escaped, lastBarrelLayer, lastEndcapLayer);
    m_muid->setIsForward(isForward);
    m_muid->setOutcome(outcome);
    logLVector = { -43.7474, -2.51744, -16.6359, -18.5523, -23.8643, -29.9728};
    for (size_t i = 0; i < m_pdgVectorMinus.size(); ++i) {
      m_muidBuilder = new MuidBuilder(m_pdgVectorMinus.at(i));
      float logL = std::log(m_muidBuilder->getPDF(m_muid));
      EXPECT_LT(std::abs(logL - logLVector.at(i)), 10E-4);
      delete m_muidBuilder;
    }
  }

}
