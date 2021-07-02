/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/V0.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Const.h>
#include <framework/utilities/TestHelpers.h>

#include <analysis/utility/ParticleCopy.h>

#include <TDatabasePDG.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::ParticleCopy;

namespace {
  /** Test fixture. */
  class ParticleTest : public ::testing::Test {
  protected:
    /** register Particle array + ParticleExtraInfoMap object. */
    void SetUp() override
    {
      DataStore::Instance().setInitializeActive(true);
      StoreObjPtr<ParticleExtraInfoMap> particleExtraInfo;
      StoreArray<Particle> particles;
      StoreArray<MCParticle> mcparticles;
      StoreArray<RestOfEvent> roes;
      StoreArray<ECLCluster> eclClusters;
      StoreArray<KLMCluster> klmClusters;
      StoreArray<V0> v0s;
      particleExtraInfo.registerInDataStore();
      particles.registerInDataStore();
      mcparticles.registerInDataStore();
      eclClusters.registerInDataStore();
      roes.registerInDataStore();
      klmClusters.registerInDataStore();
      v0s.registerInDataStore();
      particles.registerRelationTo(mcparticles);
      particles.registerRelationTo(roes);
      DataStore::Instance().setInitializeActive(false);
    }

    /** clear datastore */
    void TearDown() override
    {
      DataStore::Instance().reset();
    }
  };


  TEST_F(ParticleTest, Constructors)
  {
    {
      Particle p;
      EXPECT_EQ(0, p.getPDGCode());
      EXPECT_TRUE(TLorentzVector(0, 0, 0, 0) == p.get4Vector());
      EXPECT_EQ(Particle::c_Undefined, p.getParticleSource());
    }
    {
      TLorentzVector momentum(1, 2, 3, 4);
      Particle p(momentum, 421);
      EXPECT_EQ(421, p.getPDGCode());
      EXPECT_FLOAT_EQ(0.0, momentum.DeltaPhi(p.get4Vector()));
      EXPECT_FLOAT_EQ(0.0, momentum.DeltaR(p.get4Vector()));
      EXPECT_FLOAT_EQ(momentum.Energy(), p.get4Vector().Energy());
      EXPECT_FLOAT_EQ(momentum.Energy(), p.getEnergy());
      EXPECT_FLOAT_EQ(momentum.M(), p.getMass());
      EXPECT_EQ(Particle::c_Undefined, p.getParticleSource());
    }
    {
      TLorentzVector momentum(1, 2, 3, 4);
      Particle p(momentum, 22, Particle::c_Unflavored, Particle::c_MCParticle, 123);
      EXPECT_EQ(22, p.getPDGCode());
      EXPECT_FLOAT_EQ(0.0, momentum.DeltaPhi(p.get4Vector()));
      EXPECT_FLOAT_EQ(0.0, momentum.DeltaR(p.get4Vector()));
      EXPECT_FLOAT_EQ(momentum.Energy(), p.get4Vector().Energy());
      EXPECT_EQ(Particle::c_Unflavored, p.getFlavorType());
      EXPECT_EQ(Particle::c_MCParticle, p.getParticleSource());
      EXPECT_EQ(123u, p.getMdstArrayIndex());
    }
    {
      // test constructor from ECLClusters
      // (for now we can only create a photon this way)
      StoreArray<ECLCluster> clusters;
      ECLCluster* cluster = clusters.appendNew(ECLCluster());
      cluster->setIsTrack(false);
      cluster->addHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
      cluster->setEnergy(1337);

      Particle p(cluster);
      EXPECT_EQ(22, p.getPDGCode());
      EXPECT_EQ(Particle::c_Unflavored, p.getFlavorType());
      EXPECT_EQ(Particle::c_ECLCluster, p.getParticleSource());
      EXPECT_FLOAT_EQ(1337, p.getEnergy());
      EXPECT_EQ(cluster, p.getECLCluster());
      EXPECT_EQ(nullptr, p.getTrack());
    }
    {
      // test constructor used for V0s (there is not actually a V0 constructor,
      // the heavy-lifting is done in the particle loader), but this is V0-style
      // construction with EParticleSourceObject::V0 and the correct getters
      StoreArray<V0> v0s;
      V0* v0 = v0s.appendNew(V0());
      TLorentzVector momentum(1, 2, 3, 4);
      Particle p(momentum, 310, Particle::c_Unflavored, Particle::c_V0, 0);
      EXPECT_EQ(310, p.getPDGCode());
      EXPECT_EQ(Particle::c_Unflavored, p.getFlavorType());
      EXPECT_EQ(Particle::c_V0, p.getParticleSource());
      EXPECT_EQ(0u, p.getMdstArrayIndex());
      EXPECT_EQ(p.getV0(), v0); // pointers should be same
    }
  }

