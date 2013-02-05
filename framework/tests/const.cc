#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <TParticlePDG.h>

#include <gtest/gtest.h>
#include <math.h>
#include <cstddef>

using namespace std;

namespace Belle2 {
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** Set up a few arrays and objects in the datastore */
  class ConstTest : public ::testing::Test {
  protected:

  };

  /** Check iteration over ParticleSets. */
  TEST_F(ConstTest, ParticleIteration)
  {
    const Const::ParticleSet emptyset;
    EXPECT_FALSE(emptyset.contains(Const::Klong));

    const Const::ParticleSet set = Const::chargedStable;
    Const::ParticleType prefix = set.begin();
    EXPECT_EQ(1, (++prefix).getIndex());
    Const::ParticleType postfix = set.begin();
    EXPECT_EQ(0, (postfix++).getIndex());

    int size = 0;
    for (Const::ParticleType pdgIter = set.begin(); pdgIter != set.end(); ++pdgIter) {
      int pdg = pdgIter.getPDGCode();
      unsigned int index = pdgIter.getIndex();

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
          EXPECT_EQ(false, true) << "Index >4 encountered?";
      }
      size++;
    }
    EXPECT_EQ(5, size);

    Const::ChargedStable c = Const::ChargedStable::proton;
    EXPECT_TRUE(Const::chargedStable.contains(c));
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
    const Const::ParticleSet set = Const::chargedStable + Const::chargedStable;
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

    EXPECT_TRUE(Const::invalidParticle.getParticlePDG() == NULL);
  }

}  // namespace
