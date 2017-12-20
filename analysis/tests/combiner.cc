/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck, Anze Zupanc                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <analysis/ParticleCombiner/ParticleCombiner.h>

#include <mdst/dataobjects/ECLCluster.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/utility/EvtPDLUtil.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>

#include <utility>

#include <gtest/gtest.h>
#include <set>

using namespace std;
using namespace Belle2;

namespace {
  /** Test fixture. */
  class ParticleCombinerTest : public ::testing::Test {
  protected:
    /** register Particle array */
    virtual void SetUp()
    {
      DataStore::Instance().setInitializeActive(true);
      StoreArray<Particle> particles;
      particles.registerInDataStore();
      StoreArray<ECLCluster> eclClusters;
      eclClusters.registerInDataStore();
      DataStore::Instance().setInitializeActive(false);
    }

    /** clear datastore */
    virtual void TearDown()
    {
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
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_FlavorSpecificParticle}),
                listIndexGenerator.getCurrentIndices());
      EXPECT_FALSE(listIndexGenerator.loadNext());
    }

    {
      ListIndexGenerator listIndexGenerator;
      listIndexGenerator.init(2);
      EXPECT_TRUE(listIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_FlavorSpecificParticle, ParticleList::c_FlavorSpecificParticle }),
                listIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(listIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_SelfConjugatedParticle, ParticleList::c_FlavorSpecificParticle }),
                listIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(listIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_FlavorSpecificParticle, ParticleList::c_SelfConjugatedParticle }),
                listIndexGenerator.getCurrentIndices());
      EXPECT_FALSE(listIndexGenerator.loadNext());
    }

    {
      ListIndexGenerator listIndexGenerator;
      listIndexGenerator.init(3);
      EXPECT_TRUE(listIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_FlavorSpecificParticle, ParticleList::c_FlavorSpecificParticle, ParticleList::c_FlavorSpecificParticle }),
                listIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(listIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_SelfConjugatedParticle, ParticleList::c_FlavorSpecificParticle, ParticleList::c_FlavorSpecificParticle }),
                listIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(listIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_FlavorSpecificParticle, ParticleList::c_SelfConjugatedParticle, ParticleList::c_FlavorSpecificParticle }),
                listIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(listIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_SelfConjugatedParticle, ParticleList::c_SelfConjugatedParticle, ParticleList::c_FlavorSpecificParticle }),
                listIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(listIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_FlavorSpecificParticle, ParticleList::c_FlavorSpecificParticle, ParticleList::c_SelfConjugatedParticle }),
                listIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(listIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_SelfConjugatedParticle, ParticleList::c_FlavorSpecificParticle, ParticleList::c_SelfConjugatedParticle }),
                listIndexGenerator.getCurrentIndices());
      EXPECT_TRUE(listIndexGenerator.loadNext());
      EXPECT_EQ(std::vector<ParticleList::EParticleType>({ ParticleList::c_FlavorSpecificParticle, ParticleList::c_SelfConjugatedParticle, ParticleList::c_SelfConjugatedParticle}),
                listIndexGenerator.getCurrentIndices());
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

    typedef std::tuple<Particle::EFlavorType, int, std::set<int>> MockParticle;

  public:
    static std::string printable(const MockParticle& m)
    {
      std::string s = "MockParticle with PDG " + std::to_string(std::get<1>(m));
      s += ", type " + std::to_string(std::get<0>(m)) + ", daughter indices: ";
      for (int idx : std::get<2>(m))
        s += std::to_string(idx) + " ";
      return s;
    }

    explicit TestParticleList(std::string _decayString) : decayString(_decayString)
    {

      DecayDescriptor decaydescriptor;
      decaydescriptor.init(decayString);
      const DecayDescriptorParticle* mother = decaydescriptor.getMother();

      pdgCode = mother->getPDGCode();
      listName = mother->getFullName();

      isSelfConjugatedParticle = !(Belle2::EvtPDLUtil::hasAntiParticle(pdgCode));

      StoreObjPtr<ParticleList> list(listName);
      DataStore::Instance().setInitializeActive(true);
      list.registerInDataStore();
      DataStore::Instance().setInitializeActive(false);

      if (not list.isValid())
        list.create();
      list->initialize(pdgCode, listName);

      if (not isSelfConjugatedParticle) {
        antiListName = Belle2::EvtPDLUtil::antiParticleListName(pdgCode, mother->getLabel());
        StoreObjPtr<ParticleList> antiList(antiListName);
        DataStore::Instance().setInitializeActive(true);
        antiList.registerInDataStore();
        DataStore::Instance().setInitializeActive(false);
        if (not antiList.isValid())
          antiList.create();
        antiList->initialize(-pdgCode, antiListName);
        list->bindAntiParticleList(*(antiList));
      }
    }

    void addParticle(unsigned int mdstSource)
    {
      StoreObjPtr<ParticleList> list(listName);
      StoreArray<Particle> particles;
      Particle* part = particles.appendNew(TLorentzVector(), pdgCode,
                                           isSelfConjugatedParticle ? Particle::c_Unflavored : Particle::c_Flavored, Particle::c_MCParticle, mdstSource);
      list->addParticle(part);
    }

    void addAntiParticle(unsigned int mdstSource)
    {
      StoreObjPtr<ParticleList> list(antiListName);
      StoreArray<Particle> particles;
      Particle* part = particles.appendNew(TLorentzVector(), -pdgCode,
                                           isSelfConjugatedParticle ? Particle::c_Unflavored : Particle::c_Flavored, Particle::c_MCParticle, mdstSource);
      list->addParticle(part);
    }

    void addExpectedParticle(Particle::EFlavorType flavourType, int pdg_code, std::vector<int> daughter_indices)
    {
      expected_particles.push_back(std::make_tuple(flavourType, pdg_code, std::set<int>(daughter_indices.begin(),
                                                   daughter_indices.end())));
    }

    void addAndCheckParticlesFromGenerator()
    {

      StoreObjPtr<ParticleList> list(listName);
      StoreArray<Particle> particles;

      ParticleGenerator generator(decayString);
      generator.init();

      std::vector<MockParticle> received_particles;
      std::vector<Particle*> added_particles;
      for (unsigned int i = 0; i < expected_particles.size(); ++i) {
        bool next = generator.loadNext();
        EXPECT_TRUE(next);
        if (next) {
          const Particle& particle = generator.getCurrentParticle();
          Particle* part = particles.appendNew(particle);
          added_particles.push_back(part);
          auto daughter_indices = part->getDaughterIndices();
          received_particles.push_back(std::make_tuple(part->getFlavorType(), part->getPDGCode(), std::set<int>(daughter_indices.begin(),
                                                       daughter_indices.end())));
        }
      }
      EXPECT_FALSE(generator.loadNext());

      for (const auto& p : received_particles) {
        EXPECT_EQ(std::count(expected_particles.begin(), expected_particles.end(), p), 1) << "check failed for " << printable(p);
      }

      for (const auto& p : expected_particles) {
        EXPECT_EQ(std::count(received_particles.begin(), received_particles.end(), p), 1) << "check failed for " << printable(p);
        auto it = std::find(received_particles.begin(), received_particles.end(), p);
        if (it != received_particles.end()) {
          auto index = std::distance(received_particles.begin(), it);
          list->addParticle(added_particles[index]);
        }
      }

    }

    int operator*(int index)
    {
      StoreObjPtr<ParticleList> list(listName);
      // Check if index is smaller than FlavorSpecific+SelfConjugatedList
      // There's not possibility to check only SelfConjugated, so this has to do for the moment
      EXPECT_LT(index, list->getListSize());
      return list->getList(ParticleList::c_SelfConjugatedParticle)[index];
    }

    int operator+(int index)
    {
      StoreObjPtr<ParticleList> list(listName);
      // Check if index is smaller than FlavorSpecific+SelfConjugatedList
      // There's not possibility to check only SelfConjugated, so this has to do for the moment
      EXPECT_LT(index, list->getListSize());
      return list->getList(ParticleList::c_FlavorSpecificParticle)[index];
    }

    int operator-(int index)
    {
      StoreObjPtr<ParticleList> list(antiListName);
      // Check if index is smaller than FlavorSpecific+SelfConjugatedList
      // There's not possibility to check only SelfConjugated, so this has to do for the moment
      EXPECT_LT(index, list->getListSize());
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

  TEST_F(ParticleCombinerTest, RareBDecay)
  {
    TestParticleList e("e+");
    e.addParticle(1);
    e.addAntiParticle(2);
    e.addParticle(3);
    e.addAntiParticle(4);

    TestParticleList gamma("gamma");
    gamma.addParticle(5);
    gamma.addParticle(6);
    gamma.addParticle(7);

    TestParticleList mu("mu+");
    for (int i = 0; i < 100; ++i) {
      mu.addParticle(7 + 2 * i + 1);
      mu.addAntiParticle(7 + 2 * i + 2);
    }

    {
      TestParticleList BP("B+ -> e+");
      BP.addExpectedParticle(Particle::c_Flavored, 521, {e + 0});
      BP.addExpectedParticle(Particle::c_Flavored, 521, {e + 1});
      BP.addExpectedParticle(Particle::c_Flavored, -521, {e - 0});
      BP.addExpectedParticle(Particle::c_Flavored, -521, {e - 1});
      BP.addAndCheckParticlesFromGenerator();

      TestParticleList BP2("B+:eg -> e+ gamma");
      for (int j = 0; j < 3; ++j) {
        BP2.addExpectedParticle(Particle::c_Flavored, 521, {e + 0, gamma * j});
        BP2.addExpectedParticle(Particle::c_Flavored, 521, {e + 1, gamma * j});
        BP2.addExpectedParticle(Particle::c_Flavored, -521, {e - 0, gamma * j});
        BP2.addExpectedParticle(Particle::c_Flavored, -521, {e - 1, gamma * j});
      }
      BP2.addAndCheckParticlesFromGenerator();

      // Here we expect "few" combinations, because we combine the same list
      TestParticleList Y("Upsilon(4S) -> B+ B-");
      Y.addExpectedParticle(Particle::c_Unflavored, 300553, {BP + 0, BP - 0});
      Y.addExpectedParticle(Particle::c_Unflavored, 300553, {BP + 0, BP - 1});
      Y.addExpectedParticle(Particle::c_Unflavored, 300553, {BP + 1, BP - 0});
      Y.addExpectedParticle(Particle::c_Unflavored, 300553, {BP + 1, BP - 1});
      Y.addAndCheckParticlesFromGenerator();

      // Here we expect "more" combinations, because we combine the different list
      TestParticleList Y2("Upsilon(4S):eg -> B+:eg B-");
      for (int j = 0; j < 3; ++j) {
        Y2.addExpectedParticle(Particle::c_Unflavored, 300553, {BP2 + (0 + 2 * j), BP - 0});
        Y2.addExpectedParticle(Particle::c_Unflavored, 300553, {BP2 + (0 + 2 * j), BP - 1});
        Y2.addExpectedParticle(Particle::c_Unflavored, 300553, {BP2 + (1 + 2 * j), BP - 0});
        Y2.addExpectedParticle(Particle::c_Unflavored, 300553, {BP2 + (1 + 2 * j), BP - 1});

        Y2.addExpectedParticle(Particle::c_Unflavored, 300553, {BP2 - (0 + 2 * j), BP + 0});
        Y2.addExpectedParticle(Particle::c_Unflavored, 300553, {BP2 - (0 + 2 * j), BP + 1});
        Y2.addExpectedParticle(Particle::c_Unflavored, 300553, {BP2 - (1 + 2 * j), BP + 0});
        Y2.addExpectedParticle(Particle::c_Unflavored, 300553, {BP2 - (1 + 2 * j), BP + 1});
      }
      Y2.addAndCheckParticlesFromGenerator();

    }

    {
      TestParticleList BP("B+:mu -> mu+");
      for (int i = 0; i < 100; ++i) {
        BP.addExpectedParticle(Particle::c_Flavored, 521, {mu + i});
        BP.addExpectedParticle(Particle::c_Flavored, -521, {mu - i});
      }
      BP.addAndCheckParticlesFromGenerator();
    }
    {
      TestParticleList BP("B+:mg -> mu+ gamma");
      for (int j = 0; j < 3; ++j) {
        for (int i = 0; i < 100; ++i) {
          BP.addExpectedParticle(Particle::c_Flavored, 521, {mu + i, gamma * j});
          BP.addExpectedParticle(Particle::c_Flavored, -521, {mu - i, gamma * j});
        }
      }
      BP.addAndCheckParticlesFromGenerator();
    }

  }

  TEST_F(ParticleCombinerTest, PsiTo2D0_to2MuPlus2MuMinus)
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

    TestParticleList Psi("psi(3770) -> D0 anti-D0");
    Psi.addExpectedParticle(Particle::c_Unflavored, 30443, {D0 * 0, D0 * 2});
    Psi.addExpectedParticle(Particle::c_Unflavored, 30443, {D0 * 1, D0 * 3});
    Psi.addAndCheckParticlesFromGenerator();

    TestParticleList PsiMixed("psi(3770):mixeed -> D0 D0");
    PsiMixed.addExpectedParticle(Particle::c_Unflavored, 30443, {D0 * 0, D0 * 2});
    PsiMixed.addExpectedParticle(Particle::c_Unflavored, 30443, {D0 * 1, D0 * 3});
    PsiMixed.addAndCheckParticlesFromGenerator();

  }

  TEST_F(ParticleCombinerTest, PsiToD0D0sig)
  {
    TestParticleList mu("mu+");
    mu.addParticle(1);
    mu.addParticle(2);
    mu.addAntiParticle(3);
    mu.addAntiParticle(4);

    TestParticleList K("K+");
    K.addParticle(5);
    K.addAntiParticle(6);
    TestParticleList pi("pi+");
    pi.addParticle(7);
    pi.addAntiParticle(8);

    TestParticleList D0("D0 -> mu+ mu-");
    D0.addExpectedParticle(Particle::c_Unflavored, 421, {mu + 0, mu - 0});
    D0.addExpectedParticle(Particle::c_Unflavored, 421, {mu + 0, mu - 1});
    D0.addExpectedParticle(Particle::c_Unflavored, 421, {mu + 1, mu - 1});
    D0.addExpectedParticle(Particle::c_Unflavored, 421, {mu + 1, mu - 0});
    D0.addAndCheckParticlesFromGenerator();

    TestParticleList D0sig("D0:sig -> K+ pi-");
    D0sig.addExpectedParticle(Particle::c_Flavored, 421, {K + 0, pi - 0});
    D0sig.addExpectedParticle(Particle::c_Flavored, -421, {K - 0, pi + 0});
    D0sig.addAndCheckParticlesFromGenerator();

    TestParticleList Psi("psi(3770) -> D0:sig anti-D0");
    Psi.addExpectedParticle(Particle::c_Unflavored, 30443, {D0sig + 0, D0 * 0});
    Psi.addExpectedParticle(Particle::c_Unflavored, 30443, {D0sig + 0, D0 * 1});
    Psi.addExpectedParticle(Particle::c_Unflavored, 30443, {D0sig + 0, D0 * 2});
    Psi.addExpectedParticle(Particle::c_Unflavored, 30443, {D0sig + 0, D0 * 3});
    //also use anti-D0:sig in combination!
    Psi.addExpectedParticle(Particle::c_Unflavored, 30443, {D0sig - 0, D0 * 0});
    Psi.addExpectedParticle(Particle::c_Unflavored, 30443, {D0sig - 0, D0 * 1});
    Psi.addExpectedParticle(Particle::c_Unflavored, 30443, {D0sig - 0, D0 * 2});
    Psi.addExpectedParticle(Particle::c_Unflavored, 30443, {D0sig - 0, D0 * 3});
    Psi.addAndCheckParticlesFromGenerator();
  }

  TEST_F(ParticleCombinerTest, InputListCollisions)
  {
    // create Particle Lists
    // pi+:all
    StoreObjPtr<ParticleList> pipAll("pi+:all");
    StoreObjPtr<ParticleList> pimAll("pi-:all");
    DataStore::Instance().setInitializeActive(true);
    pipAll.registerInDataStore();
    pimAll.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    pipAll.create();
    pimAll.create();
    pipAll->initialize(211, "pi+:all");
    pimAll->initialize(-211, "pi-:all");
    pipAll->bindAntiParticleList(*(pimAll));


    // pi+:good
    StoreObjPtr<ParticleList> pipGood("pi+:good");
    StoreObjPtr<ParticleList> pimGood("pi-:good");
    DataStore::Instance().setInitializeActive(true);
    pipGood.registerInDataStore();
    pimGood.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    pipGood.create();
    pimGood.create();
    pipGood->initialize(211, "pi+:good");
    pimGood->initialize(-211, "pi-:good");
    pipGood->bindAntiParticleList(*(pimGood));

    // K+:all
    StoreObjPtr<ParticleList> kpAll("K+:all");
    StoreObjPtr<ParticleList> kmAll("K-:all");
    DataStore::Instance().setInitializeActive(true);
    kpAll.registerInDataStore();
    kmAll.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    kpAll.create();
    kmAll.create();
    kpAll->initialize(321, "K+:all");
    kmAll->initialize(-321, "K-:all");
    kpAll->bindAntiParticleList(*(kmAll));


    // K+:good
    StoreObjPtr<ParticleList> kpGood("K+:good");
    StoreObjPtr<ParticleList> kmGood("K-:good");
    DataStore::Instance().setInitializeActive(true);
    kpGood.registerInDataStore();
    kmGood.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    kpGood.create();
    kmGood.create();
    kpGood->initialize(321, "K+:good");
    kmGood->initialize(-321, "K-:good");
    kpGood->bindAntiParticleList(*(kmGood));

    // K+:good2
    StoreObjPtr<ParticleList> kpGood2("K+:good2");
    StoreObjPtr<ParticleList> kmGood2("K-:good2");
    DataStore::Instance().setInitializeActive(true);
    kpGood2.registerInDataStore();
    kmGood2.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    kpGood2.create();
    kmGood2.create();
    kpGood2->initialize(321, "K+:good2");
    kmGood2->initialize(-321, "K-:good2");
    kpGood2->bindAntiParticleList(*(kmGood2));

    // create Particle Lists
    // gamma:1
    StoreObjPtr<ParticleList> gamma_1("gamma:1");
    StoreObjPtr<ParticleList> gamma_2("gamma:2");
    DataStore::Instance().setInitializeActive(true);
    gamma_1.registerInDataStore();
    gamma_2.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    gamma_1.create();
    gamma_2.create();
    gamma_1->initialize(22, "gamma:1");
    gamma_2->initialize(22, "gamma:2");

    // Do Tests
    ParticleGenerator comb1("D0:1 -> K-:all K+:all");
    EXPECT_FALSE(comb1.inputListsCollide());
    EXPECT_FALSE(comb1.inputListsCollide(std::make_pair(0, 1)));

    ParticleGenerator comb2("D0:2 -> K-:all K+:good");
    EXPECT_TRUE(comb2.inputListsCollide());
    EXPECT_TRUE(comb2.inputListsCollide(std::make_pair(0, 1)));

    ParticleGenerator comb3("D0:3 -> K-:all K+:all pi-:all pi+:all");
    EXPECT_FALSE(comb3.inputListsCollide());
    EXPECT_FALSE(comb3.inputListsCollide(std::make_pair(0, 1)));
    EXPECT_FALSE(comb3.inputListsCollide(std::make_pair(0, 2)));
    EXPECT_FALSE(comb3.inputListsCollide(std::make_pair(0, 3)));
    EXPECT_FALSE(comb3.inputListsCollide(std::make_pair(1, 2)));
    EXPECT_FALSE(comb3.inputListsCollide(std::make_pair(1, 3)));
    EXPECT_FALSE(comb3.inputListsCollide(std::make_pair(2, 3)));

    ParticleGenerator comb4("D0:4 -> K-:all K+:good pi-:good pi+:all");
    EXPECT_TRUE(comb4.inputListsCollide());
    EXPECT_TRUE(comb4.inputListsCollide(std::make_pair(0, 1)));
    EXPECT_FALSE(comb4.inputListsCollide(std::make_pair(0, 2)));
    EXPECT_FALSE(comb4.inputListsCollide(std::make_pair(0, 3)));
    EXPECT_FALSE(comb4.inputListsCollide(std::make_pair(1, 2)));
    EXPECT_FALSE(comb4.inputListsCollide(std::make_pair(1, 3)));
    EXPECT_TRUE(comb4.inputListsCollide(std::make_pair(2, 3)));

    ParticleGenerator comb5("D+:1 -> K-:all pi+:all pi+:all");
    EXPECT_FALSE(comb5.inputListsCollide());
    EXPECT_FALSE(comb5.inputListsCollide(std::make_pair(0, 1)));
    EXPECT_FALSE(comb5.inputListsCollide(std::make_pair(0, 2)));
    EXPECT_FALSE(comb5.inputListsCollide(std::make_pair(1, 2)));

    ParticleGenerator comb6("D+:2 -> K-:all pi+:good pi+:all");
    EXPECT_TRUE(comb6.inputListsCollide());
    EXPECT_FALSE(comb6.inputListsCollide(std::make_pair(0, 1)));
    EXPECT_FALSE(comb6.inputListsCollide(std::make_pair(0, 2)));
    EXPECT_TRUE(comb6.inputListsCollide(std::make_pair(1, 2)));

    ParticleGenerator comb7("D+:3 -> K-:all K+:good K-:good2 pi+:good pi+:all");
    EXPECT_TRUE(comb7.inputListsCollide());
    EXPECT_TRUE(comb7.inputListsCollide(std::make_pair(0, 1)));
    EXPECT_TRUE(comb7.inputListsCollide(std::make_pair(0, 2)));
    EXPECT_FALSE(comb7.inputListsCollide(std::make_pair(0, 3)));
    EXPECT_FALSE(comb7.inputListsCollide(std::make_pair(0, 4)));
    EXPECT_TRUE(comb7.inputListsCollide(std::make_pair(1, 2)));
    EXPECT_FALSE(comb7.inputListsCollide(std::make_pair(1, 3)));
    EXPECT_FALSE(comb7.inputListsCollide(std::make_pair(1, 4)));
    EXPECT_FALSE(comb7.inputListsCollide(std::make_pair(2, 3)));
    EXPECT_FALSE(comb7.inputListsCollide(std::make_pair(2, 4)));
    EXPECT_TRUE(comb7.inputListsCollide(std::make_pair(3, 4)));
  }

  TEST_F(ParticleCombinerTest, FSPCopies)
  {
    // create particles
    StoreArray<Particle> particles;
    StoreArray<ECLCluster> eclClusters;

    ECLCluster* eclGamma1 = eclClusters. appendNew(ECLCluster());
    eclGamma1->setConnectedRegionId(1);
    eclGamma1->setClusterId(1);
    eclGamma1->setHypothesisId(5);
    ECLCluster* eclGamma2 = eclClusters. appendNew(ECLCluster());
    eclGamma2->setConnectedRegionId(1);
    eclGamma2->setClusterId(2);
    eclGamma2->setHypothesisId(5);
    ECLCluster* eclGamma3 = eclClusters. appendNew(ECLCluster());
    eclGamma3->setConnectedRegionId(2);
    eclGamma3->setClusterId(1);
    eclGamma3->setHypothesisId(5);
    ECLCluster* eclGamma4 = eclClusters. appendNew(ECLCluster());
    eclGamma4->setConnectedRegionId(3);
    eclGamma4->setClusterId(1);
    eclGamma4->setHypothesisId(5);
    ECLCluster* eclKL = eclClusters. appendNew(ECLCluster());
    eclKL->setConnectedRegionId(3);
    eclKL->setClusterId(1);
    eclKL->setHypothesisId(6);

    Particle* pip_1 = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0),  211, Particle::c_Flavored, Particle::c_Track, 2));
    Particle* pip_2 = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0),  211, Particle::c_Flavored, Particle::c_Track, 4));
    Particle* pip_3 = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0),  211, Particle::c_Flavored, Particle::c_Track, 6));

    Particle* pim_1 = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), -211, Particle::c_Flavored, Particle::c_Track, 1));
    Particle* pim_2 = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), -211, Particle::c_Flavored, Particle::c_Track, 3));
    Particle* pim_3 = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), -211, Particle::c_Flavored, Particle::c_Track, 5));

    Particle* kp_1 = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0),  321, Particle::c_Flavored, Particle::c_Track, 2));
    Particle* kp_2 = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0),  321, Particle::c_Flavored, Particle::c_Track, 4));
    Particle* kp_3 = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0),  321, Particle::c_Flavored, Particle::c_Track, 6));

    Particle* km_1 = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), -321, Particle::c_Flavored, Particle::c_Track, 1));
    Particle* km_2 = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), -321, Particle::c_Flavored, Particle::c_Track, 3));
    Particle* km_3 = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), -321, Particle::c_Flavored, Particle::c_Track, 5));

    //copies of FS particles
    Particle* pim_1_copy = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), -211, Particle::c_Flavored, Particle::c_Track, 1));
    Particle* pip_1_copy = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0),  211, Particle::c_Flavored, Particle::c_Track, 2));

    Particle* km_1_copy = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), -321, Particle::c_Flavored, Particle::c_Track, 1));
    Particle* kp_1_copy = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0),  321, Particle::c_Flavored, Particle::c_Track, 2));

    Particle* g_1 = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), 22, Particle::c_Unflavored, Particle::c_ECLCluster, 0));
    Particle* g_2 = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), 22, Particle::c_Unflavored, Particle::c_ECLCluster, 1));
    Particle* g_3 = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), 22, Particle::c_Unflavored, Particle::c_ECLCluster, 2));
    Particle* g_4 = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), 22, Particle::c_Unflavored, Particle::c_ECLCluster, 3));
    Particle* KL  = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), 130, Particle::c_Unflavored, Particle::c_ECLCluster, 4));


    Particle* g_1_copy = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), 22, Particle::c_Unflavored, Particle::c_ECLCluster,
                                                      0));
    Particle* g_2_copy = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), 22, Particle::c_Unflavored, Particle::c_ECLCluster,
                                                      1));
    Particle* g_3_copy = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), 22, Particle::c_Unflavored, Particle::c_ECLCluster,
                                                      2));

    // create Particle Lists
    // pi+:all
    StoreObjPtr<ParticleList> pipAll("pi+:all");
    StoreObjPtr<ParticleList> pimAll("pi-:all");
    DataStore::Instance().setInitializeActive(true);
    pipAll.registerInDataStore();
    pimAll.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    pipAll.create();
    pimAll.create();
    pipAll->initialize(211, "pi+:all");
    pimAll->initialize(-211, "pi-:all");
    pipAll->bindAntiParticleList(*(pimAll));


    // pi+:good
    StoreObjPtr<ParticleList> pipGood("pi+:good");
    StoreObjPtr<ParticleList> pimGood("pi-:good");
    DataStore::Instance().setInitializeActive(true);
    pipGood.registerInDataStore();
    pimGood.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    pipGood.create();
    pimGood.create();
    pipGood->initialize(211, "pi+:good");
    pimGood->initialize(-211, "pi-:good");
    pipGood->bindAntiParticleList(*(pimGood));

    // K+:all
    StoreObjPtr<ParticleList> kpAll("K+:all");
    StoreObjPtr<ParticleList> kmAll("K-:all");
    DataStore::Instance().setInitializeActive(true);
    kpAll.registerInDataStore();
    kmAll.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    kpAll.create();
    kmAll.create();
    kpAll->initialize(321, "K+:all");
    kmAll->initialize(-321, "K-:all");
    kpAll->bindAntiParticleList(*(kmAll));


    // K+:good
    StoreObjPtr<ParticleList> kpGood("K+:good");
    StoreObjPtr<ParticleList> kmGood("K-:good");
    DataStore::Instance().setInitializeActive(true);
    kpGood.registerInDataStore();
    kmGood.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    kpGood.create();
    kmGood.create();
    kpGood->initialize(321, "K+:good");
    kmGood->initialize(-321, "K-:good");
    kpGood->bindAntiParticleList(*(kmGood));

    // K+:good2
    StoreObjPtr<ParticleList> kpGood2("K+:good2");
    StoreObjPtr<ParticleList> kmGood2("K-:good2");
    DataStore::Instance().setInitializeActive(true);
    kpGood2.registerInDataStore();
    kmGood2.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    kpGood2.create();
    kmGood2.create();
    kpGood2->initialize(321, "K+:good2");
    kmGood2->initialize(-321, "K-:good2");
    kpGood2->bindAntiParticleList(*(kmGood2));

    // create Particle Lists
    // gamma:1
    StoreObjPtr<ParticleList> gamma_1("gamma:1");
    StoreObjPtr<ParticleList> gamma_2("gamma:2");
    DataStore::Instance().setInitializeActive(true);
    gamma_1.registerInDataStore();
    gamma_2.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    gamma_1.create();
    gamma_2.create();
    gamma_1->initialize(22, "gamma:1");
    gamma_2->initialize(22, "gamma:2");

    // Klong
    StoreObjPtr<ParticleList> klong_1("K_L0:1");
    DataStore::Instance().setInitializeActive(true);
    klong_1.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    klong_1.create();
    klong_1->initialize(130, "K_L0:1");

    // add particles to lists
    pipAll->addParticle(pip_1);
    pipAll->addParticle(pip_2);
    pipAll->addParticle(pip_3);

    pipAll->addParticle(pim_1);
    pipAll->addParticle(pim_2);
    pipAll->addParticle(pim_3);

    kpAll->addParticle(kp_1);
    kpAll->addParticle(kp_2);
    kpAll->addParticle(kp_3);

    kpAll->addParticle(km_1);
    kpAll->addParticle(km_2);
    kpAll->addParticle(km_3);

    pipGood->addParticle(pip_1_copy);
    pipGood->addParticle(pim_1_copy);

    kpGood->addParticle(kp_1_copy);
    kpGood->addParticle(km_1_copy);

    kpGood2->addParticle(kp_1);
    kpGood2->addParticle(km_1);
    kpGood2->addParticle(kp_2);
    kpGood2->addParticle(km_2);
    kpGood2->addParticle(kp_3);
    kpGood2->addParticle(km_3);

    gamma_1->addParticle(g_1);
    gamma_1->addParticle(g_2);
    gamma_1->addParticle(g_3);

    gamma_2->addParticle(g_1_copy);
    gamma_2->addParticle(g_2_copy);
    gamma_2->addParticle(g_3_copy);
    gamma_2->addParticle(g_4);

    klong_1->addParticle(KL);

    // Check consistency
    EXPECT_EQ(6, pipAll->getListSize());
    EXPECT_EQ(6, pimAll->getListSize());
    EXPECT_EQ(6, kpAll->getListSize());
    EXPECT_EQ(6, kmAll->getListSize());
    EXPECT_EQ(2, pipGood->getListSize());
    EXPECT_EQ(2, kpGood->getListSize());
    EXPECT_EQ(3, gamma_1->getListSize());
    EXPECT_EQ(4, gamma_2->getListSize());
    EXPECT_EQ(1, klong_1->getListSize());

    // check if indexToUniqueID map is properly initialized
    ParticleGenerator comb7("D+:3 -> K-:all K+:good K-:good2 pi+:good pi+:all gamma:1 gamma:2");
    comb7.init();
    std::vector<StoreObjPtr<ParticleList>> comb7PLists;
    comb7PLists.push_back(kmAll);
    comb7PLists.push_back(kpGood);
    comb7PLists.push_back(kmGood2);
    comb7PLists.push_back(pipGood);
    comb7PLists.push_back(pipAll);
    comb7PLists.push_back(gamma_1);
    comb7PLists.push_back(gamma_2);
    // each and every particle within these lists should have unique ID assigned
    // unique IDs of copies of particles should be the same
    for (unsigned i = 0; i < comb7PLists.size(); i++) {
      StoreObjPtr<ParticleList> list_i = comb7PLists[i];
      std::vector<int> iAll;
      iAll.insert(iAll.begin(), (list_i->getList(ParticleList::c_FlavorSpecificParticle, false)).begin(),
                  (list_i->getList(ParticleList::c_FlavorSpecificParticle, false)).end());
      iAll.insert(iAll.begin(), (list_i->getList(ParticleList::c_FlavorSpecificParticle,  true)).begin(),
                  (list_i->getList(ParticleList::c_FlavorSpecificParticle,  true)).end());
      iAll.insert(iAll.begin(), (list_i->getList(ParticleList::c_SelfConjugatedParticle)).begin(),
                  (list_i->getList(ParticleList::c_SelfConjugatedParticle)).end());

      for (unsigned int j = i + 1; j < comb7PLists.size(); j++) {
        //std::cout << "**** List " << i << " vs " << j << std::endl;

        StoreObjPtr<ParticleList> list_j = comb7PLists[j];
        std::vector<int> jAll;
        jAll.insert(jAll.begin(), (list_j->getList(ParticleList::c_FlavorSpecificParticle, false)).begin(),
                    (list_j->getList(ParticleList::c_FlavorSpecificParticle, false)).end());
        jAll.insert(jAll.begin(), (list_j->getList(ParticleList::c_FlavorSpecificParticle,  true)).begin(),
                    (list_j->getList(ParticleList::c_FlavorSpecificParticle,  true)).end());
        jAll.insert(jAll.begin(), (list_j->getList(ParticleList::c_SelfConjugatedParticle)).begin(),
                    (list_j->getList(ParticleList::c_SelfConjugatedParticle)).end());

        // loop over all pairs of particles within these two lists
        // this is overkill (not optimal), but for testing purposes
        // the execution time is not a concern
        for (unsigned int k = 0; k < iAll.size(); k++) {
          for (unsigned int m = 0; m < jAll.size(); m++) {
            const Particle* iP = particles[iAll[k]];
            const Particle* jP = particles[jAll[m]];

            bool copies = iP->isCopyOf(jP);

            int iID = comb7.getUniqueID(iAll[k]);
            int jID = comb7.getUniqueID(jAll[m]);

            EXPECT_TRUE(iID > 0);
            EXPECT_TRUE(jID > 0);

            //std::cout << "  -  iP/jP = " << iP->getPDGCode() << " (" << iP->getMdstSource() << ") / " << jP->getPDGCode() << " (" << jP->getMdstSource() << ") : " << iID << "/" << jID << " " << copies << std::endl;

            if (copies)
              EXPECT_TRUE(iID == jID);
            else
              EXPECT_FALSE(iID == jID);
          }
        }
      }
    }

    // create D0 lists
    StoreObjPtr<ParticleList>      D02Kpi_1("D0:kpi1");
    StoreObjPtr<ParticleList> anti_D02Kpi_1("anti-D0:kpi1");
    StoreObjPtr<ParticleList>      D02Kpi_2("D0:kpi2");
    StoreObjPtr<ParticleList> anti_D02Kpi_2("anti-D0:kpi2");
    StoreObjPtr<ParticleList>      D02KK_1("D0:kk1");
    StoreObjPtr<ParticleList> anti_D02KK_1("anti-D0:kk1");
    StoreObjPtr<ParticleList>      D02KK_2("D0:kk2");
    StoreObjPtr<ParticleList> anti_D02KK_2("anti-D0:kk2");
    StoreObjPtr<ParticleList>      D02KK_3("D0:kk3");
    StoreObjPtr<ParticleList> anti_D02KK_3("anti-D0:kk3");
    StoreObjPtr<ParticleList>      D0KLg_1("D0:klg1");
    StoreObjPtr<ParticleList>      D0KLg_2("D0:klg2");
    StoreObjPtr<ParticleList> anti_D0KLg_1("anti-D0:klg1");
    StoreObjPtr<ParticleList> anti_D0KLg_2("anti-D0:klg2");
    DataStore::Instance().setInitializeActive(true);
    D02Kpi_1.registerInDataStore();
    anti_D02Kpi_1.registerInDataStore();
    D02Kpi_2.registerInDataStore();
    anti_D02Kpi_2.registerInDataStore();
    D02KK_1.registerInDataStore();
    anti_D02KK_1.registerInDataStore();
    D02KK_2.registerInDataStore();
    anti_D02KK_2.registerInDataStore();
    D02KK_3.registerInDataStore();
    anti_D02KK_3.registerInDataStore();
    D0KLg_1.registerInDataStore();
    D0KLg_2.registerInDataStore();
    anti_D0KLg_1.registerInDataStore();
    anti_D0KLg_2.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    D02Kpi_1.create();
    anti_D02Kpi_1.create();
    D02Kpi_1->initialize(421, "D0:kpi1");
    anti_D02Kpi_1->initialize(-421, "anti-D0:kpi1");
    D02Kpi_1->bindAntiParticleList(*(anti_D02Kpi_1));

    D02Kpi_2.create();
    anti_D02Kpi_2.create();
    D02Kpi_2->initialize(421, "D0:kpi2");
    anti_D02Kpi_2->initialize(-421, "anti-D0:kpi2");
    D02Kpi_2->bindAntiParticleList(*(anti_D02Kpi_2));

    D02KK_1.create();
    anti_D02KK_1.create();
    D02KK_1->initialize(421, "D0:kk1");
    anti_D02KK_1->initialize(-421, "anti-D0:kk1");
    D02KK_1->bindAntiParticleList(*(anti_D02KK_1));

    D02KK_2.create();
    anti_D02KK_2.create();
    D02KK_2->initialize(421, "D0:kk2");
    anti_D02KK_2->initialize(-421, "anti-D0:kk2");
    D02KK_2->bindAntiParticleList(*(anti_D02KK_2));

    D02KK_3.create();
    anti_D02KK_3.create();
    D02KK_3->initialize(421, "D0:kk3");
    anti_D02KK_3->initialize(-421, "anti-D0:kk3");
    D02KK_3->bindAntiParticleList(*(anti_D02KK_3));

    D0KLg_1.create();
    D0KLg_2.create();
    anti_D0KLg_1.create();
    anti_D0KLg_2.create();
    D0KLg_1->initialize(421, "D0:klg1");
    D0KLg_2->initialize(421, "D0:klg2");
    anti_D0KLg_1->initialize(-421, "anti-D0:klg1");
    anti_D0KLg_2->initialize(-421, "anti-D0:klg2");
    D0KLg_1->bindAntiParticleList(*(anti_D0KLg_1));
    D0KLg_2->bindAntiParticleList(*(anti_D0KLg_2));
    // make combinations
    ParticleGenerator combiner_D02Kpi_1("D0:kpi1 -> K-:all pi+:all");
    combiner_D02Kpi_1.init();
    while (combiner_D02Kpi_1.loadNext()) {
      const Particle& particle = combiner_D02Kpi_1.getCurrentParticle();

      particles.appendNew(particle);
      int iparticle = particles.getEntries() - 1;

      D02Kpi_1->addParticle(iparticle, particle.getPDGCode(), particle.getFlavorType());
    }
    EXPECT_EQ(18, D02Kpi_1->getListSize());
    EXPECT_EQ(9 , D02Kpi_1->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle));
    EXPECT_EQ(9 , D02Kpi_1->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle, true));
    EXPECT_EQ(0 , D02Kpi_1->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle));

    ParticleGenerator combiner_D02Kpi_2("D0:kpi2 -> K-:all pi+:good");
    combiner_D02Kpi_2.init();
    while (combiner_D02Kpi_2.loadNext()) {
      const Particle& particle = combiner_D02Kpi_2.getCurrentParticle();

      particles.appendNew(particle);
      int iparticle = particles.getEntries() - 1;

      D02Kpi_2->addParticle(iparticle, particle.getPDGCode(), particle.getFlavorType());
    }
    EXPECT_EQ(6,  D02Kpi_2->getListSize());
    EXPECT_EQ(3 , D02Kpi_2->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle));
    EXPECT_EQ(3 , D02Kpi_2->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle, true));
    EXPECT_EQ(0 , D02Kpi_2->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle));

    ParticleGenerator combiner_D02KK_1("D0:kk1 -> K-:all K+:all");
    combiner_D02KK_1.init();
    while (combiner_D02KK_1.loadNext()) {
      const Particle& particle = combiner_D02KK_1.getCurrentParticle();

      particles.appendNew(particle);
      int iparticle = particles.getEntries() - 1;

      D02KK_1->addParticle(iparticle, particle.getPDGCode(), particle.getFlavorType());
    }
    EXPECT_EQ(9,  D02KK_1->getListSize());
    EXPECT_EQ(0 , D02KK_1->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle));
    EXPECT_EQ(0 , D02KK_1->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle, true));
    EXPECT_EQ(9 , D02KK_1->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle));
    EXPECT_EQ(9 , D02KK_1->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle, true));

    ParticleGenerator combiner_D02KK_2("D0:kk2 -> K-:all K+:good");
    combiner_D02KK_2.init();
    while (combiner_D02KK_2.loadNext()) {
      const Particle& particle = combiner_D02KK_2.getCurrentParticle();

      particles.appendNew(particle);
      int iparticle = particles.getEntries() - 1;

      D02KK_2->addParticle(iparticle, particle.getPDGCode(), particle.getFlavorType());
    }
    EXPECT_EQ(5,  D02KK_2->getListSize());
    EXPECT_EQ(0 , D02KK_2->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle));
    EXPECT_EQ(0 , D02KK_2->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle, true));
    EXPECT_EQ(5 , D02KK_2->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle));
    EXPECT_EQ(5 , D02KK_2->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle, true));

    ParticleGenerator combiner_D02KK_3("D0:kk3 -> K-:all K+:good2");
    combiner_D02KK_3.init();
    while (combiner_D02KK_3.loadNext()) {
      const Particle& particle = combiner_D02KK_3.getCurrentParticle();

      particles.appendNew(particle);
      int iparticle = particles.getEntries() - 1;

      D02KK_3->addParticle(iparticle, particle.getPDGCode(), particle.getFlavorType());
    }
    EXPECT_EQ(9,  D02KK_3->getListSize());
    EXPECT_EQ(0 , D02KK_3->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle));
    EXPECT_EQ(0 , D02KK_3->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle, true));
    EXPECT_EQ(9 , D02KK_3->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle));
    EXPECT_EQ(9 , D02KK_3->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle, true));


    ParticleGenerator combiner_D02KLg1("D0:klg1 -> K_L0:1 gamma:1");
    ParticleGenerator combiner_D02KLg2("D0:klg2 -> K_L0:1 gamma:2");
    combiner_D02KLg1.init();
    while (combiner_D02KLg1.loadNext()) {
      const Particle& particle = combiner_D02KLg1.getCurrentParticle();

      particles.appendNew(particle);
      int iparticle = particles.getEntries() - 1;

      D0KLg_1->addParticle(iparticle, particle.getPDGCode(), particle.getFlavorType());
    }
    EXPECT_EQ(3, D0KLg_1->getListSize());
    EXPECT_EQ(0 , D0KLg_1->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle));
    EXPECT_EQ(0 , D0KLg_1->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle, true));
    EXPECT_EQ(3 , D0KLg_1->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle));

    combiner_D02KLg2.init();
    while (combiner_D02KLg2.loadNext()) {
      const Particle& particle = combiner_D02KLg2.getCurrentParticle();

      particles.appendNew(particle);
      int iparticle = particles.getEntries() - 1;

      D0KLg_2->addParticle(iparticle, particle.getPDGCode(), particle.getFlavorType());
    }
    EXPECT_EQ(3, D0KLg_2->getListSize());
    EXPECT_EQ(0 , D0KLg_2->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle));
    EXPECT_EQ(0 , D0KLg_2->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle, true));
    EXPECT_EQ(3 , D0KLg_2->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle));

    // more examples
    // D+ -> K- pi+ pi+
    StoreObjPtr<ParticleList> DpKpipi_1("D+:kpipi1");
    StoreObjPtr<ParticleList> DmKpipi_1("D-:kpipi1");
    StoreObjPtr<ParticleList> DpKpipi_2("D+:kpipi2");
    StoreObjPtr<ParticleList> DmKpipi_2("D-:kpipi2");
    DataStore::Instance().setInitializeActive(true);
    DpKpipi_1.registerInDataStore();
    DmKpipi_1.registerInDataStore();
    DpKpipi_2.registerInDataStore();
    DmKpipi_2.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    DpKpipi_1.create();
    DmKpipi_1.create();
    DpKpipi_2.create();
    DmKpipi_2.create();

    DpKpipi_1->initialize(411, "D+:kpipi1");
    DmKpipi_1->initialize(-411, "D-:kpipi1");
    DpKpipi_1->bindAntiParticleList(*(DmKpipi_1));

    DpKpipi_2->initialize(411, "D+:kpipi2");
    DmKpipi_2->initialize(-411, "D-:kpipi2");
    DpKpipi_2->bindAntiParticleList(*(DmKpipi_2));

    // make combinations
    ParticleGenerator combiner_DpKpipi_1("D+:kpipi1 -> K-:all pi+:all pi+:all");
    combiner_DpKpipi_1.init();
    while (combiner_DpKpipi_1.loadNext()) {
      const Particle& particle = combiner_DpKpipi_1.getCurrentParticle();

      particles.appendNew(particle);
      int iparticle = particles.getEntries() - 1;

      DpKpipi_1->addParticle(iparticle, particle.getPDGCode(), particle.getFlavorType());
    }
    EXPECT_EQ(18, DpKpipi_1->getListSize());
    EXPECT_EQ(9 , DpKpipi_1->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle));
    EXPECT_EQ(9 , DpKpipi_1->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle, true));
    EXPECT_EQ(0 , DpKpipi_1->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle));

    ParticleGenerator combiner_DpKpipi_2("D+:kpipi1 -> K-:all pi+:all pi+:good");
    combiner_DpKpipi_2.init();
    while (combiner_DpKpipi_2.loadNext()) {
      const Particle& particle = combiner_DpKpipi_2.getCurrentParticle();

      particles.appendNew(particle);
      int iparticle = particles.getEntries() - 1;

      DpKpipi_2->addParticle(iparticle, particle.getPDGCode(), particle.getFlavorType());
    }
    EXPECT_EQ(12, DpKpipi_2->getListSize());
    EXPECT_EQ(6 , DpKpipi_2->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle));
    EXPECT_EQ(6 , DpKpipi_2->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle, true));
    EXPECT_EQ(0 , DpKpipi_2->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle));

    // D*+ -> D0 pi+
    StoreObjPtr<ParticleList> DSTp_1("D*+:D0kpi1pi");
    StoreObjPtr<ParticleList> DSTm_1("D*-:D0kpi1pi");
    StoreObjPtr<ParticleList> DSTp_2("D*+:D0kk2pi");
    StoreObjPtr<ParticleList> DSTm_2("D*-:D0kk2pi");
    DataStore::Instance().setInitializeActive(true);
    DSTp_1.registerInDataStore();
    DSTm_1.registerInDataStore();
    DSTp_2.registerInDataStore();
    DSTm_2.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    DSTp_1.create();
    DSTm_1.create();
    DSTp_2.create();
    DSTm_2.create();

    DSTp_1->initialize(413, "D*+:D0kpi1pi");
    DSTm_1->initialize(-413, "D*-:D0kpi1pi");
    DSTp_1->bindAntiParticleList(*(DSTm_1));

    DSTp_2->initialize(413, "D*+:D0kk2pi");
    DSTm_2->initialize(-413, "D*-:D0kk2pi");
    DSTp_2->bindAntiParticleList(*(DSTm_2));

    // make combinations
    ParticleGenerator combiner_DSTp_1("D*+:D0kpi1pi -> D0:kpi1 pi+:good");
    combiner_DSTp_1.init();
    while (combiner_DSTp_1.loadNext()) {
      const Particle& particle = combiner_DSTp_1.getCurrentParticle();

      particles.appendNew(particle);
      int iparticle = particles.getEntries() - 1;

      DSTp_1->addParticle(iparticle, particle.getPDGCode(), particle.getFlavorType());
    }
    EXPECT_EQ(12, DSTp_1->getListSize());
    EXPECT_EQ(6 , DSTp_1->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle));
    EXPECT_EQ(6 , DSTp_1->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle, true));
    EXPECT_EQ(0 , DSTp_1->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle));

    ParticleGenerator combiner_DSTp_2("D*+:D0kk2pi -> D0:kk2 pi+:good");
    combiner_DSTp_2.init();
    while (combiner_DSTp_2.loadNext()) {
      const Particle& particle = combiner_DSTp_2.getCurrentParticle();

      particles.appendNew(particle);
      int iparticle = particles.getEntries() - 1;

      DSTp_2->addParticle(iparticle, particle.getPDGCode(), particle.getFlavorType());
    }
    EXPECT_EQ(4, DSTp_2->getListSize());
    EXPECT_EQ(2 , DSTp_2->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle));
    EXPECT_EQ(2 , DSTp_2->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle, true));
    EXPECT_EQ(0 , DSTp_2->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle));

    // pi0:1 -> gamma:1 gamma:1
    StoreObjPtr<ParticleList> pi0_1("pi0:1");
    DataStore::Instance().setInitializeActive(true);
    pi0_1.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    pi0_1.create();

    pi0_1->initialize(111, "pi0:1");

    // make combinations
    ParticleGenerator combiner_pi0_1("pi0:1 -> gamma:1 gamma:1");
    combiner_pi0_1.init();
    while (combiner_pi0_1.loadNext()) {
      const Particle& particle = combiner_pi0_1.getCurrentParticle();

      particles.appendNew(particle);
      int iparticle = particles.getEntries() - 1;

      pi0_1->addParticle(iparticle, particle.getPDGCode(), particle.getFlavorType());
    }
    EXPECT_EQ(3, pi0_1->getListSize());
    EXPECT_EQ(0 , pi0_1->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle));
    EXPECT_EQ(0 , pi0_1->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle, true));
    EXPECT_EQ(3 , pi0_1->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle));

    // pi0:1copy -> gamma:1 gamma:1
    StoreObjPtr<ParticleList> pi0_1copy("pi0:1copy");
    DataStore::Instance().setInitializeActive(true);
    pi0_1copy.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    pi0_1copy.create();

    pi0_1copy->initialize(111, "pi0:1");

    // make combinations
    ParticleGenerator combiner_pi0_1copy("pi0:1copy -> gamma:1 gamma:1");
    combiner_pi0_1copy.init();
    while (combiner_pi0_1copy.loadNext()) {
      const Particle& particle = combiner_pi0_1copy.getCurrentParticle();

      particles.appendNew(particle);
      int iparticle = particles.getEntries() - 1;

      pi0_1copy->addParticle(iparticle, particle.getPDGCode(), particle.getFlavorType());
    }

    // pi0:2 -> gamma:1 gamma:2
    StoreObjPtr<ParticleList> pi0_2("pi0:2");
    DataStore::Instance().setInitializeActive(true);
    pi0_2.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    pi0_2.create();

    pi0_2->initialize(111, "pi0:2");

    // make combinations
    ParticleGenerator combiner_pi0_2("pi0:2 -> gamma:1 gamma:2");
    combiner_pi0_2.init();
    while (combiner_pi0_2.loadNext()) {
      const Particle& particle = combiner_pi0_2.getCurrentParticle();

      particles.appendNew(particle);
      int iparticle = particles.getEntries() - 1;

      pi0_2->addParticle(iparticle, particle.getPDGCode(), particle.getFlavorType());
    }
    EXPECT_EQ(6, pi0_2->getListSize());
    EXPECT_EQ(0 , pi0_2->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle));
    EXPECT_EQ(0 , pi0_2->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle, true));
    EXPECT_EQ(6 , pi0_2->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle));

    // eta:1 -> gamma:2 gamma:2
    StoreObjPtr<ParticleList> eta_1("eta:1");
    DataStore::Instance().setInitializeActive(true);
    eta_1.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    eta_1.create();

    eta_1->initialize(221, "eta:1");

    // make combinations
    ParticleGenerator combiner_eta_1("eta:1 -> gamma:2 gamma:2");
    combiner_eta_1.init();
    while (combiner_eta_1.loadNext()) {
      const Particle& particle = combiner_eta_1.getCurrentParticle();

      particles.appendNew(particle);
      int iparticle = particles.getEntries() - 1;

      eta_1->addParticle(iparticle, particle.getPDGCode(), particle.getFlavorType());
    }
    EXPECT_EQ(6, eta_1->getListSize());
    EXPECT_EQ(0 , eta_1->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle));
    EXPECT_EQ(0 , eta_1->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle, true));
    EXPECT_EQ(6 , eta_1->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle));

    // eta:1copy -> gamma:2 gamma:2
    StoreObjPtr<ParticleList> eta_1copy("eta:1copy");
    DataStore::Instance().setInitializeActive(true);
    eta_1copy.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    eta_1copy.create();

    eta_1copy->initialize(221, "eta:1copy");

    // make combinations
    ParticleGenerator combiner_eta_1copy("eta:1copy -> gamma:2 gamma:2");
    combiner_eta_1copy.init();
    while (combiner_eta_1copy.loadNext()) {
      const Particle& particle = combiner_eta_1copy.getCurrentParticle();

      particles.appendNew(particle);
      int iparticle = particles.getEntries() - 1;

      eta_1copy->addParticle(iparticle, particle.getPDGCode(), particle.getFlavorType());
    }

    // B0:1 -> pi0:1 eta:1
    StoreObjPtr<ParticleList>  B0_1("B0:1");
    StoreObjPtr<ParticleList> aB0_1("anti-B0:1");
    DataStore::Instance().setInitializeActive(true);
    B0_1.registerInDataStore();
    aB0_1.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    B0_1.create();
    aB0_1.create();

    B0_1->initialize(511, "B0:1");
    aB0_1->initialize(-511, "anti-B0:1");
    B0_1->bindAntiParticleList(*(aB0_1));

    ParticleGenerator combiner_B0_1("B0:1 -> pi0:1 eta:1");
    combiner_B0_1.init();
    while (combiner_B0_1.loadNext()) {
      const Particle& particle = combiner_B0_1.getCurrentParticle();

      particles.appendNew(particle);
      int iparticle = particles.getEntries() - 1;

      B0_1->addParticle(iparticle, particle.getPDGCode(), particle.getFlavorType());
    }
    EXPECT_EQ(3,   B0_1->getListSize());
    EXPECT_EQ(0 ,  B0_1->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle));
    EXPECT_EQ(0 ,  B0_1->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle, true));
    EXPECT_EQ(3 ,  B0_1->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle));
    EXPECT_EQ(3 , aB0_1->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle));

    // B0:2 -> pi0:1 pi0:1copy
    StoreObjPtr<ParticleList>  B0_2("B0:2");
    StoreObjPtr<ParticleList> aB0_2("anti-B0:2");
    DataStore::Instance().setInitializeActive(true);
    B0_2.registerInDataStore();
    aB0_2.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    B0_2.create();
    aB0_2.create();

    B0_2->initialize(511, "B0:2");
    aB0_2->initialize(-511, "anti-B0:2");
    B0_2->bindAntiParticleList(*(aB0_2));

    ParticleGenerator combiner_B0_2("B0:2 -> pi0:1 pi0:1copy");
    combiner_B0_2.init();
    while (combiner_B0_2.loadNext()) {
      const Particle& particle = combiner_B0_2.getCurrentParticle();

      particles.appendNew(particle);
      int iparticle = particles.getEntries() - 1;

      B0_2->addParticle(iparticle, particle.getPDGCode(), particle.getFlavorType());
    }
    EXPECT_EQ(0,   B0_2->getListSize());
    EXPECT_EQ(0 ,  B0_2->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle));
    EXPECT_EQ(0 ,  B0_2->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle, true));
    EXPECT_EQ(0 ,  B0_2->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle));
    EXPECT_EQ(0 , aB0_2->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle));

    // B0:3 -> eta:1 eta:1copy
    StoreObjPtr<ParticleList>  B0_3("B0:3");
    StoreObjPtr<ParticleList> aB0_3("anti-B0:3");
    DataStore::Instance().setInitializeActive(true);
    B0_3.registerInDataStore();
    aB0_3.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    B0_3.create();
    aB0_3.create();

    B0_3->initialize(511, "B0:3");
    aB0_3->initialize(-511, "anti-B0:3");
    B0_3->bindAntiParticleList(*(aB0_3));

    ParticleGenerator combiner_B0_3("B0:3 -> eta:1 eta:1copy");
    combiner_B0_3.init();
    while (combiner_B0_3.loadNext()) {
      const Particle& particle = combiner_B0_3.getCurrentParticle();

      particles.appendNew(particle);
      int iparticle = particles.getEntries() - 1;

      B0_3->addParticle(iparticle, particle.getPDGCode(), particle.getFlavorType());
    }
    EXPECT_EQ(3,   B0_3->getListSize());
    EXPECT_EQ(0 ,  B0_3->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle));
    EXPECT_EQ(0 ,  B0_3->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle, true));
    EXPECT_EQ(3 ,  B0_3->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle));
    EXPECT_EQ(3 , aB0_3->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle));

    // B0:4 -> pi0:2 eta:1
    StoreObjPtr<ParticleList>  B0_4("B0:4");
    StoreObjPtr<ParticleList> aB0_4("anti-B0:4");
    DataStore::Instance().setInitializeActive(true);
    B0_4.registerInDataStore();
    aB0_4.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    B0_4.create();
    aB0_4.create();

    B0_4->initialize(511, "B0:4");
    aB0_4->initialize(-511, "anti-B0:4");
    B0_4->bindAntiParticleList(*(aB0_4));

    ParticleGenerator combiner_B0_4("B0:4 -> pi0:2 eta:1");
    combiner_B0_4.init();
    while (combiner_B0_4.loadNext()) {
      const Particle& particle = combiner_B0_4.getCurrentParticle();

      particles.appendNew(particle);
      int iparticle = particles.getEntries() - 1;

      B0_4->addParticle(iparticle, particle.getPDGCode(), particle.getFlavorType());
    }
    EXPECT_EQ(6,   B0_4->getListSize());
    EXPECT_EQ(0 ,  B0_4->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle));
    EXPECT_EQ(0 ,  B0_4->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle, true));
    EXPECT_EQ(6 ,  B0_4->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle));
    EXPECT_EQ(6 , aB0_4->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle));

  }
}  // namespace
