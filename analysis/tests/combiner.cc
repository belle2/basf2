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
#include <analysis/ParticleCombiner/PCombinerList.h>

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

  TEST_F(ParticleCombinerTest, ListIndexGeneratorTest)
  {
    {
      ListIndexGenerator listIndexGenerator;
      listIndexGenerator.init(0);
      EXPECT_FALSE(listIndexGenerator.loadNext());
    }

    {
      ListIndexGenerator listIndexGenerator;
      listIndexGenerator.init(1);
      EXPECT_TRUE(listIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_FlavorSpecificParticle}), listIndexGenerator.getCurrentIndices());
      EXPECT_FALSE(listIndexGenerator.loadNext());
    }

    {
      ListIndexGenerator listIndexGenerator;
      listIndexGenerator.init(2);
      EXPECT_TRUE(listIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_FlavorSpecificParticle, ParticleList::c_FlavorSpecificParticle }), listIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(listIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_SelfConjugatedParticle, ParticleList::c_FlavorSpecificParticle }), listIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(listIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_FlavorSpecificParticle, ParticleList::c_SelfConjugatedParticle }), listIndexGenerator.getCurrentIndices());
      EXPECT_FALSE(listIndexGenerator.loadNext());
    }

    {
      ListIndexGenerator listIndexGenerator;
      listIndexGenerator.init(3);
      EXPECT_TRUE(listIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_FlavorSpecificParticle, ParticleList::c_FlavorSpecificParticle, ParticleList::c_FlavorSpecificParticle }), listIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(listIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_SelfConjugatedParticle, ParticleList::c_FlavorSpecificParticle, ParticleList::c_FlavorSpecificParticle }), listIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(listIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_FlavorSpecificParticle, ParticleList::c_SelfConjugatedParticle, ParticleList::c_FlavorSpecificParticle }), listIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(listIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_SelfConjugatedParticle, ParticleList::c_SelfConjugatedParticle, ParticleList::c_FlavorSpecificParticle }), listIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(listIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_FlavorSpecificParticle, ParticleList::c_FlavorSpecificParticle, ParticleList::c_SelfConjugatedParticle }), listIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(listIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_SelfConjugatedParticle, ParticleList::c_FlavorSpecificParticle, ParticleList::c_SelfConjugatedParticle }), listIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(listIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_FlavorSpecificParticle, ParticleList::c_SelfConjugatedParticle, ParticleList::c_SelfConjugatedParticle}), listIndexGenerator.getCurrentIndices());
      EXPECT_FALSE(listIndexGenerator.loadNext());
    }

  }

  TEST_F(ParticleCombinerTest, particleIndexGeneratorTest)
  {
    {
      ParticleIndexGenerator particleIndexGenerator;
      particleIndexGenerator.init(std::vector<unsigned int>({}));
      EXPECT_FALSE(particleIndexGenerator.loadNext());
    }

    {
      ParticleIndexGenerator particleIndexGenerator;
      particleIndexGenerator.init(std::vector<unsigned int>({0}));
      EXPECT_FALSE(particleIndexGenerator.loadNext());
      particleIndexGenerator.init(std::vector<unsigned int>({3}));
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({0}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({1}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({2}), particleIndexGenerator.getCurrentIndices());
      EXPECT_FALSE(particleIndexGenerator.loadNext());
    }

    {
      ParticleIndexGenerator particleIndexGenerator;
      particleIndexGenerator.init(std::vector<unsigned int>({2, 0}));
      EXPECT_FALSE(particleIndexGenerator.loadNext());

      particleIndexGenerator.init(std::vector<unsigned int>({1, 3}));
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({0, 0}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({0, 1}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({0, 2}), particleIndexGenerator.getCurrentIndices());
      EXPECT_FALSE(particleIndexGenerator.loadNext());

      particleIndexGenerator.init(std::vector<unsigned int>({2, 3}));
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({0, 0}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({1, 0}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({0, 1}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({1, 1}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({0, 2}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({1, 2}), particleIndexGenerator.getCurrentIndices());
      EXPECT_FALSE(particleIndexGenerator.loadNext());
    }

    {
      ParticleIndexGenerator particleIndexGenerator;
      particleIndexGenerator.init(std::vector<unsigned int>({2, 4, 3}));
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({0, 0, 0}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({1, 0, 0}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({0, 1, 0}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({1, 1, 0}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({0, 2, 0}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({1, 2, 0}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({0, 3, 0}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({1, 3, 0}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({0, 0, 1}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({1, 0, 1}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({0, 1, 1}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({1, 1, 1}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({0, 2, 1}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({1, 2, 1}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({0, 3, 1}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({1, 3, 1}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({0, 0, 2}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({1, 0, 2}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({0, 1, 2}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({1, 1, 2}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({0, 2, 2}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({1, 2, 2}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({0, 3, 2}), particleIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(particleIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<unsigned int>({1, 3, 2}), particleIndexGenerator.getCurrentIndices());
      EXPECT_FALSE(particleIndexGenerator.loadNext());

    }

  }

  TEST_F(ParticleCombinerTest, MuPlusMuMinus)
  {
    StoreObjPtr<ParticleList> inputA("mu+");
    StoreObjPtr<ParticleList> inputB("mu-");

    DataStore::Instance().setInitializeActive(true);
    inputA.registerAsPersistent();
    inputB.registerAsPersistent();
    DataStore::Instance().setInitializeActive(false);

    StoreArray<Particle> particles;
    particles.appendNew(TLorentzVector(), 13, Particle::c_Flavored, Particle::c_MCParticle, 0);
    particles.appendNew(TLorentzVector(), -13, Particle::c_Flavored, Particle::c_MCParticle, 1);

    inputA.create();
    inputA->initialize(13, "mu+");

    inputB.create();
    inputB->initialize(-13, "mu-");

    inputB->bindAntiParticleList(*(inputA));

    inputA->addParticle(particles[0]);
    inputA->addParticle(particles[1]);

    ParticleGenerator combiner("D0 -> mu+ mu-");
    combiner.init();
    int nCombinations = 0;
    while (combiner.loadNext()) {
      nCombinations++;
    }
    EXPECT_EQ(1, nCombinations);
  }

  TEST_F(ParticleCombinerTest, DStar)
  {
    StoreObjPtr<ParticleList> inputD("D0");
    StoreObjPtr<ParticleList> inputDbar("anti-D0");
    StoreObjPtr<ParticleList> inputKM("K-");
    StoreObjPtr<ParticleList> inputKP("K+");
    StoreObjPtr<ParticleList> inputPiP("pi+");
    StoreObjPtr<ParticleList> inputPiM("pi-");


    DataStore::Instance().setInitializeActive(true);
    inputD.registerAsPersistent();
    inputDbar.registerAsPersistent();
    inputKM.registerAsPersistent();
    inputKP.registerAsPersistent();
    inputPiP.registerAsPersistent();
    inputPiM.registerAsPersistent();
    DataStore::Instance().setInitializeActive(false);

    StoreArray<Particle> particles;
    particles.appendNew(TLorentzVector(), 211, Particle::c_Flavored, Particle::c_MCParticle, 0);
    particles.appendNew(TLorentzVector(), 211, Particle::c_Flavored, Particle::c_MCParticle, 1);
    particles.appendNew(TLorentzVector(), -211, Particle::c_Flavored, Particle::c_MCParticle, 2);

    particles.appendNew(TLorentzVector(), -321, Particle::c_Flavored, Particle::c_MCParticle, 3);
    particles.appendNew(TLorentzVector(), -321, Particle::c_Flavored, Particle::c_MCParticle, 4);
    particles.appendNew(TLorentzVector(), 321, Particle::c_Flavored, Particle::c_MCParticle, 5);

    inputKM.create();
    inputKM->initialize(-321, "K-");

    inputKP.create();
    inputKP->initialize(321, "K+");

    inputKM->bindAntiParticleList(*(inputKP));
    inputKP->addParticle(particles[3]);
    inputKP->addParticle(particles[4]);
    inputKP->addParticle(particles[5]);

    inputPiP.create();
    inputPiP->initialize(211, "pi+");
    inputPiM.create();
    inputPiM->initialize(-211, "pi-");

    inputPiP->bindAntiParticleList(*(inputPiM));

    inputPiP->addParticle(particles[0]);
    inputPiP->addParticle(particles[1]);
    inputPiP->addParticle(particles[2]);

    inputD.create();
    inputD->initialize(421, "D0");
    inputDbar.create();
    inputDbar->initialize(-421, "anti-D0");
    inputD->bindAntiParticleList(*(inputDbar));

    Particle* part = nullptr;

    ParticleGenerator combinerD0("D0 -> K- pi+");
    combinerD0.init();

    EXPECT_TRUE(combinerD0.loadNext()); // 6
    EXPECT_EQ(std::vector<int>({5, 2}), combinerD0.getCurrentParticle().getDaughterIndices());
    EXPECT_EQ(Particle::c_Flavored, combinerD0.getCurrentParticle().getFlavorType());
    part = particles.appendNew(combinerD0.getCurrentParticle());
    inputD->addParticle(part);

    EXPECT_TRUE(combinerD0.loadNext());  // 7
    EXPECT_EQ(std::vector<int>({3, 0}), combinerD0.getCurrentParticle().getDaughterIndices());
    EXPECT_EQ(Particle::c_Flavored, combinerD0.getCurrentParticle().getFlavorType());
    part = particles.appendNew(combinerD0.getCurrentParticle());
    inputD->addParticle(part);

    EXPECT_TRUE(combinerD0.loadNext());  // 8
    EXPECT_EQ(std::vector<int>({4, 0}), combinerD0.getCurrentParticle().getDaughterIndices());
    EXPECT_EQ(Particle::c_Flavored, combinerD0.getCurrentParticle().getFlavorType());
    part = particles.appendNew(combinerD0.getCurrentParticle());
    inputD->addParticle(part);

    EXPECT_TRUE(combinerD0.loadNext());  // 9
    EXPECT_EQ(std::vector<int>({3, 1}), combinerD0.getCurrentParticle().getDaughterIndices());
    EXPECT_EQ(Particle::c_Flavored, combinerD0.getCurrentParticle().getFlavorType());
    part = particles.appendNew(combinerD0.getCurrentParticle());
    inputD->addParticle(part);

    EXPECT_TRUE(combinerD0.loadNext());  // 10
    EXPECT_EQ(std::vector<int>({4, 1}), combinerD0.getCurrentParticle().getDaughterIndices());
    EXPECT_EQ(Particle::c_Flavored, combinerD0.getCurrentParticle().getFlavorType());
    part = particles.appendNew(combinerD0.getCurrentParticle());
    inputD->addParticle(part);


    EXPECT_FALSE(combinerD0.loadNext());

    ParticleGenerator combinerD0_KK("D0 -> K+ K-"); //non-self-conjugated particle
    combinerD0_KK.init();

    EXPECT_TRUE(combinerD0_KK.loadNext()); // 11
    EXPECT_EQ(std::vector<int>({5, 3}), combinerD0_KK.getCurrentParticle().getDaughterIndices());
    EXPECT_EQ(Particle::c_Unflavored, combinerD0_KK.getCurrentParticle().getFlavorType());
    part = particles.appendNew(combinerD0_KK.getCurrentParticle());
    inputD->addParticle(part);

    EXPECT_TRUE(combinerD0_KK.loadNext()); // 12
    EXPECT_EQ(std::vector<int>({5, 4}), combinerD0_KK.getCurrentParticle().getDaughterIndices());
    EXPECT_EQ(Particle::c_Unflavored, combinerD0_KK.getCurrentParticle().getFlavorType());
    part = particles.appendNew(combinerD0_KK.getCurrentParticle());
    inputD->addParticle(part);

    EXPECT_FALSE(combinerD0_KK.loadNext());

    ParticleGenerator combinerDStar("D*+ -> D0 pi+");
    combinerDStar.init();

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(std::vector<int>({11, 2}), combinerDStar.getCurrentParticle().getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(std::vector<int>({12, 2}), combinerDStar.getCurrentParticle().getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(std::vector<int>({9, 0}), combinerDStar.getCurrentParticle().getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(std::vector<int>({10, 0}), combinerDStar.getCurrentParticle().getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(std::vector<int>({7, 1}), combinerDStar.getCurrentParticle().getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(std::vector<int>({8, 1}), combinerDStar.getCurrentParticle().getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(std::vector<int>({11, 0}), combinerDStar.getCurrentParticle().getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(std::vector<int>({12, 0}), combinerDStar.getCurrentParticle().getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(std::vector<int>({11, 1}), combinerDStar.getCurrentParticle().getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(std::vector<int>({12, 1}), combinerDStar.getCurrentParticle().getDaughterIndices());

    EXPECT_FALSE(combinerDStar.loadNext());

  }

}  // namespace
