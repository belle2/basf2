#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/TestHelpers.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;

namespace {
  /** Test fixture. */
  class ParticleTest : public ::testing::Test {
  protected:
    /** register Particle array + ParticleExtraInfoMap object. */
    virtual void SetUp()
    {
      DataStore::Instance().setInitializeActive(true);
      StoreObjPtr<ParticleExtraInfoMap>::registerPersistent();
      StoreArray<Particle>::registerPersistent();
      DataStore::Instance().setInitializeActive(false);
    }

    /** clear datastore */
    virtual void TearDown()
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
      EXPECT_EQ(Particle::c_Undefined, p.getParticleType());
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
      EXPECT_EQ(Particle::c_Undefined, p.getParticleType());
    }
    {
      TLorentzVector momentum(1, 2, 3, 4);
      Particle p(momentum, 22, Particle::c_Unflavored, Particle::c_MCParticle, 123);
      EXPECT_EQ(22, p.getPDGCode());
      EXPECT_FLOAT_EQ(0.0, momentum.DeltaPhi(p.get4Vector()));
      EXPECT_FLOAT_EQ(0.0, momentum.DeltaR(p.get4Vector()));
      EXPECT_FLOAT_EQ(momentum.Energy(), p.get4Vector().Energy());
      EXPECT_EQ(Particle::c_Unflavored, p.getFlavorType());
      EXPECT_EQ(Particle::c_MCParticle, p.getParticleType());
      EXPECT_EQ(123u, p.getMdstArrayIndex());
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
    EXPECT_EQ(Particle::c_Composite, p.getParticleType());
    EXPECT_EQ(0u, p.getMdstArrayIndex());
    EXPECT_EQ((unsigned int)nDaughters, p.getNDaughters());
    EXPECT_EQ((unsigned int)nDaughters, p.getDaughters().size());
    EXPECT_EQ((unsigned int)nDaughters, p.getFinalStateDaughters().size());

    const Particle pLocal(momentum, 411, Particle::c_Unflavored, daughterIndices, particles.getPtr());
    EXPECT_DOUBLE_EQ(p.getMass(), pLocal.getMass());
    EXPECT_EQ((unsigned int)nDaughters, pLocal.getNDaughters());
    EXPECT_EQ((unsigned int)nDaughters, pLocal.getDaughters().size());
    EXPECT_EQ((unsigned int)nDaughters, pLocal.getFinalStateDaughters().size());

    Particle outsideArray;
    EXPECT_TRUE(outsideArray.getArrayPointer() == nullptr);
    EXPECT_B2FATAL(Particle p2 = Particle(momentum, 411, Particle::c_Unflavored, daughterIndices));
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
                                                        1));
    // T2Gamma
    Particle* T2Gamma    = particles.appendNew(Particle(TLorentzVector(0, 0, 0, 0), 22, Particle::c_Unflavored, Particle::c_ECLCluster,
                                                        2));

    EXPECT_TRUE(T1Pion->isCopyOf(T1PionCopy));
    EXPECT_FALSE(T1Pion->isCopyOf(T1Kaon));
    EXPECT_FALSE(T1Pion->isCopyOf(T1Gamma));
    EXPECT_FALSE(T2Gamma->isCopyOf(T1Gamma));

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

}  // namespace
