/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <analysis/ParticleCombiner/ParticleCombiner.h>

#include <analysis/dataobjects/Particle.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;

namespace {
  /** Test fixture. */
  class ParticleCombinerTest : public ::testing::Test {
  protected:
    /** register Particle array */
    virtual void SetUp() {
      DataStore::Instance().setInitializeActive(true);
      StoreArray<Particle>::registerPersistent();
      DataStore::Instance().setInitializeActive(false);
    }

    /** clear datastore */
    virtual void TearDown() {
      DataStore::Instance().reset();
    }
  };

  TEST_F(ParticleCombinerTest, ListCombinerTest)
  {

    ListCombiner listCombiner(3);
    listCombiner.init(ParticleList::c_Particle);
    EXPECT_TRUE(listCombiner.loadNext());
    EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_Particle, ParticleList::c_Particle, ParticleList::c_Particle }), listCombiner.getCurrentTypes());
    EXPECT_TRUE(listCombiner.loadNext());
    EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_SelfConjugatedParticle, ParticleList::c_Particle, ParticleList::c_Particle }), listCombiner.getCurrentTypes());
    EXPECT_TRUE(listCombiner.loadNext());
    EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_Particle, ParticleList::c_SelfConjugatedParticle, ParticleList::c_Particle }), listCombiner.getCurrentTypes());
    EXPECT_TRUE(listCombiner.loadNext());
    EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_SelfConjugatedParticle, ParticleList::c_SelfConjugatedParticle, ParticleList::c_Particle }), listCombiner.getCurrentTypes());
    EXPECT_TRUE(listCombiner.loadNext());
    EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_Particle, ParticleList::c_Particle, ParticleList::c_SelfConjugatedParticle }), listCombiner.getCurrentTypes());
    EXPECT_TRUE(listCombiner.loadNext());
    EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_SelfConjugatedParticle, ParticleList::c_Particle, ParticleList::c_SelfConjugatedParticle }), listCombiner.getCurrentTypes());
    EXPECT_TRUE(listCombiner.loadNext());
    EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_Particle, ParticleList::c_SelfConjugatedParticle, ParticleList::c_SelfConjugatedParticle }), listCombiner.getCurrentTypes());
    EXPECT_FALSE(listCombiner.loadNext());

    listCombiner.init(ParticleList::c_AntiParticle);
    EXPECT_TRUE(listCombiner.loadNext());
    EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_AntiParticle, ParticleList::c_AntiParticle, ParticleList::c_AntiParticle }), listCombiner.getCurrentTypes());
    EXPECT_TRUE(listCombiner.loadNext());
    EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_SelfConjugatedParticle, ParticleList::c_AntiParticle, ParticleList::c_AntiParticle }), listCombiner.getCurrentTypes());
    EXPECT_TRUE(listCombiner.loadNext());
    EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_AntiParticle, ParticleList::c_SelfConjugatedParticle, ParticleList::c_AntiParticle }), listCombiner.getCurrentTypes());
    EXPECT_TRUE(listCombiner.loadNext());
    EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_SelfConjugatedParticle, ParticleList::c_SelfConjugatedParticle, ParticleList::c_AntiParticle }), listCombiner.getCurrentTypes());
    EXPECT_TRUE(listCombiner.loadNext());
    EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_AntiParticle, ParticleList::c_AntiParticle, ParticleList::c_SelfConjugatedParticle }), listCombiner.getCurrentTypes());
    EXPECT_TRUE(listCombiner.loadNext());
    EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_SelfConjugatedParticle, ParticleList::c_AntiParticle, ParticleList::c_SelfConjugatedParticle }), listCombiner.getCurrentTypes());
    EXPECT_TRUE(listCombiner.loadNext());
    EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_AntiParticle, ParticleList::c_SelfConjugatedParticle, ParticleList::c_SelfConjugatedParticle }), listCombiner.getCurrentTypes());
    EXPECT_FALSE(listCombiner.loadNext());

    listCombiner.init(ParticleList::c_SelfConjugatedParticle);
    EXPECT_TRUE(listCombiner.loadNext());
    EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_SelfConjugatedParticle, ParticleList::c_SelfConjugatedParticle, ParticleList::c_SelfConjugatedParticle }), listCombiner.getCurrentTypes());
    EXPECT_FALSE(listCombiner.loadNext());

  }

  TEST_F(ParticleCombinerTest, IndexCombinerTest)
  {

    IndexCombiner indexCombiner(3);
    indexCombiner.init(std::vector<unsigned int>({2, 4, 3}));
    EXPECT_TRUE(indexCombiner.loadNext());
    EXPECT_EQ(std::vector<unsigned int>({0, 0, 0}), indexCombiner.getCurrentIndices());
    EXPECT_TRUE(indexCombiner.loadNext());
    EXPECT_EQ(std::vector<unsigned int>({1, 0, 0}), indexCombiner.getCurrentIndices());
    EXPECT_TRUE(indexCombiner.loadNext());
    EXPECT_EQ(std::vector<unsigned int>({0, 1, 0}), indexCombiner.getCurrentIndices());
    EXPECT_TRUE(indexCombiner.loadNext());
    EXPECT_EQ(std::vector<unsigned int>({1, 1, 0}), indexCombiner.getCurrentIndices());
    EXPECT_TRUE(indexCombiner.loadNext());
    EXPECT_EQ(std::vector<unsigned int>({0, 2, 0}), indexCombiner.getCurrentIndices());
    EXPECT_TRUE(indexCombiner.loadNext());
    EXPECT_EQ(std::vector<unsigned int>({1, 2, 0}), indexCombiner.getCurrentIndices());
    EXPECT_TRUE(indexCombiner.loadNext());
    EXPECT_EQ(std::vector<unsigned int>({0, 3, 0}), indexCombiner.getCurrentIndices());
    EXPECT_TRUE(indexCombiner.loadNext());
    EXPECT_EQ(std::vector<unsigned int>({1, 3, 0}), indexCombiner.getCurrentIndices());
    EXPECT_TRUE(indexCombiner.loadNext());
    EXPECT_EQ(std::vector<unsigned int>({0, 0, 1}), indexCombiner.getCurrentIndices());
    EXPECT_TRUE(indexCombiner.loadNext());
    EXPECT_EQ(std::vector<unsigned int>({1, 0, 1}), indexCombiner.getCurrentIndices());
    EXPECT_TRUE(indexCombiner.loadNext());
    EXPECT_EQ(std::vector<unsigned int>({0, 1, 1}), indexCombiner.getCurrentIndices());
    EXPECT_TRUE(indexCombiner.loadNext());
    EXPECT_EQ(std::vector<unsigned int>({1, 1, 1}), indexCombiner.getCurrentIndices());
    EXPECT_TRUE(indexCombiner.loadNext());
    EXPECT_EQ(std::vector<unsigned int>({0, 2, 1}), indexCombiner.getCurrentIndices());
    EXPECT_TRUE(indexCombiner.loadNext());
    EXPECT_EQ(std::vector<unsigned int>({1, 2, 1}), indexCombiner.getCurrentIndices());
    EXPECT_TRUE(indexCombiner.loadNext());
    EXPECT_EQ(std::vector<unsigned int>({0, 3, 1}), indexCombiner.getCurrentIndices());
    EXPECT_TRUE(indexCombiner.loadNext());
    EXPECT_EQ(std::vector<unsigned int>({1, 3, 1}), indexCombiner.getCurrentIndices());
    EXPECT_TRUE(indexCombiner.loadNext());
    EXPECT_EQ(std::vector<unsigned int>({0, 0, 2}), indexCombiner.getCurrentIndices());
    EXPECT_TRUE(indexCombiner.loadNext());
    EXPECT_EQ(std::vector<unsigned int>({1, 0, 2}), indexCombiner.getCurrentIndices());
    EXPECT_TRUE(indexCombiner.loadNext());
    EXPECT_EQ(std::vector<unsigned int>({0, 1, 2}), indexCombiner.getCurrentIndices());
    EXPECT_TRUE(indexCombiner.loadNext());
    EXPECT_EQ(std::vector<unsigned int>({1, 1, 2}), indexCombiner.getCurrentIndices());
    EXPECT_TRUE(indexCombiner.loadNext());
    EXPECT_EQ(std::vector<unsigned int>({0, 2, 2}), indexCombiner.getCurrentIndices());
    EXPECT_TRUE(indexCombiner.loadNext());
    EXPECT_EQ(std::vector<unsigned int>({1, 2, 2}), indexCombiner.getCurrentIndices());
    EXPECT_TRUE(indexCombiner.loadNext());
    EXPECT_EQ(std::vector<unsigned int>({0, 3, 2}), indexCombiner.getCurrentIndices());
    EXPECT_TRUE(indexCombiner.loadNext());
    EXPECT_EQ(std::vector<unsigned int>({1, 3, 2}), indexCombiner.getCurrentIndices());
    EXPECT_FALSE(indexCombiner.loadNext());


  }

}  // namespace
