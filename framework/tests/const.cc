#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <TParticlePDG.h>

#include <gtest/gtest.h>
#include <math.h>

using namespace std;

namespace Belle2 {
  /** command x should exit using B2FATAL. */
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** test fixture. */
  class ConstTest : public ::testing::Test {
  protected:

  };

  /** Check basics. */
  TEST_F(ConstTest, ParticleBasics)
  {
    //construction
    Const::ChargedStable(Const::ParticleType(211));
    Const::ChargedStable(Const::ParticleType(321));
    Const::ChargedStable(Const::ParticleType(2212));
    Const::ChargedStable(Const::ParticleType(11));
    Const::ChargedStable(Const::ParticleType(13));
    EXPECT_THROW(Const::ChargedStable(Const::ParticleType(22)), std::runtime_error);
    EXPECT_THROW(Const::ChargedStable(Const::ParticleType(-211)), std::runtime_error);

    const Const::ParticleSet emptyset;
    EXPECT_FALSE(emptyset.contains(Const::Klong));

    //check indices of some defined particles
    EXPECT_EQ(0, Const::pion.getIndex());
    EXPECT_EQ(1, Const::kaon.getIndex());
    EXPECT_EQ(2, Const::proton.getIndex());
    EXPECT_EQ(3, Const::electron.getIndex());
    EXPECT_EQ(4, Const::muon.getIndex());

    //and after a copy
    Const::ChargedStable c = Const::muon;
    EXPECT_EQ(4, c.getIndex());
    Const::ParticleType p = Const::muon;
    EXPECT_EQ(4, p.getIndex());

    //and after construction from PDG code
    EXPECT_EQ(4, Const::ChargedStable(13).getIndex());

    //not in any set
    EXPECT_EQ(-1, Const::invalidParticle.getIndex());
    EXPECT_EQ(-1, Const::Klong.getIndex());
    EXPECT_EQ(-1, Const::photon.getIndex());
  }

  /** Check iteration over ParticleSets. */
  TEST_F(ConstTest, ParticleIteration)
  {
    const Const::ParticleSet emptyset;
    EXPECT_FALSE(emptyset.contains(Const::Klong));

    const Const::ParticleSet set = Const::chargedStableSet;
    Const::ParticleType prefix = set.begin();
    EXPECT_EQ(1, (++prefix).getIndex());
    Const::ParticleType postfix = set.begin();
    EXPECT_EQ(0, (postfix++).getIndex());

    int size = 0;
    for (Const::ParticleType pdgIter = set.begin(); pdgIter != set.end(); ++pdgIter) {
      Const::ChargedStable c = pdgIter; //creating the restricted type should work

      int pdg = pdgIter.getPDGCode();
      unsigned int index = c.getIndex();

      switch (index) {
        case 0:
          EXPECT_EQ(211, pdg);
          break;
        case 1:
          EXPECT_EQ(321, pdg);
          break;
        case 2:
          EXPECT_EQ(2212, pdg);
          break;
        case 3:
          EXPECT_EQ(11, pdg);
          break;
        case 4:
          EXPECT_EQ(13, pdg);
          break;
        default:
          EXPECT_TRUE(false) << "Index >4 encountered?";
      }
      size++;
    }
    EXPECT_EQ(5, size);

    Const::ChargedStable c = Const::proton;
    EXPECT_TRUE(Const::chargedStableSet.contains(c));
    EXPECT_EQ(2, c.getIndex());
    ++c;
    ++c;
    EXPECT_EQ(4, c.getIndex());
    EXPECT_EQ(13, c.getPDGCode());
  }

  /** Check combination of ParticleSets. */
  TEST_F(ConstTest, ParticleCombination)
  {
    //no-op
    const Const::ParticleSet set = Const::chargedStableSet + Const::chargedStableSet;
    int size = 0;
    for (Const::ParticleType pdgIter = set.begin(); pdgIter != set.end(); ++pdgIter) {
      size++;
    }
    EXPECT_EQ(5, size);

    const Const::ParticleSet kaonSet = Const::kaon + Const::Klong + Const::Kshort;
    size = 0;
    for (Const::ParticleType pdgIter = kaonSet.begin(); pdgIter != kaonSet.end(); ++pdgIter) {
      size++;
    }
    EXPECT_EQ(3, size);

    const Const::ParticleSet mergedSet = set + kaonSet;
    size = 0;
    for (Const::ParticleType pdgIter = mergedSet.begin(); pdgIter != mergedSet.end(); ++pdgIter) {
      size++;
    }
    EXPECT_EQ(size, 7); //kaon should be removed
  }

  /** Check TDatabasePDG lookups. */
  TEST_F(ConstTest, TDatabasePDG)
  {
    EXPECT_DOUBLE_EQ(Const::Klong.getParticlePDG()->Charge(), 0);
    EXPECT_EQ(Const::Klong.getParticlePDG()->PdgCode(), 130);

    EXPECT_DOUBLE_EQ(Const::proton.getParticlePDG()->Charge(), 3);
    EXPECT_EQ(Const::proton.getParticlePDG()->PdgCode(), 2212);

    Const::ParticleType antiproton(-2212);
    EXPECT_DOUBLE_EQ(antiproton.getParticlePDG()->Charge(), -3);
    EXPECT_EQ(antiproton.getParticlePDG()->PdgCode(), -2212);

    EXPECT_TRUE(Const::invalidParticle.getParticlePDG() == NULL);
  }

  /** Check DetectorSet. */
  TEST_F(ConstTest, DetectorSet)
  {
    Const::DetectorSet set(Const::IR);
    EXPECT_EQ(set, Const::IR);
    set += Const::PXD;
    EXPECT_EQ(set, Const::IR + Const::PXD);
    set += Const::PXD;
    EXPECT_EQ(set, Const::IR + Const::PXD);
    EXPECT_TRUE(set == Const::IR + Const::PXD);
    EXPECT_FALSE(set == Const::IR);
    set -= Const::IR;
    EXPECT_EQ(set, Const::PXD);
    EXPECT_TRUE(set.contains(Const::PXD));
    EXPECT_FALSE(set.contains(Const::IR));
    set += Const::SVD + Const::TEST;
    EXPECT_EQ(set.getIndex(Const::IR), -1);
    EXPECT_EQ(set.getIndex(Const::PXD), 0);
    EXPECT_EQ(set.getIndex(Const::TEST), 2);
    EXPECT_EQ(set[0], Const::PXD);
    EXPECT_EQ(set[2], Const::TEST);
    EXPECT_EQ(set[3], Const::invalidDetector);
    EXPECT_EQ(set.size(), (size_t)3);
    EXPECT_EQ(Const::allDetectors.size(), (size_t)12);
  }

}  // namespace