  TEST_F(ParticleTest, Daughters)
  {
    TLorentzVector momentum;
    const int nDaughters = 6;
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    for (int i = 0; i < nDaughters; i++) {
      Particle d(TLorentzVector(1, 0, 0, 3.0), (i % 2) ? 211 : -211);
      momentum += d.get4Vector();
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices.push_back(newDaughters->getArrayIndex());
    }

    const Particle& p = *(particles.appendNew(momentum, 411, Particle::c_Unflavored, daughterIndices));
    EXPECT_EQ(411, p.getPDGCode());
    EXPECT_FLOAT_EQ(0.0, momentum.DeltaPhi(p.get4Vector()));
    EXPECT_FLOAT_EQ(0.0, momentum.DeltaR(p.get4Vector()));
    EXPECT_FLOAT_EQ(momentum.Energy(), p.get4Vector().Energy());
    EXPECT_EQ(Particle::c_Unflavored, p.getFlavorType());
    EXPECT_EQ(Particle::c_Composite, p.getParticleSource());
    EXPECT_EQ(0u, p.getMdstArrayIndex());
    EXPECT_EQ((unsigned int)nDaughters, p.getNDaughters());
    EXPECT_EQ((unsigned int)nDaughters, p.getDaughters().size());
    EXPECT_EQ((unsigned int)nDaughters, p.getFinalStateDaughters().size());
    EXPECT_EQ((unsigned int)nDaughters, p.getDaughterProperties().size());

    const Particle pLocal(momentum, 411, Particle::c_Unflavored, daughterIndices, particles.getPtr());
    EXPECT_DOUBLE_EQ(p.getMass(), pLocal.getMass());
    EXPECT_EQ((unsigned int)nDaughters, pLocal.getNDaughters());
    EXPECT_EQ((unsigned int)nDaughters, pLocal.getDaughters().size());
    EXPECT_EQ((unsigned int)nDaughters, pLocal.getFinalStateDaughters().size());
    EXPECT_EQ((unsigned int)nDaughters, pLocal.getDaughterProperties().size());

    Particle outsideArray;
    EXPECT_TRUE(outsideArray.getArrayPointer() == nullptr);
    EXPECT_B2FATAL(Particle p2 = Particle(momentum, 411, Particle::c_Unflavored, daughterIndices));
  }

  TEST_F(ParticleTest, DaughterProperties)
  {
    TLorentzVector momentum;
    const int nDaughters = 6;
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    std::vector<int> daughterProperties;
    for (int i = 0; i < nDaughters; i++) {
      Particle d(TLorentzVector(1, 0, 0, 3.0), (i % 2) ? 211 : -211);
      momentum += d.get4Vector();
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices.push_back(newDaughters->getArrayIndex());
      daughterProperties.push_back(Particle::PropertyFlags::c_Ordinary);
    }

    const Particle& p = *(particles.appendNew(momentum, 411, Particle::c_Unflavored, daughterIndices));
    EXPECT_EQ(411, p.getPDGCode());
    EXPECT_FLOAT_EQ(0.0, momentum.DeltaPhi(p.get4Vector()));
    EXPECT_FLOAT_EQ(0.0, momentum.DeltaR(p.get4Vector()));
    EXPECT_FLOAT_EQ(momentum.Energy(), p.get4Vector().Energy());
    EXPECT_EQ(Particle::c_Unflavored, p.getFlavorType());
    EXPECT_EQ(Particle::c_Composite, p.getParticleSource());
    EXPECT_EQ(0u, p.getMdstArrayIndex());
    EXPECT_EQ((unsigned int)nDaughters, p.getNDaughters());
    EXPECT_EQ((unsigned int)nDaughters, p.getDaughters().size());
    EXPECT_EQ((unsigned int)nDaughters, p.getFinalStateDaughters().size());
    EXPECT_EQ((unsigned int)nDaughters, p.getDaughterProperties().size());
    EXPECT_EQ(Particle::PropertyFlags::c_Ordinary, (p.getDaughterProperties())[0]);


    const Particle pLocal(momentum, 411, Particle::c_Unflavored, daughterIndices,
                          Particle::PropertyFlags::c_Ordinary, daughterProperties,
                          particles.getPtr());
    EXPECT_DOUBLE_EQ(p.getMass(), pLocal.getMass());
    EXPECT_EQ((unsigned int)nDaughters, pLocal.getNDaughters());
    EXPECT_EQ((unsigned int)nDaughters, pLocal.getDaughters().size());
    EXPECT_EQ((unsigned int)nDaughters, pLocal.getFinalStateDaughters().size());
    EXPECT_EQ((unsigned int)nDaughters, pLocal.getDaughterProperties().size());

    Particle outsideArray;
    EXPECT_TRUE(outsideArray.getArrayPointer() == nullptr);
    EXPECT_B2FATAL(Particle p2 = Particle(momentum, 411, Particle::c_Unflavored, daughterIndices));
  }

