#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <TParticlePDG.h>

#include <gtest/gtest.h>
#include <math.h>

using namespace std;
using namespace Belle2;

namespace {
  /** Check basics. */
  TEST(ConstTest, ParticleBasics)
  {
    //construction
    Const::ChargedStable(Const::ParticleType(211));
    Const::ChargedStable(Const::ParticleType(321));
    Const::ChargedStable(Const::ParticleType(2212));
    Const::ChargedStable(Const::ParticleType(11));
    Const::ChargedStable(Const::ParticleType(13));
    Const::ChargedStable(Const::ParticleType(1000010020));
    EXPECT_THROW(Const::ChargedStable(Const::ParticleType(22)), std::runtime_error);
    // EXPECT_THROW(Const::ChargedStable(Const::ParticleType(-211)), std::runtime_error);

    const Const::ParticleSet emptyset;
    EXPECT_FALSE(emptyset.contains(Const::Klong));

    //check indices of some defined particles
    EXPECT_EQ(0, Const::electron.getIndex());
    EXPECT_EQ(1, Const::muon.getIndex());
    EXPECT_EQ(2, Const::pion.getIndex());
    EXPECT_EQ(3, Const::kaon.getIndex());
    EXPECT_EQ(4, Const::proton.getIndex());
    EXPECT_EQ(5, Const::deuteron.getIndex());

    //and after a copy
    Const::ChargedStable c = Const::muon;
    EXPECT_EQ(1, c.getIndex());
    Const::ParticleType p = Const::muon;
    EXPECT_EQ(1, p.getIndex());

    //and after construction from PDG code
    EXPECT_EQ(1, Const::ChargedStable(13).getIndex());

    //not in any set
    EXPECT_EQ(-1, Const::invalidParticle.getIndex());
    EXPECT_EQ(-1, Const::Klong.getIndex());
    EXPECT_EQ(-1, Const::photon.getIndex());
  }

  /** Check iteration over ParticleSets. */
  TEST(ConstTest, ParticleIteration)
  {
    const Const::ParticleSet emptyset;
    EXPECT_FALSE(emptyset.contains(Const::Klong));

    const Const::ParticleSet set = Const::chargedStableSet;
    Const::ParticleType prefix = set.begin();
    EXPECT_EQ(1, (++prefix).getIndex());
    Const::ParticleType postfix = set.begin();
    EXPECT_EQ(0, (postfix++).getIndex());

    int size = 0;
    //note: iterating over the restricted type (ParticleType would work, too)
    for (const Const::ChargedStable& c : set) {

      int pdg = c.getPDGCode();
      unsigned int index = c.getIndex();

      switch (index) {
        case 0:
          EXPECT_EQ(11, pdg);
          break;
        case 1:
          EXPECT_EQ(13, pdg);
          break;
        case 2:
          EXPECT_EQ(211, pdg);
          break;
        case 3:
          EXPECT_EQ(321, pdg);
          break;
        case 4:
          EXPECT_EQ(2212, pdg);
          break;
        case 5:
          EXPECT_EQ(1000010020, pdg);
          break;
        case 6:
          EXPECT_EQ(-11, pdg);
          break;
        case 7:
          EXPECT_EQ(-13, pdg);
          break;
        case 8:
          EXPECT_EQ(-211, pdg);
          break;
        case 9:
          EXPECT_EQ(-321, pdg);
          break;
        case 10:
          EXPECT_EQ(-2212, pdg);
          break;
        default:
          EXPECT_TRUE(false) << "Index >10 encountered?";
      }
      size++;
    }
    int setSize = Const::ChargedStable::c_SetSize;
    EXPECT_EQ(setSize, size);

    size = 0;
    for (Const::ParticleType pdgIter = set.begin(); pdgIter != set.end(); ++pdgIter) {
      size++;
    }
    EXPECT_EQ(6, size);

    Const::ChargedStable c = Const::kaon;
    EXPECT_TRUE(Const::chargedStableSet.contains(c));
    EXPECT_EQ(3, c.getIndex());
    ++c;
    ++c;
    EXPECT_EQ(5, c.getIndex());
    EXPECT_EQ(1000010020, c.getPDGCode());
  }

