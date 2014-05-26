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

    /** convert ParticleList to PCombinerList */
    virtual void convert(const StoreObjPtr<ParticleList>& in, PCombinerList& out) {
      std::vector<int> particles     = in->getList(ParticleList::c_FlavorSpecificParticle);
      std::vector<int> antiParticles = in->getList(ParticleList::c_FlavorSpecificParticle, true);
      std::vector<int> scParticles   = in->getList(ParticleList::c_SelfConjugatedParticle);

      out.setPDG(in->getPDGCode());
      out.setList(PCombinerList::c_Particle, particles);
      out.setList(PCombinerList::c_AntiParticle, antiParticles);
      out.setList(PCombinerList::c_SelfConjugatedParticle, scParticles);
    }
  };

  TEST_F(ParticleCombinerTest, ListCombinerTest)
  {
    {
      ListCombiner listCombiner(0);
      listCombiner.init(PCombinerList::c_Particle);
      EXPECT_FALSE(listCombiner.loadNext());
      listCombiner.init(PCombinerList::c_AntiParticle);
      EXPECT_FALSE(listCombiner.loadNext());
      listCombiner.init(PCombinerList::c_SelfConjugatedParticle);
      EXPECT_FALSE(listCombiner.loadNext());
    }

    {
      ListCombiner listCombiner(1);
      listCombiner.init(PCombinerList::c_Particle);
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<PCombinerList::EParticleType>({ PCombinerList::c_Particle}), listCombiner.getCurrentTypes());
      EXPECT_FALSE(listCombiner.loadNext());
      listCombiner.init(PCombinerList::c_AntiParticle);
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<PCombinerList::EParticleType>({ PCombinerList::c_AntiParticle}), listCombiner.getCurrentTypes());
      EXPECT_FALSE(listCombiner.loadNext());
      listCombiner.init(PCombinerList::c_SelfConjugatedParticle);
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<PCombinerList::EParticleType>({ PCombinerList::c_SelfConjugatedParticle}), listCombiner.getCurrentTypes());
      EXPECT_FALSE(listCombiner.loadNext());
    }

    {
      ListCombiner listCombiner(2);
      listCombiner.init(PCombinerList::c_Particle);
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<PCombinerList::EParticleType>({ PCombinerList::c_Particle, PCombinerList::c_Particle}), listCombiner.getCurrentTypes());
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<PCombinerList::EParticleType>({ PCombinerList::c_SelfConjugatedParticle, PCombinerList::c_Particle}), listCombiner.getCurrentTypes());
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<PCombinerList::EParticleType>({ PCombinerList::c_Particle, PCombinerList::c_SelfConjugatedParticle}), listCombiner.getCurrentTypes());
      EXPECT_FALSE(listCombiner.loadNext());
      listCombiner.init(PCombinerList::c_AntiParticle);
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<PCombinerList::EParticleType>({ PCombinerList::c_AntiParticle, PCombinerList::c_AntiParticle}), listCombiner.getCurrentTypes());
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<PCombinerList::EParticleType>({ PCombinerList::c_SelfConjugatedParticle, PCombinerList::c_AntiParticle}), listCombiner.getCurrentTypes());
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<PCombinerList::EParticleType>({ PCombinerList::c_AntiParticle, PCombinerList::c_SelfConjugatedParticle}), listCombiner.getCurrentTypes());
      EXPECT_FALSE(listCombiner.loadNext());
      listCombiner.init(PCombinerList::c_SelfConjugatedParticle);
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<PCombinerList::EParticleType>({ PCombinerList::c_SelfConjugatedParticle, PCombinerList::c_SelfConjugatedParticle}), listCombiner.getCurrentTypes());
      EXPECT_FALSE(listCombiner.loadNext());
    }

    {
      ListCombiner listCombiner(3);
      listCombiner.init(PCombinerList::c_Particle);
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<PCombinerList::EParticleType>({ PCombinerList::c_Particle, PCombinerList::c_Particle, PCombinerList::c_Particle }), listCombiner.getCurrentTypes());
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<PCombinerList::EParticleType>({ PCombinerList::c_SelfConjugatedParticle, PCombinerList::c_Particle, PCombinerList::c_Particle }), listCombiner.getCurrentTypes());
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<PCombinerList::EParticleType>({ PCombinerList::c_Particle, PCombinerList::c_SelfConjugatedParticle, PCombinerList::c_Particle }), listCombiner.getCurrentTypes());
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<PCombinerList::EParticleType>({ PCombinerList::c_SelfConjugatedParticle, PCombinerList::c_SelfConjugatedParticle, PCombinerList::c_Particle }), listCombiner.getCurrentTypes());
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<PCombinerList::EParticleType>({ PCombinerList::c_Particle, PCombinerList::c_Particle, PCombinerList::c_SelfConjugatedParticle }), listCombiner.getCurrentTypes());
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<PCombinerList::EParticleType>({ PCombinerList::c_SelfConjugatedParticle, PCombinerList::c_Particle, PCombinerList::c_SelfConjugatedParticle }), listCombiner.getCurrentTypes());
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<PCombinerList::EParticleType>({ PCombinerList::c_Particle, PCombinerList::c_SelfConjugatedParticle, PCombinerList::c_SelfConjugatedParticle }), listCombiner.getCurrentTypes());
      EXPECT_FALSE(listCombiner.loadNext());

      listCombiner.init(PCombinerList::c_AntiParticle);
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<PCombinerList::EParticleType>({ PCombinerList::c_AntiParticle, PCombinerList::c_AntiParticle, PCombinerList::c_AntiParticle }), listCombiner.getCurrentTypes());
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<PCombinerList::EParticleType>({ PCombinerList::c_SelfConjugatedParticle, PCombinerList::c_AntiParticle, PCombinerList::c_AntiParticle }), listCombiner.getCurrentTypes());
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<PCombinerList::EParticleType>({ PCombinerList::c_AntiParticle, PCombinerList::c_SelfConjugatedParticle, PCombinerList::c_AntiParticle }), listCombiner.getCurrentTypes());
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<PCombinerList::EParticleType>({ PCombinerList::c_SelfConjugatedParticle, PCombinerList::c_SelfConjugatedParticle, PCombinerList::c_AntiParticle }), listCombiner.getCurrentTypes());
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<PCombinerList::EParticleType>({ PCombinerList::c_AntiParticle, PCombinerList::c_AntiParticle, PCombinerList::c_SelfConjugatedParticle }), listCombiner.getCurrentTypes());
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<PCombinerList::EParticleType>({ PCombinerList::c_SelfConjugatedParticle, PCombinerList::c_AntiParticle, PCombinerList::c_SelfConjugatedParticle }), listCombiner.getCurrentTypes());
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<PCombinerList::EParticleType>({ PCombinerList::c_AntiParticle, PCombinerList::c_SelfConjugatedParticle, PCombinerList::c_SelfConjugatedParticle }), listCombiner.getCurrentTypes());
      EXPECT_FALSE(listCombiner.loadNext());

      listCombiner.init(PCombinerList::c_SelfConjugatedParticle);
      EXPECT_TRUE(listCombiner.loadNext());
      EXPECT_EQ(std::vector<PCombinerList::EParticleType>({ PCombinerList::c_SelfConjugatedParticle, PCombinerList::c_SelfConjugatedParticle, PCombinerList::c_SelfConjugatedParticle }), listCombiner.getCurrentTypes());
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
    inputA->initialize(13, "a");

    inputB.create();
    inputB->initialize(-13, "b");

    inputB->bindAntiParticleList(*(inputA));

    inputA->addParticle(particles[0]);
    inputA->addParticle(particles[1]);

    // Convert input ParticleList(s) to PCombinerList(s)
    vector<PCombinerList> inputPCombinerLists;
    PCombinerList plistA;
    PCombinerList plistB;

    convert(inputA, plistA);
    convert(inputB, plistB);

    inputPCombinerLists.push_back(plistA);
    inputPCombinerLists.push_back(plistB);
    ParticleCombiner combiner(inputPCombinerLists, true); //self-conjugated decay
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
    StoreObjPtr<ParticleList> inputPiP("pi+");
    StoreObjPtr<ParticleList> inputPiM("pi-");


    DataStore::Instance().setInitializeActive(true);
    inputD.registerAsPersistent();
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
    Particle* part = nullptr;

    // Convert input ParticleList(s) to PCombinerList(s)
    vector<PCombinerList> inputPCombinerListsD0;
    vector<PCombinerList> inputPCombinerListsD0_KK;

    PCombinerList plistKP;
    PCombinerList plistKM;
    PCombinerList plistPiP;

    convert(inputKP,  plistKP);
    convert(inputKP,  plistKM);
    convert(inputPiP, plistPiP);

    inputPCombinerListsD0.push_back(plistKM);
    inputPCombinerListsD0.push_back(plistPiP);
    inputPCombinerListsD0_KK.push_back(plistKP);
    inputPCombinerListsD0_KK.push_back(plistKM);

    ParticleCombiner combinerD0(inputPCombinerListsD0, false); //non-self-conjugated decay

    EXPECT_TRUE(combinerD0.loadNext());
    EXPECT_EQ(PCombinerList::c_Particle, combinerD0.getCurrentType());
    EXPECT_EQ(std::vector<int>({3, 0}), combinerD0.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[3], particles[0]}), combinerD0.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({3, 0}), combinerD0.getCurrentParticle(421, -421).getDaughterIndices());
    EXPECT_EQ(Particle::c_Flavored, combinerD0.getCurrentParticle(421, -421).getFlavorType());
    part = particles.appendNew(combinerD0.getCurrentParticle(421, -421));
    inputD->addParticle(part);

    EXPECT_TRUE(combinerD0.loadNext());
    EXPECT_EQ(PCombinerList::c_Particle, combinerD0.getCurrentType());
    EXPECT_EQ(std::vector<int>({4, 0}), combinerD0.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[4], particles[0]}), combinerD0.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({4, 0}), combinerD0.getCurrentParticle(421, -421).getDaughterIndices());
    EXPECT_EQ(Particle::c_Flavored, combinerD0.getCurrentParticle(421, -421).getFlavorType());
    part = particles.appendNew(combinerD0.getCurrentParticle(421, -421));
    inputD->addParticle(part);

    EXPECT_TRUE(combinerD0.loadNext());
    EXPECT_EQ(PCombinerList::c_Particle, combinerD0.getCurrentType());
    EXPECT_EQ(std::vector<int>({3, 1}), combinerD0.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[3], particles[1]}), combinerD0.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({3, 1}), combinerD0.getCurrentParticle(421, -421).getDaughterIndices());
    EXPECT_EQ(Particle::c_Flavored, combinerD0.getCurrentParticle(421, -421).getFlavorType());
    part = particles.appendNew(combinerD0.getCurrentParticle(421, -421));
    inputD->addParticle(part);

    EXPECT_TRUE(combinerD0.loadNext());
    EXPECT_EQ(PCombinerList::c_Particle, combinerD0.getCurrentType());
    EXPECT_EQ(std::vector<int>({4, 1}), combinerD0.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[4], particles[1]}), combinerD0.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({4, 1}), combinerD0.getCurrentParticle(421, -421).getDaughterIndices());
    EXPECT_EQ(Particle::c_Flavored, combinerD0.getCurrentParticle(421, -421).getFlavorType());
    part = particles.appendNew(combinerD0.getCurrentParticle(421, -421));
    inputD->addParticle(part);

    EXPECT_TRUE(combinerD0.loadNext());
    EXPECT_EQ(PCombinerList::c_AntiParticle, combinerD0.getCurrentType());
    EXPECT_EQ(std::vector<int>({5, 2}), combinerD0.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[5], particles[2]}), combinerD0.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({5, 2}), combinerD0.getCurrentParticle(421, -421).getDaughterIndices());
    EXPECT_EQ(Particle::c_Flavored, combinerD0.getCurrentParticle(421, -421).getFlavorType());
    part = particles.appendNew(combinerD0.getCurrentParticle(421, -421));
    inputD->addParticle(part);

    EXPECT_FALSE(combinerD0.loadNext());

    ParticleCombiner combinerD0_KK(inputPCombinerListsD0_KK, false); //non-self-conjugated particle

    EXPECT_TRUE(combinerD0_KK.loadNext());
    EXPECT_EQ(PCombinerList::c_SelfConjugatedParticle, combinerD0_KK.getCurrentType());
    EXPECT_EQ(std::vector<int>({5, 3}), combinerD0_KK.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[5], particles[3]}), combinerD0_KK.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({5, 3}), combinerD0_KK.getCurrentParticle(421, -421).getDaughterIndices());
    EXPECT_EQ(Particle::c_Unflavored, combinerD0_KK.getCurrentParticle(421, -421).getFlavorType());
    part = particles.appendNew(combinerD0_KK.getCurrentParticle(421, -421));
    inputD->addParticle(part);

    EXPECT_TRUE(combinerD0_KK.loadNext());
    EXPECT_EQ(PCombinerList::c_SelfConjugatedParticle, combinerD0_KK.getCurrentType());
    EXPECT_EQ(std::vector<int>({5, 4}), combinerD0_KK.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[5], particles[4]}), combinerD0_KK.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({5, 4}), combinerD0_KK.getCurrentParticle(421, -421).getDaughterIndices());
    EXPECT_EQ(Particle::c_Unflavored, combinerD0_KK.getCurrentParticle(421, -421).getFlavorType());
    part = particles.appendNew(combinerD0_KK.getCurrentParticle(421, -421));
    inputD->addParticle(part);

    EXPECT_FALSE(combinerD0_KK.loadNext());

    /*
    //TODO: fix this
    ParticleCombiner combinerDStar({"D0", "pi+"}, false); //non-self-conjugated particle

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(PCombinerList::c_Particle, combinerDStar.getCurrentType());
    EXPECT_EQ(std::vector<int>({8, 0}), combinerDStar.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[8], particles[0]}), combinerDStar.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({8, 0}), combinerDStar.getCurrentParticle(413, -413).getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(PCombinerList::c_Particle, combinerDStar.getCurrentType());
    EXPECT_EQ(std::vector<int>({9, 0}), combinerDStar.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[9], particles[0]}), combinerDStar.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({9, 0}), combinerDStar.getCurrentParticle(413, -413).getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(PCombinerList::c_Particle, combinerDStar.getCurrentType());
    EXPECT_EQ(std::vector<int>({6, 1}), combinerDStar.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[6], particles[1]}), combinerDStar.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({6, 1}), combinerDStar.getCurrentParticle(413, -413).getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(PCombinerList::c_Particle, combinerDStar.getCurrentType());
    EXPECT_EQ(std::vector<int>({7, 1}), combinerDStar.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[7], particles[1]}), combinerDStar.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({7, 1}), combinerDStar.getCurrentParticle(413, -413).getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(PCombinerList::c_Particle, combinerDStar.getCurrentType());
    EXPECT_EQ(std::vector<int>({11, 0}), combinerDStar.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[11], particles[0]}), combinerDStar.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({11, 0}), combinerDStar.getCurrentParticle(413, -413).getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(PCombinerList::c_Particle, combinerDStar.getCurrentType());
    EXPECT_EQ(std::vector<int>({12, 0}), combinerDStar.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[12], particles[0]}), combinerDStar.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({12, 0}), combinerDStar.getCurrentParticle(413, -413).getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(PCombinerList::c_Particle, combinerDStar.getCurrentType());
    EXPECT_EQ(std::vector<int>({11, 1}), combinerDStar.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[11], particles[1]}), combinerDStar.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({11, 1}), combinerDStar.getCurrentParticle(413, -413).getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(PCombinerList::c_Particle, combinerDStar.getCurrentType());
    EXPECT_EQ(std::vector<int>({12, 1}), combinerDStar.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[12], particles[1]}), combinerDStar.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({12, 1}), combinerDStar.getCurrentParticle(413, -413).getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(PCombinerList::c_AntiParticle, combinerDStar.getCurrentType());
    EXPECT_EQ(std::vector<int>({11, 2}), combinerDStar.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[11], particles[2]}), combinerDStar.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({11, 2}), combinerDStar.getCurrentParticle(413, -413).getDaughterIndices());

    EXPECT_TRUE(combinerDStar.loadNext());
    EXPECT_EQ(PCombinerList::c_AntiParticle, combinerDStar.getCurrentType());
    EXPECT_EQ(std::vector<int>({12, 2}), combinerDStar.getCurrentIndices());
    EXPECT_EQ(std::vector<Particle*>({ particles[12], particles[2]}), combinerDStar.getCurrentParticles());
    EXPECT_EQ(std::vector<int>({12, 2}), combinerDStar.getCurrentParticle(413, -413).getDaughterIndices());

    EXPECT_FALSE(combinerDStar.loadNext());
    */

  }

}  // namespace
