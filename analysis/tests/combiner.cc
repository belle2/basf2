/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <analysis/ParticleCombiner/ParticleCombiner.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/utility/EvtPDLUtil.h>

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


  class TestParticleList {

    typedef std::tuple<Particle::EFlavorType, int, std::vector<int>> MockParticle;

  public:
    TestParticleList(std::string decayString) : decayString(decayString) {

      DecayDescriptor decaydescriptor;
      decaydescriptor.init(decayString);
      const DecayDescriptorParticle* mother = decaydescriptor.getMother();

      pdgCode = mother->getPDGCode();
      listName = mother->getFullName();

      isSelfConjugatedParticle = !(Belle2::EvtPDLUtil::hasAntiParticle(pdgCode));

      StoreObjPtr<ParticleList> list(listName);
      DataStore::Instance().setInitializeActive(true);
      list.registerAsPersistent();
      DataStore::Instance().setInitializeActive(false);

      if (not list.isValid())
        list.create();
      list->initialize(pdgCode, listName);

      if (not isSelfConjugatedParticle) {
        antiListName = Belle2::EvtPDLUtil::antiParticleListName(pdgCode, mother->getLabel());
        StoreObjPtr<ParticleList> antiList(antiListName);
        DataStore::Instance().setInitializeActive(true);
        antiList.registerAsPersistent();
        DataStore::Instance().setInitializeActive(false);
        if (not antiList.isValid())
          antiList.create();
        antiList->initialize(-pdgCode, antiListName);
        list->bindAntiParticleList(*(antiList));
      }
    }

    void addParticle(unsigned int mdstSource) {
      StoreObjPtr<ParticleList> list(listName);
      StoreArray<Particle> particles;
      Particle* part = particles.appendNew(TLorentzVector(), pdgCode, isSelfConjugatedParticle ? Particle::c_Unflavored : Particle::c_Flavored, Particle::c_MCParticle, mdstSource);
      list->addParticle(part);
    }

    void addAntiParticle(unsigned int mdstSource) {
      StoreObjPtr<ParticleList> list(antiListName);
      StoreArray<Particle> particles;
      Particle* part = particles.appendNew(TLorentzVector(), -pdgCode, isSelfConjugatedParticle ? Particle::c_Unflavored : Particle::c_Flavored, Particle::c_MCParticle, mdstSource);
      list->addParticle(part);
    }

    void addExpectedParticle(Particle::EFlavorType flavourType, int pdg_code, std::vector<int> daughter_indices) {
      expected_particles.push_back(std::make_tuple(flavourType, pdg_code, daughter_indices));
    }

    void addAndCheckParticlesFromGenerator() {

      StoreObjPtr<ParticleList> list(listName);
      StoreArray<Particle> particles;

      ParticleGenerator generator(decayString);
      generator.init();

      std::vector<MockParticle> received_particles;
      std::vector<Particle*> added_particles;
      for (unsigned int i = 0; i < expected_particles.size(); ++i) {
        EXPECT_TRUE(generator.loadNext());
        const Particle& particle = generator.getCurrentParticle();
        Particle* part = particles.appendNew(particle);
        added_particles.push_back(part);
        received_particles.push_back(std::make_tuple(part->getFlavorType(), part->getPDGCode(), part->getDaughterIndices()));
      }
      EXPECT_FALSE(generator.loadNext());

      for (const auto & p : received_particles) {
        EXPECT_EQ(std::count(expected_particles.begin(), expected_particles.end(), p), 1);
      }

      for (const auto & p : expected_particles) {
        EXPECT_EQ(std::count(received_particles.begin(), received_particles.end(), p), 1);
        auto it = std::find(received_particles.begin(), received_particles.end(), p);
        if (it != received_particles.end()) {
          auto index = std::distance(received_particles.begin(), it);
          list->addParticle(added_particles[index]);
        }
      }

    }

    int operator*(int index) {
      StoreObjPtr<ParticleList> list(listName);
      return list->getList(ParticleList::c_SelfConjugatedParticle)[index];
    }

    int operator+(int index) {
      StoreObjPtr<ParticleList> list(listName);
      return list->getList(ParticleList::c_FlavorSpecificParticle)[index];
    }

    int operator-(int index) {
      StoreObjPtr<ParticleList> list(antiListName);
      return list->getList(ParticleList::c_FlavorSpecificParticle)[index];
    }

  private:
    int pdgCode;
    std::string decayString;
    std::string listName;
    std::string antiListName;
    bool isSelfConjugatedParticle;
    std::vector<MockParticle> expected_particles;

  };

  TEST_F(ParticleCombinerTest, DStar)
  {
    TestParticleList K("K+");
    K.addParticle(1);
    K.addAntiParticle(2);
    K.addAntiParticle(3);

    TestParticleList pi("pi+");
    pi.addParticle(4);
    pi.addParticle(5);
    pi.addAntiParticle(6);

    {
      TestParticleList D0("D0 -> K- pi+");
      D0.addExpectedParticle(Particle::c_Flavored, 421, {K - 0, pi + 0});
      D0.addExpectedParticle(Particle::c_Flavored, 421, {K - 0, pi + 1});
      D0.addExpectedParticle(Particle::c_Flavored, 421, {K - 1, pi + 0});
      D0.addExpectedParticle(Particle::c_Flavored, 421, {K - 1, pi + 1});
      D0.addExpectedParticle(Particle::c_Flavored, -421, {K + 0, pi - 0});
      D0.addAndCheckParticlesFromGenerator();
    }

    {
      TestParticleList D0("D0 -> K- K+");
      D0.addExpectedParticle(Particle::c_Unflavored, 421, {K - 0, K + 0});
      D0.addExpectedParticle(Particle::c_Unflavored, 421, {K - 1, K + 0});
      D0.addAndCheckParticlesFromGenerator();
    }

    TestParticleList D0("D0");

    TestParticleList DS("D*+ -> D0 pi+");
    DS.addExpectedParticle(Particle::c_Flavored, 413, {D0 + 0, pi + 1});
    DS.addExpectedParticle(Particle::c_Flavored, 413, {D0 + 1, pi + 0});
    DS.addExpectedParticle(Particle::c_Flavored, 413, {D0 + 2, pi + 1});
    DS.addExpectedParticle(Particle::c_Flavored, 413, {D0 + 3, pi + 0});
    DS.addExpectedParticle(Particle::c_Flavored, 413, {D0 * 0, pi + 0});
    DS.addExpectedParticle(Particle::c_Flavored, 413, {D0 * 0, pi + 1});
    DS.addExpectedParticle(Particle::c_Flavored, 413, {D0 * 1, pi + 0});
    DS.addExpectedParticle(Particle::c_Flavored, 413, {D0 * 1, pi + 1});

    DS.addExpectedParticle(Particle::c_Flavored, -413, {D0 * 0, pi - 0});
    DS.addExpectedParticle(Particle::c_Flavored, -413, {D0 * 1, pi - 0});

    DS.addAndCheckParticlesFromGenerator();

  }

  TEST_F(ParticleCombinerTest, MuPlusMuMinus)
  {

    TestParticleList mu("mu+");
    mu.addParticle(1);
    mu.addParticle(2);
    mu.addAntiParticle(3);
    mu.addAntiParticle(4);

    TestParticleList D0("D0 -> mu+ mu-");
    D0.addExpectedParticle(Particle::c_Unflavored, 421, {mu + 0, mu - 0});
    D0.addExpectedParticle(Particle::c_Unflavored, 421, {mu + 0, mu - 1});
    D0.addExpectedParticle(Particle::c_Unflavored, 421, {mu + 1, mu - 1});
    D0.addExpectedParticle(Particle::c_Unflavored, 421, {mu + 1, mu - 0});
    D0.addAndCheckParticlesFromGenerator();


  }

}  // namespace