  /** Check combination of ParticleSets. */
  TEST(ConstTest, ParticleCombination)
  {
    //no-op
    const Const::ParticleSet set = Const::chargedStableSet + Const::chargedStableSet;
    int size = 0;
    for (Const::ParticleType pdgIter = set.begin(); pdgIter != set.end(); ++pdgIter) {
      size++;
    }
    EXPECT_EQ(6, size);

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
    EXPECT_EQ(size, 8); //kaon should've not been double counted
  }

  TEST(ConstTest, FindInParticleSet)
  {
    for (const Const::ChargedStable& c : Const::chargedStableSet) {
      int pdg = c.getPDGCode();
      EXPECT_EQ(pdg, Const::chargedStableSet.find(pdg).getPDGCode());
    }
    EXPECT_TRUE(Const::chargedStableSet.find(12356467) == Const::invalidParticle);
  }

  /** Check TDatabasePDG lookups. */
  TEST(ConstTest, TDatabasePDG)
  {
    EXPECT_DOUBLE_EQ(Const::Klong.getParticlePDG()->Charge(), 0);
    EXPECT_EQ(Const::Klong.getParticlePDG()->PdgCode(), 130);

    EXPECT_DOUBLE_EQ(Const::proton.getParticlePDG()->Charge(), 3);
    EXPECT_EQ(Const::proton.getParticlePDG()->PdgCode(), 2212);

    Const::ParticleType antiproton(-2212);
    EXPECT_DOUBLE_EQ(antiproton.getParticlePDG()->Charge(), -3);
    EXPECT_EQ(antiproton.getParticlePDG()->PdgCode(), -2212);

    //Spin and lifetime are added manually to TDatabasePDG, test this
    EXPECT_DOUBLE_EQ(1.0, Const::photon.getParticlePDG()->Spin());
    EXPECT_DOUBLE_EQ(0.5, Const::proton.getParticlePDG()->Spin());
    EXPECT_DOUBLE_EQ(0.5, Const::electron.getParticlePDG()->Spin());

    EXPECT_DOUBLE_EQ(0.0, Const::proton.getParticlePDG()->Lifetime());
    //lifetime (about 881s for neutron, 2.197e-6s for muon)
    EXPECT_TRUE(Const::neutron.getParticlePDG()->Lifetime() > 800);
    EXPECT_TRUE(Const::neutron.getParticlePDG()->Lifetime() < 900);
    EXPECT_TRUE(Const::muon.getParticlePDG()->Lifetime() < 2.2e-6);
    EXPECT_TRUE(Const::muon.getParticlePDG()->Lifetime() > 2.1e-6);

    //AntiParticle is non-const...
    TParticlePDG* protonnonconst = const_cast<TParticlePDG*>(Const::proton.getParticlePDG());
    TParticlePDG* photonnonconst = const_cast<TParticlePDG*>(Const::photon.getParticlePDG());

    //test that AntiParticle() works as expected (not the case for previous implementation)
    EXPECT_DOUBLE_EQ(protonnonconst->AntiParticle()->Charge(), -3);
    EXPECT_EQ(protonnonconst->AntiParticle()->PdgCode(), -2212);

    EXPECT_TRUE(photonnonconst->AntiParticle() == photonnonconst);


    EXPECT_TRUE(Const::invalidParticle.getParticlePDG() == NULL);
    EXPECT_TRUE(Const::unspecifiedParticle.getParticlePDG() == NULL);
  }

  /** Check DetectorSet. */
  TEST(ConstTest, DetectorSet)
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

  /** Check RestrictedDetectorSet. */
  TEST(ConstTest, RestrictedDetectorSet)
  {
    //note: cannot use EXPECT_EQ() here because c_size is only declared in header, but EXPECT_EQ wants a const&, leading to an undefined reference
    EXPECT_TRUE(Const::PIDDetectors::c_set.size() == Const::PIDDetectors::c_size);
  }

}  // namespace
