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
#include <analysis/dataobjects/ParticleList.h>
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
    {
      ListCombiner listCombiner(0);
      listCombiner.init(ParticleList::c_Particle);
      EXPECT_FALSE(listCombiner.loadNext());
      listCombiner.init(ParticleList::c_AntiParticle);
      EXPECT_FALSE(listCombiner.loadNext());
      listCombiner.init(ParticleList::c_SelfConjugatedParticle);
      EXPECT_FALSE(listCombiner.loadNext());
    }

    {
      ListCombiner listCombiner(1);
      listCombiner.init(ParticleList::c_Particle);
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_Particle}), listCombiner.getCurrentTypes());
      EXPECT_FALSE(listCombiner.loadNext());
      listCombiner.init(ParticleList::c_AntiParticle);
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_AntiParticle}), listCombiner.getCurrentTypes());
      EXPECT_FALSE(listCombiner.loadNext());
      listCombiner.init(ParticleList::c_SelfConjugatedParticle);
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_SelfConjugatedParticle}), listCombiner.getCurrentTypes());
      EXPECT_FALSE(listCombiner.loadNext());
    }

    {
      ListCombiner listCombiner(2);
      listCombiner.init(ParticleList::c_Particle);
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_Particle, ParticleList::c_Particle}), listCombiner.getCurrentTypes());
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_SelfConjugatedParticle, ParticleList::c_Particle}), listCombiner.getCurrentTypes());
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_Particle, ParticleList::c_SelfConjugatedParticle}), listCombiner.getCurrentTypes());
      EXPECT_FALSE(listCombiner.loadNext());
      listCombiner.init(ParticleList::c_AntiParticle);
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_AntiParticle, ParticleList::c_AntiParticle}), listCombiner.getCurrentTypes());
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_SelfConjugatedParticle, ParticleList::c_AntiParticle}), listCombiner.getCurrentTypes());
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_AntiParticle, ParticleList::c_SelfConjugatedParticle}), listCombiner.getCurrentTypes());
      EXPECT_FALSE(listCombiner.loadNext());
      listCombiner.init(ParticleList::c_SelfConjugatedParticle);
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_SelfConjugatedParticle, ParticleList::c_SelfConjugatedParticle}), listCombiner.getCurrentTypes());
      EXPECT_FALSE(listCombiner.loadNext());
    }

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

  }

  TEST_F(ParticleCombinerTest, IndexCombinerTest)
  {
    {
      IndexCombiner indexCombiner(0);
      indexCombiner.init(std::vector<unsigned int>({}));
      EXPECT_FALSE(indexCombiner.loadNext());
    }

    {
      IndexCombiner indexCombiner(1);
      indexCombiner.init(std::vector<unsigned int>({0}));
      EXPECT_FALSE(indexCombiner.loadNext());
      indexCombiner.init(std::vector<unsigned int>({3}));
      EXPECT_TRUE(indexCombiner.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({0}), indexCombiner.getCurrentIndices());
      EXPECT_TRUE(indexCombiner.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({1}), indexCombiner.getCurrentIndices());
      EXPECT_TRUE(indexCombiner.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({2}), indexCombiner.getCurrentIndices());
      EXPECT_FALSE(indexCombiner.loadNext());
    }

    {
      IndexCombiner indexCombiner(2);
      indexCombiner.init(std::vector<unsigned int>({2, 0}));
      EXPECT_FALSE(indexCombiner.loadNext());

      indexCombiner.init(std::vector<unsigned int>({1, 3}));
      EXPECT_TRUE(indexCombiner.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({0, 0}), indexCombiner.getCurrentIndices());
      EXPECT_TRUE(indexCombiner.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({0, 1}), indexCombiner.getCurrentIndices());
      EXPECT_TRUE(indexCombiner.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({0, 2}), indexCombiner.getCurrentIndices());
      EXPECT_FALSE(indexCombiner.loadNext());

      indexCombiner.init(std::vector<unsigned int>({2, 3}));
      EXPECT_TRUE(indexCombiner.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({0, 0}), indexCombiner.getCurrentIndices());
      EXPECT_TRUE(indexCombiner.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({1, 0}), indexCombiner.getCurrentIndices());
      EXPECT_TRUE(indexCombiner.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({0, 1}), indexCombiner.getCurrentIndices());
      EXPECT_TRUE(indexCombiner.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({1, 1}), indexCombiner.getCurrentIndices());
      EXPECT_TRUE(indexCombiner.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({0, 2}), indexCombiner.getCurrentIndices());
      EXPECT_TRUE(indexCombiner.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({1, 2}), indexCombiner.getCurrentIndices());
      EXPECT_FALSE(indexCombiner.loadNext());
    }

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

  }

  TEST_F(ParticleCombinerTest, MuPlusMuMinus)
  {
    StoreObjPtr<ParticleList> inputA("a");
    StoreObjPtr<ParticleList> inputB("b");

    DataStore::Instance().setInitializeActive(true);
    inputA.registerAsPersistent();
    inputB.registerAsPersistent();
    DataStore::Instance().setInitializeActive(false);

    StoreArray<Particle> particles;
    particles.appendNew(TLorentzVector(), 13, Particle::c_Flavored, Particle::c_MCParticle, 0);
    particles.appendNew(TLorentzVector(), -13, Particle::c_Flavored, Particle::c_MCParticle, 1);

    inputA.create();
    inputA->setPDG(13);
    inputA->addParticle(particles[0]);
    inputA->addParticle(particles[1]);

    inputB.create();
    inputB->setPDG(-13);
    inputB->addParticle(particles[0]);
    inputB->addParticle(particles[1]);


    ParticleCombiner combiner({"a", "b"}, true); //self-conjugated particle
    int nCombinations = 0;
    while (combiner.loadNext()) {
      nCombinations++;
    }
    EXPECT_EQ(1, nCombinations);

  }

  TEST_F(ParticleCombinerTest, DStar)
  {
    StoreObjPtr<ParticleList> inputD("D0");
    StoreObjPtr<ParticleList> inputKM("K-");
    StoreObjPtr<ParticleList> inputKP("K+");
    StoreObjPtr<ParticleList> inputPi("pi+");

    DataStore::Instance().setInitializeActive(true);
    inputD.registerAsPersistent();
    inputKM.registerAsPersistent();
    inputKP.registerAsPersistent();
    inputPi.registerAsPersistent();
    DataStore::Instance().setInitializeActive(false);

    StoreArray<Particle> particles;
    particles.appendNew(TLorentzVector(), 211, Particle::c_Flavored, Particle::c_MCParticle, 0);
    particles.appendNew(TLorentzVector(), 211, Particle::c_Flavored, Particle::c_MCParticle, 1);
    particles.appendNew(TLorentzVector(), -211, Particle::c_Flavored, Particle::c_MCParticle, 2);

    particles.appendNew(TLorentzVector(), -321, Particle::c_Flavored, Particle::c_MCParticle, 3);
    particles.appendNew(TLorentzVector(), -321, Particle::c_Flavored, Particle::c_MCParticle, 4);
    particles.appendNew(TLorentzVector(), 321, Particle::c_Flavored, Particle::c_MCParticle, 5);

    inputPi.create();
    inputPi->setPDG(211);
    inputPi->addParticle(particles[0]);
    inputPi->addParticle(particles[1]);
    inputPi->addParticle(particles[2]);

    inputKP.create();
    inputKP->setPDG(321);
    inputKP->addParticle(particles[3]);
    inputKP->addParticle(particles[4]);
    inputKP->addParticle(particles[5]);

    inputKM.create();
    inputKM->setPDG(-321);
    inputKM->addParticle(particles[3]);
    inputKM->addParticle(particles[4]);
    inputKM->addParticle(particles[5]);

    inputD.create();
    inputD->setPDG(421);
    Particle* part = nullptr;

    ParticleCombiner combinerD0({"K-", "pi+"}, false); //non-self-conjugated particle
    EXPECT_TRUE(combinerD0.loadNext());
    EXPECT_EQ(ParticleList::c_Particle, combinerD0.getCurrentType());
    EXPECT_EQ(std::vector<int>({3, 0}), combinerD0.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[3], particles[0]}), combinerD0.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({3, 0}), combinerD0.getCurrentParticle(421, -421).getDaughterIndices());
    EXPECT_EQ(Particle::c_Flavored, combinerD0.getCurrentParticle(421, -421).getFlavorType());
    part = particles.appendNew(combinerD0.getCurrentParticle(421, -421));
    inputD->addParticle(part);

    EXPECT_TRUE(combinerD0.loadNext());
    EXPECT_EQ(ParticleList::c_Particle, combinerD0.getCurrentType());
    EXPECT_EQ(std::vector<int>({4, 0}), combinerD0.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[4], particles[0]}), combinerD0.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({4, 0}), combinerD0.getCurrentParticle(421, -421).getDaughterIndices());
    EXPECT_EQ(Particle::c_Flavored, combinerD0.getCurrentParticle(421, -421).getFlavorType());
    part = particles.appendNew(combinerD0.getCurrentParticle(421, -421));
    inputD->addParticle(part);

    EXPECT_TRUE(combinerD0.loadNext());
    EXPECT_EQ(ParticleList::c_Particle, combinerD0.getCurrentType());
    EXPECT_EQ(std::vector<int>({3, 1}), combinerD0.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[3], particles[1]}), combinerD0.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({3, 1}), combinerD0.getCurrentParticle(421, -421).getDaughterIndices());
    EXPECT_EQ(Particle::c_Flavored, combinerD0.getCurrentParticle(421, -421).getFlavorType());
    part = particles.appendNew(combinerD0.getCurrentParticle(421, -421));
    inputD->addParticle(part);

    EXPECT_TRUE(combinerD0.loadNext());
    EXPECT_EQ(ParticleList::c_Particle, combinerD0.getCurrentType());
    EXPECT_EQ(std::vector<int>({4, 1}), combinerD0.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[4], particles[1]}), combinerD0.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({4, 1}), combinerD0.getCurrentParticle(421, -421).getDaughterIndices());
    EXPECT_EQ(Particle::c_Flavored, combinerD0.getCurrentParticle(421, -421).getFlavorType());
    part = particles.appendNew(combinerD0.getCurrentParticle(421, -421));
    inputD->addParticle(part);

    EXPECT_TRUE(combinerD0.loadNext());
    EXPECT_EQ(ParticleList::c_AntiParticle, combinerD0.getCurrentType());
    EXPECT_EQ(std::vector<int>({5, 2}), combinerD0.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[5], particles[2]}), combinerD0.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({5, 2}), combinerD0.getCurrentParticle(421, -421).getDaughterIndices());
    EXPECT_EQ(Particle::c_Flavored, combinerD0.getCurrentParticle(421, -421).getFlavorType());
    part = particles.appendNew(combinerD0.getCurrentParticle(421, -421));
    inputD->addParticle(part);

    EXPECT_FALSE(combinerD0.loadNext());

    ParticleCombiner combinerD0_KK({"K+", "K-"}, false); //non-self-conjugated particle

    EXPECT_TRUE(combinerD0_KK.loadNext());
    EXPECT_EQ(ParticleList::c_SelfConjugatedParticle, combinerD0_KK.getCurrentType());
    EXPECT_EQ(std::vector<int>({5, 3}), combinerD0_KK.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[5], particles[3]}), combinerD0_KK.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({5, 3}), combinerD0_KK.getCurrentParticle(421, -421).getDaughterIndices());
    EXPECT_EQ(Particle::c_Unflavored, combinerD0_KK.getCurrentParticle(421, -421).getFlavorType());
    part = particles.appendNew(combinerD0_KK.getCurrentParticle(421, -421));
    inputD->addParticle(part);

    EXPECT_TRUE(combinerD0_KK.loadNext());
    EXPECT_EQ(ParticleList::c_SelfConjugatedParticle, combinerD0_KK.getCurrentType());
    EXPECT_EQ(std::vector<int>({5, 4}), combinerD0_KK.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[5], particles[4]}), combinerD0_KK.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({5, 4}), combinerD0_KK.getCurrentParticle(421, -421).getDaughterIndices());
    EXPECT_EQ(Particle::c_Unflavored, combinerD0_KK.getCurrentParticle(421, -421).getFlavorType());
    part = particles.appendNew(combinerD0_KK.getCurrentParticle(421, -421));
    inputD->addParticle(part);

    EXPECT_FALSE(combinerD0_KK.loadNext());

    ParticleCombiner combinerDStar({"D0", "pi+"}, false); //non-self-conjugated particle

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(ParticleList::c_Particle, combinerDStar.getCurrentType());
    EXPECT_EQ(std::vector<int>({8, 0}), combinerDStar.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[8], particles[0]}), combinerDStar.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({8, 0}), combinerDStar.getCurrentParticle(413, -413).getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(ParticleList::c_Particle, combinerDStar.getCurrentType());
    EXPECT_EQ(std::vector<int>({9, 0}), combinerDStar.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[9], particles[0]}), combinerDStar.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({9, 0}), combinerDStar.getCurrentParticle(413, -413).getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(ParticleList::c_Particle, combinerDStar.getCurrentType());
    EXPECT_EQ(std::vector<int>({6, 1}), combinerDStar.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[6], particles[1]}), combinerDStar.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({6, 1}), combinerDStar.getCurrentParticle(413, -413).getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(ParticleList::c_Particle, combinerDStar.getCurrentType());
    EXPECT_EQ(std::vector<int>({7, 1}), combinerDStar.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[7], particles[1]}), combinerDStar.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({7, 1}), combinerDStar.getCurrentParticle(413, -413).getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(ParticleList::c_Particle, combinerDStar.getCurrentType());
    EXPECT_EQ(std::vector<int>({11, 0}), combinerDStar.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[11], particles[0]}), combinerDStar.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({11, 0}), combinerDStar.getCurrentParticle(413, -413).getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(ParticleList::c_Particle, combinerDStar.getCurrentType());
    EXPECT_EQ(std::vector<int>({12, 0}), combinerDStar.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[12], particles[0]}), combinerDStar.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({12, 0}), combinerDStar.getCurrentParticle(413, -413).getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(ParticleList::c_Particle, combinerDStar.getCurrentType());
    EXPECT_EQ(std::vector<int>({11, 1}), combinerDStar.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[11], particles[1]}), combinerDStar.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({11, 1}), combinerDStar.getCurrentParticle(413, -413).getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(ParticleList::c_Particle, combinerDStar.getCurrentType());
    EXPECT_EQ(std::vector<int>({12, 1}), combinerDStar.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[12], particles[1]}), combinerDStar.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({12, 1}), combinerDStar.getCurrentParticle(413, -413).getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(ParticleList::c_AntiParticle, combinerDStar.getCurrentType());
    EXPECT_EQ(std::vector<int>({11, 2}), combinerDStar.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[11], particles[2]}), combinerDStar.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({11, 2}), combinerDStar.getCurrentParticle(413, -413).getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(ParticleList::c_AntiParticle, combinerDStar.getCurrentType());
    EXPECT_EQ(std::vector<int>({12, 2}), combinerDStar.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[12], particles[2]}), combinerDStar.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({12, 2}), combinerDStar.getCurrentParticle(413, -413).getDaughterIndices());

    EXPECT_FALSE(combinerDStar.loadNext());


  }

}  // namespace