  /** Functor to count children of a particle */
  struct ParticleChildrenCounter {
    int count{0}; /**< number of calls to this object */
    /** increase counter ... duh */
    bool operator()(const Particle*) { ++count; return false; }
  };

  TEST_F(ParticleTest, ForEachDaughters)
  {
    // setup a particle with some daughters and grand daughters
    TLorentzVector momentum;
    const int nDaughters = 6;
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    int nGrandDaughters = 0;
    for (int i = 0; i < nDaughters; i++) {
      Particle d(TLorentzVector(1, 0, 0, 3.0), (i % 2) ? 211 : -211);
      momentum += d.get4Vector();
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices.push_back(newDaughters->getArrayIndex());
      if (i % 2 == 0) {
        Particle* grandDaughter = particles.appendNew(d);
        newDaughters->appendDaughter(grandDaughter);
        ++nGrandDaughters;
      }
    }
    const Particle& p = *(particles.appendNew(momentum, 411, Particle::c_Unflavored, daughterIndices));

    // Check if we get called the correct amount of times
    int count{0};
    auto counterFct = [&count](const Particle*) { ++count; return false; };
    EXPECT_FALSE(p.forEachDaughter(counterFct, false, false));
    EXPECT_EQ(count, nDaughters);
    count = 0;
    EXPECT_FALSE(p.forEachDaughter(counterFct, true, false));
    EXPECT_EQ(count, nDaughters + nGrandDaughters);
    count = 0;
    EXPECT_FALSE(p.forEachDaughter(counterFct, false, true));
    EXPECT_EQ(count, nDaughters + 1);
    count = 0;
    EXPECT_FALSE(p.forEachDaughter(counterFct, true, true));
    EXPECT_EQ(count, nDaughters + nGrandDaughters + 1);

    // Functors passed by value don't return the state
    ParticleChildrenCounter counterStruct;
    EXPECT_FALSE(p.forEachDaughter(counterStruct));
    EXPECT_EQ(counterStruct.count, 0);
    // But if reference_wrapped it should work fine
    EXPECT_FALSE(p.forEachDaughter(std::reference_wrapper<ParticleChildrenCounter>(counterStruct)));
    EXPECT_EQ(counterStruct.count, nDaughters + nGrandDaughters + 1);

    // Test return value: if we return true the processing should be stopped so
    // count should not be increased anymore
    int maxchildren{1}, total{nDaughters + nGrandDaughters + 1};
    auto returnFctTester = [&count, &maxchildren](const Particle*) {++count; return count >= maxchildren; };
    for (; maxchildren < 2 * total; ++maxchildren) {
      count = 0;
      EXPECT_EQ(p.forEachDaughter(returnFctTester), maxchildren <= total);
      EXPECT_EQ(count, std::min(maxchildren, total));
    }
  }

  /** test some basics for extra information. */
  TEST_F(ParticleTest, ExtraInfo)
  {
    Particle p;
    //doesn't exist
    EXPECT_THROW(p.getExtraInfo("htns"), std::runtime_error);

    p.addExtraInfo("htns", 32.0);

    //cannot add it again
    EXPECT_THROW(p.addExtraInfo("htns", 1234.0), std::runtime_error);

    EXPECT_DOUBLE_EQ(32.0, p.getExtraInfo("htns"));

  }

  /** test identification of copies */
  TEST_F(ParticleTest, Copies)
  {
    StoreArray<Particle> particles;
    StoreArray<MCParticle> mcparticles;
    StoreArray<ECLCluster> eclClusters;
    ECLCluster* eclGamma1 = eclClusters. appendNew(ECLCluster());
    eclGamma1->setConnectedRegionId(1);
    eclGamma1->setClusterId(1);
    eclGamma1->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
    ECLCluster* eclGamma2 = eclClusters. appendNew(ECLCluster());
    eclGamma2->setConnectedRegionId(1);
    eclGamma2->setClusterId(2);
    eclGamma2->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
    ECLCluster* eclGamma3 = eclClusters. appendNew(ECLCluster());
    eclGamma3->setConnectedRegionId(2);
    eclGamma3->setClusterId(1);
    eclGamma3->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
    ECLCluster* eclKL = eclClusters. appendNew(ECLCluster());
    eclKL->setConnectedRegionId(2);
    eclKL->setClusterId(1);
    eclKL->setHypothesis(ECLCluster::EHypothesisBit::c_neutralHadron);



    // create some particles
    Particle* T1Pion     = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0),  211, Particle::c_Flavored, Particle::c_Track, 1));
    Particle* T2Pion     = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), -211, Particle::c_Flavored, Particle::c_Track, 2));

    // T1PionCopy is a copy of T1Pion (both are created from the same Track and are pions)
    Particle* T1PionCopy = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), 211, Particle::c_Flavored, Particle::c_Track, 1));

    // T1Kaon is not a coy of T1Pion (both are created from the same Track, but are of different hypothesis)
    Particle* T1Kaon     = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0),  321, Particle::c_Flavored, Particle::c_Track, 1));
    Particle* T2Kaon     = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), -321, Particle::c_Flavored, Particle::c_Track, 2));
    Particle* T3Kaon     = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0),  321, Particle::c_Flavored, Particle::c_Track, 3));
    Particle* T4Kaon     = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), -321, Particle::c_Flavored, Particle::c_Track, 4));

    // T1Gamma
    Particle* T1Gamma    = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), 22, Particle::c_Unflavored, Particle::c_ECLCluster,
                                                        0));
    // T2Gamma
    Particle* T2Gamma    = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), 22, Particle::c_Unflavored, Particle::c_ECLCluster,
                                                        1));

    // T3Gamma
    Particle* T3Gamma    = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), 22, Particle::c_Unflavored, Particle::c_ECLCluster,
                                                        2));
    // T4KL
    Particle* T4KL       = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), Const::Klong.getPDGCode(), Particle::c_Unflavored,
                                                        Particle::c_ECLCluster,
                                                        3));
    MCParticle* MC1Pion = mcparticles. appendNew(MCParticle());
    MC1Pion->setPDG(Const::pion.getPDGCode());
    MC1Pion->set4Vector(TLorentzVector(0, 0, 0, 0));
    Particle* T1PionFromMC     = particles.appendNew(Particle(MC1Pion));

    EXPECT_TRUE(T3Gamma->overlapsWith(T4KL));
    EXPECT_FALSE(T3Gamma->overlapsWith(T2Gamma));
    EXPECT_FALSE(T3Gamma->overlapsWith(T1Gamma));
    EXPECT_FALSE(T1Gamma->overlapsWith(T2Gamma));

    EXPECT_TRUE(T1Pion->isCopyOf(T1PionCopy));
    EXPECT_FALSE(T1Pion->isCopyOf(T1Kaon));
    EXPECT_FALSE(T1Pion->isCopyOf(T1Gamma));
    EXPECT_FALSE(T2Gamma->isCopyOf(T1Gamma));

    //detailed comparison
    EXPECT_TRUE(T1Pion->isCopyOf(T1PionCopy,      true));
    EXPECT_TRUE(T1Pion->isCopyOf(T1Kaon,          true));
    EXPECT_FALSE(T2Gamma->isCopyOf(T1Gamma,       true));
    EXPECT_TRUE(T3Gamma->isCopyOf(T4KL,           true));
    EXPECT_FALSE(T1PionFromMC->isCopyOf(T1Pion, true));
    EXPECT_FALSE(T1Pion->isCopyOf(T1PionFromMC, true));
    // Construct composite particles
    Particle* D0Pi1Pi2 = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), 421));
    D0Pi1Pi2->appendDaughter(T1Pion);
    D0Pi1Pi2->appendDaughter(T2Pion);

    Particle* D0Pi1Pi2Copy = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), 421));
    D0Pi1Pi2Copy->appendDaughter(T1Pion);
    D0Pi1Pi2Copy->appendDaughter(T2Pion);

    Particle* D0Pi1Pi2Copy2 = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), 421));
    D0Pi1Pi2Copy2->appendDaughter(T1PionCopy);
    D0Pi1Pi2Copy2->appendDaughter(T2Pion);

    Particle* D0K1K2 = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), 421));
    D0K1K2->appendDaughter(T1Kaon);
    D0K1K2->appendDaughter(T2Kaon);

    Particle* D0K1Pi2 = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), 421));
    D0K1Pi2->appendDaughter(T1Kaon);
    D0K1Pi2->appendDaughter(T2Pion);

    EXPECT_FALSE(D0Pi1Pi2->isCopyOf(D0K1K2));
    EXPECT_FALSE(D0Pi1Pi2->isCopyOf(D0K1Pi2));
    EXPECT_TRUE(D0Pi1Pi2->isCopyOf(D0Pi1Pi2Copy));
    EXPECT_TRUE(D0Pi1Pi2->isCopyOf(D0Pi1Pi2Copy2));
    EXPECT_TRUE(D0Pi1Pi2Copy->isCopyOf(D0Pi1Pi2Copy2));

    // even more composite particles
    Particle* D0K3K4 = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), 421));
    D0K3K4->appendDaughter(T3Kaon);
    D0K3K4->appendDaughter(T4Kaon);

    Particle* B0_1 = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), 511));
    B0_1->appendDaughter(D0Pi1Pi2);
    B0_1->appendDaughter(D0K3K4);

    Particle* B0_2 = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), 511));
    B0_2->appendDaughter(D0Pi1Pi2Copy);
    B0_2->appendDaughter(D0K3K4);

    Particle* B0_3 = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), 511));
    B0_3->appendDaughter(D0Pi1Pi2Copy2);
    B0_3->appendDaughter(D0K3K4);

    EXPECT_TRUE(B0_1->isCopyOf(B0_2));
    EXPECT_TRUE(B0_1->isCopyOf(B0_3));
    EXPECT_TRUE(B0_2->isCopyOf(B0_3));

    Particle* B0_4 = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), 511));
    B0_4->appendDaughter(D0Pi1Pi2);
    B0_4->appendDaughter(D0K1K2);

    Particle* B0_5 = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), 511));
    B0_5->appendDaughter(D0Pi1Pi2);
    B0_5->appendDaughter(T1Kaon);
    B0_5->appendDaughter(T2Kaon);

    EXPECT_FALSE(B0_1->isCopyOf(B0_4));
    EXPECT_FALSE(B0_4->isCopyOf(B0_5));
  }

  /** test string -> index mapping. */
  TEST_F(ParticleTest, ExtraInfoMap)
  {
    StoreObjPtr<ParticleExtraInfoMap> map;

    Particle p;
    p.print();
    p.addExtraInfo("htns", 1.0);
    EXPECT_EQ(1u, map->getNMaps());

    //adding another variable should just extend the current map
    p.addExtraInfo("somethingelse", 2.0);
    EXPECT_EQ(1u, map->getNMaps());

    //storing same things in another particle shouldn't create a new map
    Particle q;
    q.addExtraInfo("htns", 11.0);
    EXPECT_EQ(1u, map->getNMaps());
    q.addExtraInfo("somethingelse", 12.0);
    EXPECT_EQ(1u, map->getNMaps());

    //lets store something different in p and q
    p.addExtraInfo("thirdvar_p", 3.0);
    EXPECT_EQ(1u, map->getNMaps());
    //different var in 3rd place, needs new map
    q.addExtraInfo("thirdvar_q", 13.0);
    EXPECT_EQ(2u, map->getNMaps());


    //set same vars as with p and q again, one of them should switch maps
    Particle s;
    s.addExtraInfo("htns", 1.0);
    s.addExtraInfo("somethingelse", 2.0);
    s.addExtraInfo("thirdvar_p", 3.0);
    Particle t;
    t.addExtraInfo("htns", 1.0);
    t.addExtraInfo("somethingelse", 2.0);
    t.addExtraInfo("thirdvar_q", 3.0);
    //if switching works, there should still be the same number of maps
    EXPECT_EQ(2u, map->getNMaps());
    //verify switching worked
    EXPECT_THROW(s.getExtraInfo("thirdvar_q"), std::runtime_error);
    EXPECT_THROW(t.getExtraInfo("thirdvar_p"), std::runtime_error);

    //reusing maps shouldn't cause it to find unset vars (e.g. when one value is set, but map has more entries)
    Particle u;
    EXPECT_THROW(u.getExtraInfo("htns"), std::runtime_error);
    u.addExtraInfo("htns", 1.0);
    EXPECT_THROW(u.getExtraInfo("somethingelse"), std::runtime_error);
    EXPECT_THROW(u.getExtraInfo("thirdvar_p"), std::runtime_error);
    EXPECT_THROW(u.getExtraInfo("thirdvar_q"), std::runtime_error);
    u.addExtraInfo("somethingelse", 2.0);
    EXPECT_THROW(u.getExtraInfo("thirdvar_p"), std::runtime_error);
    EXPECT_THROW(u.getExtraInfo("thirdvar_q"), std::runtime_error);


    //add something else first, followed by variables we already used
    Particle v;
    v.addExtraInfo("first", 0.0);
    v.addExtraInfo("htns", 1.0);
    v.addExtraInfo("somethingelse", 2.0);
    v.addExtraInfo("thirdvar_q", 3.0);


    //verify the values we set
    EXPECT_DOUBLE_EQ(1.0, p.getExtraInfo("htns"));
    EXPECT_DOUBLE_EQ(2.0, p.getExtraInfo("somethingelse"));
    EXPECT_DOUBLE_EQ(3.0, p.getExtraInfo("thirdvar_p"));

    EXPECT_DOUBLE_EQ(11.0, q.getExtraInfo("htns"));
    EXPECT_DOUBLE_EQ(12.0, q.getExtraInfo("somethingelse"));
    EXPECT_DOUBLE_EQ(13.0, q.getExtraInfo("thirdvar_q"));

    EXPECT_DOUBLE_EQ(1.0, s.getExtraInfo("htns"));
    EXPECT_DOUBLE_EQ(2.0, s.getExtraInfo("somethingelse"));
    EXPECT_DOUBLE_EQ(3.0, s.getExtraInfo("thirdvar_p"));

    EXPECT_DOUBLE_EQ(1.0, t.getExtraInfo("htns"));
    EXPECT_DOUBLE_EQ(2.0, t.getExtraInfo("somethingelse"));
    EXPECT_DOUBLE_EQ(3.0, t.getExtraInfo("thirdvar_q"));

    EXPECT_DOUBLE_EQ(0.0, v.getExtraInfo("first"));
    EXPECT_DOUBLE_EQ(1.0, v.getExtraInfo("htns"));
    EXPECT_DOUBLE_EQ(2.0, v.getExtraInfo("somethingelse"));
    EXPECT_DOUBLE_EQ(3.0, v.getExtraInfo("thirdvar_q"));

    //check data is copied with Particle
    Particle tCopy = t;
    EXPECT_DOUBLE_EQ(1.0, tCopy.getExtraInfo("htns"));
    EXPECT_DOUBLE_EQ(2.0, tCopy.getExtraInfo("somethingelse"));
    EXPECT_DOUBLE_EQ(3.0, tCopy.getExtraInfo("thirdvar_q"));
    tCopy.print();
  }


  /** test ParticleCopy utility */
  TEST_F(ParticleTest, ParticleCopyUtility)
  {
    StoreArray<Particle> particles;
    StoreArray<MCParticle> mcparticles;
    StoreArray<RestOfEvent> roes;

    // create some particles
    Particle* T1Pion     = particles.appendNew(Particle(TLorentzVector(1, 1, 1, 1),  211, Particle::c_Flavored, Particle::c_Track, 1));
    MCParticle* MC1      = mcparticles. appendNew(MCParticle());
    MC1->setPDG(1);
    T1Pion->addExtraInfo("test_var", 1.0);
    T1Pion->addRelationTo(MC1);
    Particle* T2Kaon     = particles.appendNew(Particle(TLorentzVector(2, 2, 2, 2), -321, Particle::c_Flavored, Particle::c_Track, 2));
    MCParticle* MC2      = mcparticles. appendNew(MCParticle());
    MC1->setPDG(2);
    T2Kaon->addExtraInfo("test_var", 2.0);
    T2Kaon->addRelationTo(MC2);
    Particle* T3Kaon     = particles.appendNew(Particle(TLorentzVector(3, 3, 3, 3),  321, Particle::c_Flavored, Particle::c_Track, 3));
    MCParticle* MC3      = mcparticles. appendNew(MCParticle());
    MC3->setPDG(3);
    T3Kaon->addExtraInfo("test_var", 3.0);
    T3Kaon->addRelationTo(MC3);
    Particle* ROEPion    = particles.appendNew(Particle(TLorentzVector(3.5, 3.5, 3.5, 3.5),  211, Particle::c_Flavored,
                                                        Particle::c_Track, 4));

    // Construct composite particles
    Particle* D0KK = particles.appendNew(Particle(TLorentzVector(4, 4, 4, 4), 421));
    D0KK->appendDaughter(T2Kaon);
    D0KK->appendDaughter(T3Kaon);

    Particle* B0 = particles.appendNew(Particle(TLorentzVector(5, 5, 5, 5), 511));
    B0->appendDaughter(D0KK);
    B0->appendDaughter(T1Pion);

    RestOfEvent* roe = roes.appendNew(RestOfEvent());

    roe->addParticles({ROEPion});
    B0->addRelationTo(roe);

    // Perform tests
    // First sanity check
    // at this point the size of Particle/MCParticle/ROE StoreArray should be 6/3/1
    EXPECT_EQ(particles.getEntries(), 6);
    EXPECT_EQ(mcparticles.getEntries(), 3);
    EXPECT_EQ(roes.getEntries(), 1);

    // now make a copy of B0
    Particle* B0_copy = copyParticle(B0);
    // at this point the size of Particle/MCParticle/ROE StoreArray should be 11/3/1
    EXPECT_EQ(particles.getEntries(), 11);
    EXPECT_EQ(mcparticles.getEntries(), 3);
    EXPECT_EQ(roes.getEntries(), 1);

    // compare copy with original
    EXPECT_EQ(B0->getPDGCode(), B0_copy->getPDGCode());
    EXPECT_EQ(B0->getNDaughters(), B0_copy->getNDaughters());
    EXPECT_EQ(B0->getDaughter(0)->getPDGCode(), B0_copy->getDaughter(0)->getPDGCode());
    EXPECT_EQ(B0->getDaughter(1)->getPDGCode(), B0_copy->getDaughter(1)->getPDGCode());
    EXPECT_EQ(B0->getDaughter(0)->getDaughter(0)->getPDGCode(), B0_copy->getDaughter(0)->getDaughter(0)->getPDGCode());
    EXPECT_EQ(B0->getDaughter(0)->getDaughter(1)->getPDGCode(), B0_copy->getDaughter(0)->getDaughter(1)->getPDGCode());

    EXPECT_FALSE(B0->getArrayIndex() == B0_copy->getArrayIndex());
    EXPECT_FALSE(B0->getDaughter(0)->getArrayIndex() == B0_copy->getDaughter(0)->getArrayIndex());
    EXPECT_FALSE(B0->getDaughter(1)->getArrayIndex() == B0_copy->getDaughter(1)->getArrayIndex());
    EXPECT_FALSE(B0->getDaughter(0)->getDaughter(0)->getArrayIndex() == B0_copy->getDaughter(0)->getDaughter(0)->getArrayIndex());
    EXPECT_FALSE(B0->getDaughter(0)->getDaughter(1)->getArrayIndex() == B0_copy->getDaughter(0)->getDaughter(1)->getArrayIndex());

    EXPECT_TRUE(B0->get4Vector() == B0_copy->get4Vector());
    EXPECT_TRUE(B0->getDaughter(0)->get4Vector() == B0_copy->getDaughter(0)->get4Vector());
    EXPECT_TRUE(B0->getDaughter(1)->get4Vector() == B0_copy->getDaughter(1)->get4Vector());
    EXPECT_TRUE(B0->getDaughter(0)->getDaughter(0)->get4Vector() == B0_copy->getDaughter(0)->getDaughter(0)->get4Vector());
    EXPECT_TRUE(B0->getDaughter(0)->getDaughter(1)->get4Vector() == B0_copy->getDaughter(0)->getDaughter(1)->get4Vector());

    EXPECT_TRUE(B0->getDaughter(1)->getExtraInfo("test_var") == B0_copy->getDaughter(1)->getExtraInfo("test_var"));
    EXPECT_TRUE(B0->getDaughter(0)->getDaughter(0)->getExtraInfo("test_var") == B0_copy->getDaughter(0)->getDaughter(
                  0)->getExtraInfo("test_var"));
    EXPECT_TRUE(B0->getDaughter(0)->getDaughter(1)->getExtraInfo("test_var") == B0_copy->getDaughter(0)->getDaughter(
                  1)->getExtraInfo("test_var"));

    // check relations
    const MCParticle* mc1orig = B0->getDaughter(1)->getRelated<MCParticle>();
    const MCParticle* mc2orig = B0->getDaughter(0)->getDaughter(0)->getRelated<MCParticle>();
    const MCParticle* mc3orig = B0->getDaughter(0)->getDaughter(1)->getRelated<MCParticle>();

    const MCParticle* mc1copy = B0_copy->getDaughter(1)->getRelated<MCParticle>();
    const MCParticle* mc2copy = B0_copy->getDaughter(0)->getDaughter(0)->getRelated<MCParticle>();
    const MCParticle* mc3copy = B0_copy->getDaughter(0)->getDaughter(1)->getRelated<MCParticle>();

    const RestOfEvent* roeorig = B0->getRelated<RestOfEvent>();
    const RestOfEvent* roecopy = B0_copy->getRelated<RestOfEvent>();

    EXPECT_TRUE(mc1orig == mc1copy);
    EXPECT_TRUE(mc2orig == mc2copy);
    EXPECT_TRUE(mc3orig == mc3copy);

    EXPECT_TRUE(mc1orig->getPDG() == mc1copy->getPDG());
    EXPECT_TRUE(mc2orig->getPDG() == mc2copy->getPDG());
    EXPECT_TRUE(mc3orig->getPDG() == mc3copy->getPDG());

    EXPECT_TRUE(roeorig->hasParticle(ROEPion) && roecopy->hasParticle(ROEPion));

    // modify original and check the copy
    MCParticle* MC4      = mcparticles. appendNew(MCParticle());
    MC4->setPDG(4);
    B0->getDaughter(0)->addRelationTo(MC4);

    const MCParticle* mc4orig = B0->getDaughter(0)->getRelated<MCParticle>();
    const MCParticle* mc4copy = B0_copy->getDaughter(0)->getRelated<MCParticle>();

    EXPECT_FALSE(mc4orig == nullptr);
    EXPECT_TRUE(mc4copy == nullptr);

    const_cast<Particle*>(B0->getDaughter(1))->addExtraInfo("origOnly_var", 10.0);

    EXPECT_DOUBLE_EQ(10.0, B0->getDaughter(1)->getExtraInfo("origOnly_var"));
    EXPECT_THROW(B0_copy->getDaughter(1)->getExtraInfo("origOnly_var"), std::runtime_error);

    // modify copy and check the original
    MCParticle* MC5      = mcparticles. appendNew(MCParticle());
    MC5->setPDG(5);
    B0_copy->addRelationTo(MC5);

    const MCParticle* mc5orig = B0->getRelated<MCParticle>();
    const MCParticle* mc5copy = B0_copy->getRelated<MCParticle>();

    EXPECT_TRUE(mc5orig == nullptr);
    EXPECT_FALSE(mc5copy == nullptr);

    const_cast<Particle*>(B0_copy->getDaughter(1))->addExtraInfo("copyOnly_var", 15.0);

    EXPECT_THROW(B0->getDaughter(1)->getExtraInfo("copyOnly_var"), std::runtime_error);
    EXPECT_DOUBLE_EQ(15.0, B0_copy->getDaughter(1)->getExtraInfo("copyOnly_var"));

  }

  /** test cluster based functionality: hypotheses and such */
  TEST_F(ParticleTest, ECLClusterBased)
  {
    StoreArray<ECLCluster> eclclusters;
    {
      ECLCluster* cluster = eclclusters.appendNew(ECLCluster());
      cluster->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
      cluster->setEnergy(1.);
      cluster->setEnergyRaw(2.);

      Particle p(cluster);
      EXPECT_FLOAT_EQ(1., p.getECLClusterEnergy());
      EXPECT_FLOAT_EQ(1., p.getEnergy());
      EXPECT_EQ(ECLCluster::EHypothesisBit::c_nPhotons, p.getECLClusterEHypothesisBit());
      EXPECT_FLOAT_EQ(0, p.getMass());
    }

    {
      ECLCluster* cluster = eclclusters.appendNew(ECLCluster());
      cluster->setHypothesis(ECLCluster::EHypothesisBit::c_neutralHadron);
      cluster->setEnergy(1.);
      cluster->setEnergyRaw(2.);

      Particle p(cluster, Const::Klong);
      EXPECT_EQ(130, p.getPDGCode());
      EXPECT_FLOAT_EQ(2., p.getECLClusterEnergy());
      EXPECT_FLOAT_EQ(std::sqrt(4. + 0.497614 * 0.497614), p.getEnergy());
      EXPECT_EQ(ECLCluster::EHypothesisBit::c_neutralHadron, p.getECLClusterEHypothesisBit());
      int pdg = Const::Klong.getPDGCode();
      double m = TDatabasePDG::Instance()->GetParticle(pdg)->Mass();
      EXPECT_FLOAT_EQ(m, p.getMass());
    }

    {
      ECLCluster* cluster = eclclusters.appendNew(ECLCluster());
      cluster->setHypothesis(ECLCluster::EHypothesisBit::c_neutralHadron);
      cluster->setEnergy(1.);
      cluster->setEnergyRaw(2.);

      Particle p(cluster, Const::neutron);
      EXPECT_EQ(2112, p.getPDGCode());
      EXPECT_FLOAT_EQ(2., p.getECLClusterEnergy());
      EXPECT_FLOAT_EQ(std::sqrt(4. + 0.93956536 * 0.93956536), p.getEnergy());
      EXPECT_EQ(ECLCluster::EHypothesisBit::c_neutralHadron, p.getECLClusterEHypothesisBit());
      int pdg = Const::neutron.getPDGCode();
      double m = TDatabasePDG::Instance()->GetParticle(pdg)->Mass();
      EXPECT_FLOAT_EQ(m, p.getMass());
    }
  }

  /** test particle creation from KLMCluster */
  TEST_F(ParticleTest, KLMClusterBased)
  {
    StoreArray<KLMCluster> klmClusters;
    {
      KLMCluster* cluster = klmClusters.appendNew(KLMCluster());
      cluster->setTime(1.1);
      cluster->setClusterPosition(1.1, 1.1, 1.0);
      cluster->setLayers(1);
      cluster->setInnermostLayer(1);
      cluster->setMomentumMag(1.0);

      Particle p(cluster);
      int pdg = Const::Klong.getPDGCode();
      EXPECT_EQ(pdg, p.getPDGCode());
      double m = TDatabasePDG::Instance()->GetParticle(pdg)->Mass();
      EXPECT_FLOAT_EQ(sqrt(1. + m * m), p.getEnergy());
      EXPECT_EQ(Particle::c_Unflavored, p.getFlavorType());
      EXPECT_FLOAT_EQ(m, p.getMass());
    }

    {
      KLMCluster* cluster = klmClusters.appendNew(KLMCluster());
      cluster->setTime(1.1);
      cluster->setClusterPosition(1.1, 1.1, 1.0);
      cluster->setLayers(1);
      cluster->setInnermostLayer(1);
      cluster->setMomentumMag(1.0);

      Particle p(cluster, Const::neutron.getPDGCode());
      int pdg = Const::neutron.getPDGCode();
      EXPECT_EQ(pdg, p.getPDGCode());
      double m = TDatabasePDG::Instance()->GetParticle(pdg)->Mass();
      EXPECT_FLOAT_EQ(sqrt(1. + m * m), p.getEnergy());
      EXPECT_EQ(Particle::c_Flavored, p.getFlavorType());
      EXPECT_FLOAT_EQ(m, p.getMass());
    }
  }
}  // namespace
