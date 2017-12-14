#include <analysis/utility/MCMatching.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/MCParticleGraph.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;

//checks against update3
#if defined(__INTEL_COMPILER) && ((__INTEL_COMPILER < 1400) || (__INTEL_COMPILER_BUILD_DATE < 20140422))
namespace {
  TEST(MCMatchingTest, TestsDisabled)
  {
    EXPECT_TRUE(false) <<
                       "MC matching test disabled on intel compiler (version < 14 sp1 update2), please see https://software.intel.com/en-us/forums/topic/475378";
  }
}
#else

namespace {
  MCParticleGraph gParticleGraph;

  /** is this a final state particle? (determines wether a Particle->MCParticle relation will be created during reconstruction) */
  bool isFSP(int pdg)
  {
    switch (abs(pdg)) {
      case 211:
      case 321:
      case 11:
      case 13:
      case 2212:
      case 22:
      case 130:
        return true;
      default:
        return false;
    }
  }

  /** Helper for constructing MCParticle trees and reconstructing them. */
  struct Decay {
  public:
    /** Controls reconstruction behaviour. */
    enum EBehavior {
      c_Default, /**< for FSPs, relate with corresponding particle in created decay. */
      c_CreateNewMCParticle, /**< Create a new MCParticle and relate this particle with it. */
      c_RelateWith, /**< Relate with specified MCParticle (but reconstruct from same particle as usually). */
      c_ReconstructFrom, /**< Create Particle from given Decay (and associated daughters). */
    };
    /** create MCParticles for decay of particle with 'pdg' to given daughter PDG codes. */
    Decay(int pdg, std::vector<Decay> daughters = std::vector<Decay>()):
      m_pdg(pdg), m_daughterDecays(daughters), m_mcparticle(nullptr), m_particle(nullptr)
    {
      m_graphParticle = &gParticleGraph.addParticle();
      m_graphParticle->setPDG(m_pdg);
      m_graphParticle->setStatus(MCParticle::c_PrimaryParticle);
      for (Decay& d : daughters) {
        gParticleGraph.addDecay(*m_graphParticle, *d.m_graphParticle);
      }
    }
    /** add all MCParticles created so far to data store. Must be called manually if you want to check MCParticles before calling reconstruct(). */
    void finalize()
    {
      gParticleGraph.generateList();
      gParticleGraph.clear(); //don't add them again in the next call..

      StoreArray<MCParticle> mcparticles;
      m_mcparticle = mcparticles[m_graphParticle->getIndex() - 1];

      for (Decay& d : m_daughterDecays)
        d.finalize();
    }

    /** get first Particle with matching PDG code. */
    Particle* getParticle(int pdg) const
    {
      if (m_pdg == pdg and m_particle)
        return m_particle;

      for (auto& d : m_daughterDecays) {
        Particle* res = d.getParticle(pdg);
        if (res)
          return res;
      }
      return nullptr;
    }
    /** get first MCParticle with matching PDG code. */
    MCParticle* getMCParticle(int pdg) const
    {
      if (m_pdg == pdg and m_mcparticle)
        return m_mcparticle;

      for (auto& d : m_daughterDecays) {
        MCParticle* res = d.getMCParticle(pdg);
        if (res)
          return res;
      }
      return nullptr;
    }
    /** get first Decay with matching PDG code. */
    Decay* getDecay(int pdg)
    {
      if (m_pdg == pdg)
        return this;

      for (auto& d : m_daughterDecays) {
        Decay* res = d.getDecay(pdg);
        if (res)
          return res;
      }
      return nullptr;
    }

    Decay& operator[](int i) { return m_daughterDecays[i]; }

    /** Helper for constructing Particles. */
    struct ReconstructedDecay {
      ReconstructedDecay(int pdg, std::vector<ReconstructedDecay> daughters = std::vector<ReconstructedDecay>(),
                         EBehavior behavior = c_Default):
        m_pdg(pdg), m_daughterDecays(daughters), m_behavior(behavior), m_optMcPart(nullptr), m_optDecay(nullptr) { }
      ReconstructedDecay(int pdg, std::vector<ReconstructedDecay> daughters, EBehavior behavior, MCParticle* optMcPart):
        m_pdg(pdg), m_daughterDecays(daughters), m_behavior(behavior), m_optMcPart(optMcPart), m_optDecay(nullptr) { }
      ReconstructedDecay(int pdg, std::vector<ReconstructedDecay> daughters, EBehavior behavior, Decay* optDecay):
        m_pdg(pdg), m_daughterDecays(daughters), m_behavior(behavior), m_optMcPart(nullptr), m_optDecay(optDecay) { }
      int m_pdg; /**< PDG code of this particle. */
      vector<ReconstructedDecay> m_daughterDecays; /**< decay products. */
      EBehavior m_behavior; /**< special treatment for this reconstruction step? */
      MCParticle* m_optMcPart; /**< meaning depends on m_behavior. */
      Decay* m_optDecay; /**< meaning depends on m_behavior. */
    };

    /** create Particles corresponding to the MC particles specified in the constructor. For final state particles, relations to MCParticles are created.
     *
     * Particle creation can be skipped by specifying PDG=0 in a specific place.
     *
     * TODO: adding additional Particles (i.e. more than MCParticles) doesn't work yet
     */
    void reconstruct(ReconstructedDecay decay)
    {
      if (!m_mcparticle) {
        finalize();
      }
      if (decay.m_behavior == c_CreateNewMCParticle) {
        m_graphParticle = &gParticleGraph.addParticle();
        m_graphParticle->setPDG(decay.m_pdg);
        finalize(); //overwrites m_mcparticle with the new particle
      } else if (decay.m_behavior == c_RelateWith) {
        m_mcparticle = decay.m_optMcPart;
      } else if (decay.m_behavior == c_ReconstructFrom) {
        ASSERT_TRUE(decay.m_optDecay != nullptr);
        Decay* mcDecay = decay.m_optDecay;
        decay.m_optDecay = nullptr;
        decay.m_behavior = Decay::c_Default;
        mcDecay->reconstruct(decay);
        return;
      }

      StoreArray<Particle> particles;
      if (isFSP(decay.m_pdg)) {
        //is a final state particle, link with MCParticle
        m_particle = particles.appendNew(m_graphParticle->get4Vector(), decay.m_pdg);
        m_particle->addRelationTo(m_mcparticle);
      }

      if (!decay.m_daughterDecays.empty()) {
        Decay* mcDecay = this;
        if (decay.m_behavior == c_ReconstructFrom) {
          //use given decay to reconstruct daughters instead
          mcDecay = decay.m_optDecay;
        }
        std::vector<int> daughterIndices;
        for (unsigned int i = 0; i < decay.m_daughterDecays.size(); i++) {
          Decay* d = nullptr;
          ReconstructedDecay rd = decay.m_daughterDecays[i];
          //we must make sure that m_graphParticle always corresponds to the same thing in the reconstructed thing.
          if (rd.m_behavior == c_ReconstructFrom) {
            ASSERT_NE(rd.m_optDecay, nullptr);
            d = rd.m_optDecay;
            rd.m_optDecay = nullptr;
            rd.m_behavior = Decay::c_Default;
          } else {
            ASSERT_TRUE(decay.m_daughterDecays.size() > i);
            d = &(mcDecay->m_daughterDecays[i]);
          }
          d->reconstruct({rd});
          if (d->m_particle)
            daughterIndices.push_back(d->m_particle->getArrayIndex());

        }
        if (decay.m_pdg != 0) {
          //is decay self conjugated?
          std::vector<int> decaylist, decaybarlist;
          for (int idx : daughterIndices) {
            const Particle* daughterPart = particles[idx];
            int daughterPDG = daughterPart->getPDGCode();
            decaylist.push_back(daughterPDG);
            decaybarlist.push_back((daughterPart->getFlavorType() == Particle::c_Flavored) ? (-daughterPDG) : daughterPDG);
          }
          std::sort(decaylist.begin(), decaylist.end());
          std::sort(decaybarlist.begin(), decaybarlist.end());
          bool isUnflavored = (decaylist == decaybarlist);

          m_particle = particles.appendNew(TLorentzVector(), decay.m_pdg, isUnflavored ? (Particle::c_Unflavored) : (Particle::c_Flavored),
                                           daughterIndices);
        }
      }
    }

    string getString() const { return "Particles(MCParticles,MCMatch,Flags):\n" + getStringInternal(); }

    int m_pdg; /**< PDG code of this MCParticle. */
    vector<Decay> m_daughterDecays; /**< decay products. */
    MCParticleGraph::GraphParticle*
    m_graphParticle; /**< GraphParticle (derived from MCParticle) corresponding to pdg. Linked to the specified daughters. */
    MCParticle* m_mcparticle; /**< corresponding MCParticle. if finalize() hasn't been called, = nullptr. */
    Particle* m_particle; /**< corresponding Particle. if reconstruct() hasn't been called, = nullptr. */

  private:
    /** implementation of getString(), without descriptive prefix. */
    string getStringInternal(int depth = 0) const
    {
      stringstream s;
      string spaces;
      for (int i = 0; i < depth; i++)
        spaces += "    ";
      s << spaces;

      if (m_particle)
        s << m_particle->getPDGCode();
      else
        s << "?";

      s << " (";
      if (m_mcparticle)
        s << m_mcparticle->getPDG();
      else
        s << "?";
      const MCParticle* mcMatch = nullptr;
      if (m_particle)
        mcMatch = m_particle->getRelated<MCParticle>();
      if (mcMatch) {
        s << ", " << mcMatch->getPDG() << ", ";
        if (m_particle->hasExtraInfo(MCMatching::c_extraInfoMCErrors))
          s << MCMatching::explainFlags(m_particle->getExtraInfo(MCMatching::c_extraInfoMCErrors));
        else
          s << "-not set-";
      } else {
        s << ", ?";
        s << ", ?";
      }
      s << ") ";

      if (!m_daughterDecays.empty()) {
        s << " [";
        for (const Decay& d : m_daughterDecays) {
          s << "\n" << d.getStringInternal(depth + 1);
        }

        s << "\n" << spaces << "]";
      }

      return s.str();
    }
  };

  /** Test fixture. */
  class MCMatchingTest : public ::testing::Test {
  protected:
    /** register Particle array + ParticleExtraInfoMap object. */
    virtual void SetUp()
    {
      StoreObjPtr<ParticleExtraInfoMap> particleExtraInfo;
      StoreArray<Particle> particles;
      StoreArray<MCParticle> mcparticles;
      particleExtraInfo.registerInDataStore();
      particles.registerInDataStore();
      mcparticles.registerInDataStore();
      particles.registerRelationTo(mcparticles);
    }

    /** clear datastore */
    virtual void TearDown()
    {
      DataStore::Instance().reset();
    }
  };


  /** adding mcparticles to the graph is weird, let's see if it actually works. */
  TEST_F(MCMatchingTest, MCParticleGraph)
  {
    Decay d(111, {22, 22});
    StoreArray<MCParticle> mcparticles;
    //actually push things into StoreArray
    d.finalize();
    EXPECT_EQ(mcparticles.getEntries(), 3);
    EXPECT_EQ(mcparticles[0]->getPDG(), 111);
    EXPECT_EQ(mcparticles[1]->getPDG(), 22);
    EXPECT_EQ(mcparticles[2]->getPDG(), 22);
    EXPECT_EQ(mcparticles[0]->getMother(), nullptr);
    EXPECT_EQ(mcparticles[1]->getMother(), mcparticles[0]);
    EXPECT_EQ(mcparticles[2]->getMother(), mcparticles[0]);

    Decay e(111, {22, 22});
    e.finalize();
    EXPECT_EQ(mcparticles.getEntries(), 6);
    EXPECT_EQ(mcparticles[3]->getPDG(), 111);
    EXPECT_EQ(mcparticles[3]->getNDaughters(), 2);
    EXPECT_EQ(mcparticles[4]->getPDG(), 22);
    EXPECT_EQ(mcparticles[5]->getPDG(), 22);
    EXPECT_EQ(mcparticles[4]->getNDaughters(), 0);
    EXPECT_EQ(mcparticles[5]->getNDaughters(), 0);
    EXPECT_EQ(mcparticles[3]->getMother(), nullptr);
    EXPECT_EQ(mcparticles[4]->getMother(), mcparticles[3]);
    EXPECT_EQ(mcparticles[5]->getMother(), mcparticles[3]);

    Decay f(211);
    f.finalize();
    EXPECT_EQ(mcparticles.getEntries(), 7);
    EXPECT_EQ(mcparticles[6]->getPDG(), 211);

    Decay g(421, {321, -211, {111, {22, 22}}});
    g.finalize();
    EXPECT_EQ(3, g.m_mcparticle->getNDaughters());
    EXPECT_EQ(mcparticles.getEntries(), 13);
  }

  /** adding reconstructed particles. */
  TEST_F(MCMatchingTest, CorrectReconstruction)
  {
    StoreArray<MCParticle> mcparticles;
    StoreArray<Particle> particles;

    Decay d(421, {321, -211, {111, {22, 22}}});
    d.reconstruct({421, {321, -211, {111, {22, 22}}}});
    //reconstruct() calls finalize(), so MCParticles are filled now
    EXPECT_EQ(mcparticles.getEntries(), 6);
    EXPECT_EQ(mcparticles[0]->getPDG(), 421);
    EXPECT_EQ(mcparticles[5]->getPDG(), 22);
    EXPECT_EQ(particles.getEntries(), 6);

    ASSERT_NE(d.m_particle, nullptr);
    const auto& fspParticles = d.m_particle->getFinalStateDaughters();
    EXPECT_EQ(fspParticles.size(), 4u);
    //all final state particles should have relations...
    for (const Particle* p : fspParticles) {
      EXPECT_EQ(p->getRelated<MCParticle>()->getDaughters().size(), 0u);
    }
    //composite particles don't have them
    EXPECT_TRUE(mcparticles[0] == d.m_mcparticle);
    EXPECT_TRUE(mcparticles[0]->getRelated<Particle>() == nullptr);
    EXPECT_TRUE(mcparticles[3]->getRelated<Particle>() == nullptr);

    //run MC matching (should be able to set a relation)
    ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();

    EXPECT_EQ(MCMatching::c_Correct, MCMatching::getMCErrors(d.m_particle)) << d.getString();

  }
  /** all information should be empty, and we should get errors if we try to get flags */
  TEST_F(MCMatchingTest, SetMCTruthNeverRun)
  {
    Decay d(421, {321, -211, {111, {22, 22}}});
    d.reconstruct({421, {211, -211, {111, {22, 22}}}});

    EXPECT_EQ(nullptr, d.m_particle->getRelated<MCParticle>());
    ASSERT_FALSE(d.m_particle->hasExtraInfo(MCMatching::c_extraInfoMCErrors));
    EXPECT_EQ(MCMatching::c_InternalError, MCMatching::getMCErrors(d.m_particle)) << d.getString();
  }

  TEST_F(MCMatchingTest, SettingTruths)
  {
    Decay d(421, {321, -211, {111, {22, 22}}});
    d.reconstruct({421, {211, -211, {111, {22, 22}}}});

    //setMCTruth should set relation
    ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
    EXPECT_EQ(d.m_mcparticle->getPDG(), d.m_particle->getRelated<MCParticle>()->getPDG());

    //but no extra-info flags
    ASSERT_FALSE(d.m_particle->hasExtraInfo(MCMatching::c_extraInfoMCErrors));
    ASSERT_FALSE(d.getParticle(111)->hasExtraInfo(MCMatching::c_extraInfoMCErrors));

    EXPECT_EQ(MCMatching::c_MisID, MCMatching::getMCErrors(d.m_particle)) << d.getString();

    //now it's set
    ASSERT_TRUE(d.m_particle->hasExtraInfo(MCMatching::c_extraInfoMCErrors));
    ASSERT_TRUE(d.getParticle(111)->hasExtraInfo(MCMatching::c_extraInfoMCErrors));
  }

  /** test misID flag. */
  TEST_F(MCMatchingTest, MisID)
  {
    {
      Decay d(421, {321, -211, {111, {22, 22}}});
      d.reconstruct({421, {211, -211, {111, {22, 22}}}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(d.m_mcparticle->getPDG(), d.m_particle->getRelated<MCParticle>()->getPDG());
      EXPECT_EQ(MCMatching::c_MisID, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
    {
      //+ wrong non-FSP
      Decay d(421, {321, -211, {111, {22, 22}}});
      d.reconstruct({413, {321, -13, {111, {22, 22}}}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::c_MisID | MCMatching::c_AddedWrongParticle, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
    {
      Decay d(421, {321, -211, {111, {22, 22}}});
      d.reconstruct({421, {321, 13, {111, {22, 22}}}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::c_MisID, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
    {
      Decay d(421, {321, -211, {111, {22, 22}}});
      d.reconstruct({421, {211, 13, {111, {22, 22}}}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::c_MisID, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
    {
      //pion and kaon switched
      Decay d(421, {321, -211, {111, {22, 22}}});
      d.reconstruct({421, { -211, 321, {111, {22, 22}}}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::c_MisID, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
  }

  /** missing particles. */
  TEST_F(MCMatchingTest, MissingParticles)
  {
    {
      //pi0 is not FSP, so doesn't get MCMatching::c_MissMassiveParticle (but MCMatching::c_MissingResonance)
      Decay d(421, {321, -211, {111, {22, 22}}});
      d.reconstruct({421, {321, -211, {0}}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::c_MissGamma | MCMatching::c_MissingResonance, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
    {
      Decay d(421, {321, -211, {111, {22, 22}}});
      d.reconstruct({421, {321, 0, {111, {22, 22}}}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::getMCErrors(d.m_particle), MCMatching::c_MissMassiveParticle) << d.getString();
    }
    {
      Decay d(421, {321, -211, {111, {22, 22}}});
      d.reconstruct({421, {0, -211, {111, {22, 22}}}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::getMCErrors(d.m_particle), MCMatching::c_MissMassiveParticle) << d.getString();
    }
    {
      Decay d(421, {321, -211, {111, {22, 22}}});
      d.reconstruct({421, {0, -13, {111, {22, 22}}}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::getMCErrors(d.m_particle), MCMatching::c_MissMassiveParticle | MCMatching::c_MisID) << d.getString();
    }
  }
  TEST_F(MCMatchingTest, KLongCorrect)
  {
    //correct (we miss the 'K0' resonance, but that's fine)
    Decay d(431, { {323, {321, {111, {22, 22}} }}, { -311, {130}}});
    d.finalize();
    //K0L and daughters are secondary
    MCParticle* k0l = d.getMCParticle(130);
    k0l->setStatus(k0l->getStatus() & (~MCParticle::c_PrimaryParticle)); //remove c_PrimaryParticle

    d.reconstruct({431, { {323, {321, {111, {22, 22}} }}, {130, {}, Decay::c_ReconstructFrom, d.getDecay(130)}}});
    ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
    EXPECT_EQ(MCMatching::c_MissingResonance, MCMatching::getMCErrors(d.m_particle)) << d.getString();
  }
  TEST_F(MCMatchingTest, KLongMissed)
  {
    //K0L not reconstructed
    Decay d(431, { {323, {321, {111, {22, 22}} }}, { -311, {130}}});
    d.finalize();
    //K0L and daughters are secondary
    MCParticle* k0l = d.getMCParticle(130);
    k0l->setStatus(k0l->getStatus() & (~MCParticle::c_PrimaryParticle)); //remove c_PrimaryParticle

    d.reconstruct({431, { {323, {321, {111, {22, 22}} }}, 0}});
    ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
    EXPECT_EQ(MCMatching::c_MissKlong | MCMatching::c_MissMassiveParticle | MCMatching::c_MissingResonance,
              MCMatching::getMCErrors(d.m_particle)) << d.getString();
  }
  TEST_F(MCMatchingTest, KShortCorrect)
  {
    //correct (we miss the 'K0' resonance, but that's fine)
    Decay d(431, { {323, {321, {111, {22, 22}} }}, { -311, {{310, {211, -211}}}}});
    d.finalize();
    //K0S and daughters are secondary
    MCParticle* k0s = d.getMCParticle(310);
    k0s->setStatus(k0s->getStatus() & (~MCParticle::c_PrimaryParticle)); //remove c_PrimaryParticle
    MCParticle* pi1 = d.getMCParticle(211);
    pi1->setStatus(pi1->getStatus() & (~MCParticle::c_PrimaryParticle));
    MCParticle* pi2 = d.getMCParticle(-211);
    pi2->setStatus(pi1->getStatus() & (~MCParticle::c_PrimaryParticle));

    d.reconstruct({431, { {323, {321, {111, {22, 22}} }}, {310, {{211, {}, Decay::c_ReconstructFrom, d.getDecay(211)}, { -211, {}, Decay::c_ReconstructFrom, d.getDecay(-211)}}}}});
    ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
    EXPECT_EQ(MCMatching::c_MissingResonance, MCMatching::getMCErrors(d.m_particle)) << d.getString();
  }
  TEST_F(MCMatchingTest, KShortMissed)
  {
    //K0S not reconstructed
    Decay d(431, { {323, {321, {111, {22, 22}} }}, { -311, {{310, {211, -211}}}}});
    d.finalize();
    //K0S and daughters are secondary
    MCParticle* k0s = d.getMCParticle(310);
    k0s->setStatus(k0s->getStatus() & (~MCParticle::c_PrimaryParticle)); //remove c_PrimaryParticle
    MCParticle* pi1 = d.getMCParticle(211);
    pi1->setStatus(pi1->getStatus() & (~MCParticle::c_PrimaryParticle));
    MCParticle* pi2 = d.getMCParticle(-211);
    pi2->setStatus(pi1->getStatus() & (~MCParticle::c_PrimaryParticle));

    d.reconstruct({431, { {323, {321, {111, {22, 22}} }}, 0}});
    ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
    EXPECT_EQ(MCMatching::c_MissMassiveParticle | MCMatching::c_MissingResonance,
              MCMatching::getMCErrors(d.m_particle)) << d.getString();
  }
  /** more missing particles. */
  TEST_F(MCMatchingTest, PionWithOneGamma)
  {
    {
      StoreArray<Particle> particles;
      StoreArray<MCParticle> mcparticles;
      Decay d(421, {321, -211, {111, {22, 22}}});
      d.reconstruct({421, {321, -211, {111, {0, 22}}}});
      EXPECT_EQ(mcparticles.getEntries(), 6);
      EXPECT_EQ(particles.getEntries(), 5); //we added only 5 Particles
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::c_MissGamma, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
    {
      Decay d(421, {321, -211, {111, {22, 22}}});
      d.reconstruct({421, {321, 0, {111, {0, 22}}}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::c_MissMassiveParticle | MCMatching::c_MissGamma, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
  }
  TEST_F(MCMatchingTest, TauWithResonance)
  {
    // Correct tau
    {
      StoreArray<Particle> particles;
      StoreArray<MCParticle> mcparticles;
      Decay d(15, {16, { -213, { -211, {111, {22, 22}}}}});
      d.reconstruct({15, {0, { -213, { -211, {111, {22, 22}}}}}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::c_MissNeutrino, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
    // Miss Resoanace
    {
      StoreArray<Particle> particles;
      StoreArray<MCParticle> mcparticles;
      Decay d(15, {16, { -213, { -211, {111, {22, 22}}}}});
      d.reconstruct({15, {{ -211, {}, Decay::c_ReconstructFrom, &d[1][0]}, {111, {22, 22}, Decay::c_ReconstructFrom, &d[1][1]}}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::c_MissingResonance, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
    // Miss Gamma
    {
      StoreArray<Particle> particles;
      StoreArray<MCParticle> mcparticles;
      Decay d(15, {16, { -213, { -211, {111, {22, 22}}}}});
      d.reconstruct({15, {{ -211, {}, Decay::c_ReconstructFrom, &d[1][0]}, {111, {0, {22, {}, Decay::c_ReconstructFrom, &d[1][1][1]}},  Decay::c_ReconstructFrom, &d[1][1]}}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::c_MissGamma | MCMatching::c_MissingResonance, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
    // Added Wrong Pion
    {
      StoreArray<Particle> particles;
      StoreArray<MCParticle> mcparticles;
      Decay g(-512, {211, -211, -16, {15, {16, { -213, { -211, {111, {22, 22}}}}}}});
      Decay& d = g[3];
      d.reconstruct({15, {{ -211, {}, Decay::c_ReconstructFrom, &g[1]}, {111, {22, 22}, Decay::c_ReconstructFrom, &d[1][1]}}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::c_MissMassiveParticle | MCMatching::c_MissingResonance | MCMatching::c_MissNeutrino |
                MCMatching::c_AddedWrongParticle,
                MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }


  }
  TEST_F(MCMatchingTest, MissGamma)
  {
    {
      //D*+ -> D+ pi0
      Decay d(431, {{421, {321, -211, {111, {22, 22}}}}, {111, {22, 22}}});
      d.reconstruct({431, {{421, {321, -211, {111, {22, 22}}}}, {111, {22, 22}}}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::c_Correct, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
    {
      //D*+ -> D+ pi0 reconstructed as D*+ -> D+ gamma
      Decay d(431, {{421, {321, -211, {111, {22, 22}}}}, {111, {22, 22}}});
      Decay& gamma = d[1][0]; //first gamma from second pi0
      d.reconstruct({431, {{421, {321, -211, {111, {22, 22}}}}, {22, {}, Decay::c_ReconstructFrom, &gamma}}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::c_MissGamma | MCMatching::c_MissingResonance, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }

    {
      //D*+ -> D+ gamma
      Decay d(431, {{421, {321, -211, {111, {22, 22}}}}, 22});
      d.reconstruct({431, {{421, {321, -211, {111, {22, 22}}}}, 22}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::c_Correct, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
    {
      //pi0 -> e+ e- gamma
      Decay d(111, {11, -11, 22});
      d.reconstruct({111, {11, -11, 22}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::c_Correct, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
    {
      //pi0 -> e+ e- gamma reconstructed without gamma
      Decay d(111, {11, -11, 22});
      d.reconstruct({111, {11, -11, 0}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::c_MissGamma, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
    {
      //pi0 -> 2 gamma, with both clusters coming from same photon
      Decay d(111, {22, 22});
      Decay& gamma = d[0];
      d.reconstruct({111, {22, {22, {}, Decay::c_ReconstructFrom, &gamma}}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      //MCMatch of pi0 is first gamma
      EXPECT_EQ(MCMatching::c_AddedWrongParticle, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
    {
      //pi0 -> 4 gamma
      Decay d(111, {22, 22, 22, 22});
      d.reconstruct({111, {22, 22, 22, 22}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::c_Correct, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
    {
      //pi0 -> 4 gamma as 2 gamma
      Decay d(111, {22, 22, 22, 22});
      d.reconstruct({111, {22, 22, 0, 0}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::c_MissGamma, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
    {
      //pi0 -> 4 gamma, with two clusters coming from same photon
      Decay d(111, {22, 22, 22, 22});
      Decay& gamma = d[0];
      d.reconstruct({111, {22, 22, 22, {22, {}, Decay::c_ReconstructFrom, &gamma}}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::c_MissGamma, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
  }
  /** photon 'reconstructed' from a pi+ track, combined into other stuff. */
  TEST_F(MCMatchingTest, WrongPhotonForPi0)
  {
    {
      Decay d(521, {211, Decay(421, {321, -211, Decay(111, {22, 22})}), 22});
      d.finalize();
      d.reconstruct({521, {211, {421, {321, -211, {111, {{22}, {22, {}, Decay::c_RelateWith, d.getMCParticle(211)}}}}}}});
      //result: pi0 gets MC match 521. Gets misID & c_AddedWrongParticle because of 'wrong' photon, plus c_MissMassiveParticle since the B's daughters are missing, plus c_MissGamma because one photon was not reconstructed, plus c_MissingResonance because non-FSPs were missed (MC matched particle (521) has lots of daughters)
      Particle* pi0 = d.getParticle(111);
      Decay* pi0decay = d.getDecay(111);
      ASSERT_TRUE(MCMatching::setMCTruth(pi0)) << pi0decay->getString();
      EXPECT_EQ(521, pi0->getRelated<MCParticle>()->getPDG());
      EXPECT_EQ(MCMatching::c_MisID | MCMatching::c_AddedWrongParticle | MCMatching::c_MissMassiveParticle | MCMatching::c_MissGamma |
                MCMatching::c_MissingResonance,
                MCMatching::getMCErrors(pi0)) << pi0decay->getString();

      //flags migrate upstream
      Particle* p = d.getParticle(421);
      Decay* d0decay = d.getDecay(421);
      ASSERT_TRUE(MCMatching::setMCTruth(p)) << d0decay->getString();
      EXPECT_EQ(MCMatching::c_MisID | MCMatching::c_AddedWrongParticle | MCMatching::c_MissGamma | MCMatching::c_MissingResonance,
                MCMatching::getMCErrors(p)) << d0decay->getString();

      //even with addedWrongParticle (inherited from daughter), missFSR should be detected.
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::c_MisID | MCMatching::c_AddedWrongParticle | MCMatching::c_MissGamma | MCMatching::c_MissingResonance,
                MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
  }

  /** pi+ decays into muon, pi+ track is found. */
  TEST_F(MCMatchingTest, DecayInFlightCorrect)
  {
    {
      Decay d(421, {321, { -211, {13}}, {111, {22, 22}}});
      d.finalize();
      MCParticle* muon = d.getMCParticle(13);
      muon->setStatus(muon->getStatus() & (~MCParticle::c_PrimaryParticle)); //remove c_PrimaryParticle
      ASSERT_FALSE(muon->hasStatus(MCParticle::c_PrimaryParticle));
      d.reconstruct({421, {321, -211, {111, {22, 22}}}});

      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(d.m_mcparticle->getPDG(), d.m_particle->getRelated<MCParticle>()->getPDG());
      EXPECT_EQ(MCMatching::c_Correct, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
    {
      Decay d(421, {{321, {11, -12, {111, {22,  22}}}}, -211, {111, {22, 22}}});
      d.reconstruct({421, {321, -211, {111, {22, 22}}}});

      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(d.m_mcparticle->getPDG(), d.m_particle->getRelated<MCParticle>()->getPDG());
      EXPECT_EQ(MCMatching::c_Correct, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
  }
  /** pi+ decays into muon, muon track with pi+ hypothesis in reconstruction. */
  TEST_F(MCMatchingTest, DecayInFlight)
  {
    {
      Decay d(-211, {13});
      d.finalize();
      MCParticle* muon = d.getMCParticle(13);
      muon->setStatus(muon->getStatus() & (~MCParticle::c_PrimaryParticle)); //remove c_PrimaryParticle
      d.reconstruct({ -211, {}, Decay::c_RelateWith, muon});

      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(d.m_mcparticle->getPDG(), d.m_particle->getRelated<MCParticle>()->getPDG());
      EXPECT_EQ(MCMatching::c_DecayInFlight, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
    {
      Decay d(421, {321, { -211, {13}}, {111, {22, 22}}});
      d.finalize();
      MCParticle* muon = d.getMCParticle(13);
      muon->setStatus(muon->getStatus() & (~MCParticle::c_PrimaryParticle)); //remove c_PrimaryParticle
      d.reconstruct({421, {321, { -211, {}, Decay::c_RelateWith, muon}, {111, {22, 22}}}});


      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(d.m_mcparticle->getPDG(), d.m_particle->getRelated<MCParticle>()->getPDG());
      EXPECT_EQ(MCMatching::c_DecayInFlight, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
  }
  /** we reconstrcut D*+ -> D0 pi+, but it's actually D+ pi0. */
  TEST_F(MCMatchingTest, CorrectFSPsWrongDecay)
  {
    {
      Decay d(-413, {{ -411, {321, -211, -211}}, {111, {22, 22}}});

      Decay& pi0 = d[1];
      Decay& pi2 = d[0][2];

      ASSERT_TRUE(d.getDecay(111) ==  &pi0);

      d.reconstruct({ -413, {{ -421, {321, -211, {111, {22, 22}, Decay::c_ReconstructFrom, &pi0}}}, { -211, {}, Decay::c_ReconstructFrom, &pi2}}});


      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(d.m_mcparticle->getPDG(), d.m_particle->getRelated<MCParticle>()->getPDG());
      //inherits c_AddedWrongParticle from D0, gets c_MissingResonance since D+ is missing
      EXPECT_EQ(MCMatching::c_AddedWrongParticle | MCMatching::c_MissingResonance,
                MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
  }
  /** Correctly reconstructed decay, except we switched some tracks around. */
  TEST_F(MCMatchingTest, WrongCombination)
  {
    /** decay correctly reconstructed, but we messed up the assignment of pion tracks */
    {
      Decay d(-413, {{ -421, {321, -211, {111, {22, 22}}}}, -211});

      Decay* pi1 = &(d[0][1]);
      Decay* pi2 = &(d[1]);
      ASSERT_TRUE(pi1->m_pdg == pi2->m_pdg);

      d.reconstruct({ -413, {{ -421, {321, { -211, {}, Decay::c_ReconstructFrom, pi2}, {111, {22, 22}}}}, { -211, {}, Decay::c_ReconstructFrom, pi1}}});


      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(d.m_mcparticle->getPDG(), d.m_particle->getRelated<MCParticle>()->getPDG());
      //gets c_MissingResonance since none of the Particles got the D0 MCParticle as MC match
      EXPECT_EQ(MCMatching::c_AddedWrongParticle | MCMatching::c_MissingResonance,
                MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }

    /** B0 -> phi [K+ K-] phi [K+ K-] with Ks from both sides switched*/
    {
      Decay d(511, {{333, {321, -321}}, {333, {321, -321}}});

      Decay* k1 = &(d[0][1]);
      Decay* k2 = &(d[1][1]);
      ASSERT_TRUE(k1->m_pdg == k2->m_pdg);

      d.reconstruct({511, {
          {333, {321, { -321, {}, Decay::c_ReconstructFrom, k2}}},
          {333, {321, { -321, {}, Decay::c_ReconstructFrom, k1}}}
        }
      });


      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(d.m_mcparticle->getPDG(), d.m_particle->getRelated<MCParticle>()->getPDG());
      //gets c_MissingResonance since none of the Particles got the phi MCParticle as MC match
      EXPECT_EQ(MCMatching::c_AddedWrongParticle | MCMatching::c_MissingResonance,
                MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
  }
  /** Reconstruct both Bs, but switch pi0 to other B */
  TEST_F(MCMatchingTest, SelfCrossFeed)
  {
    {
      Decay d(300533, {{511, {321, -211, {111, {22, 22}}}}, { -511, { -321, 211, {111, {22, 22}}}}});

      Decay* pi1 = &(d[0][2]);
      Decay* pi2 = &(d[1][2]);
      ASSERT_TRUE(pi1->m_pdg == pi2->m_pdg);

      d.reconstruct({300533, {
          {511, {321, -211, {111, {22, 22}, Decay::c_ReconstructFrom, pi2}}},
          { -511, { -321, 211, {111, {22, 22}, Decay::c_ReconstructFrom, pi1}}}
        }
      });


      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(d.m_mcparticle->getPDG(), d.m_particle->getRelated<MCParticle>()->getPDG());
      //gets c_MissingResonance since none of the Particles got the B0/anti-B0 MCParticle as MC match
      EXPECT_EQ(MCMatching::c_AddedWrongParticle | MCMatching::c_MissingResonance,
                MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
  }

  TEST_F(MCMatchingTest, FlavouredD0Decay)
  {
    {
      //ok
      Decay d(421, { -321, 211});
      d.reconstruct({421, { -321, 211}});
      ASSERT_EQ(Particle::c_Flavored, d.m_particle->getFlavorType());

      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(d.m_particle->getPDGCode(), d.m_particle->getRelated<MCParticle>()->getPDG());
      EXPECT_EQ(MCMatching::c_Correct, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
    {
      //also exists, but suppressed
      Decay d(-421, { -321, 211});
      d.reconstruct({ -421, { -321, 211}});
      ASSERT_EQ(Particle::c_Flavored, d.m_particle->getFlavorType());

      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(d.m_particle->getPDGCode(), d.m_particle->getRelated<MCParticle>()->getPDG());
      EXPECT_EQ(MCMatching::c_Correct, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
    {
      //however, reconstructing the wrong D0 is not okay
      Decay d(421, { -321, 211});
      d.reconstruct({ -421, { -321, 211}});
      ASSERT_EQ(Particle::c_Flavored, d.m_particle->getFlavorType());

      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::c_AddedWrongParticle, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
    {
      //however, reconstructing the wrong D0 is not okay
      Decay d(-421, { -321, 211});
      d.reconstruct({421, { -321, 211}});
      ASSERT_EQ(Particle::c_Flavored, d.m_particle->getFlavorType());

      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::c_AddedWrongParticle, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
  }

  TEST_F(MCMatchingTest, UnflavouredD0Decay)
  {
    {
      //ok
      Decay d(421, { -321, 321});
      d.reconstruct({421, { -321, 321}});
      ASSERT_EQ(Particle::c_Unflavored, d.m_particle->getFlavorType());

      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(d.m_mcparticle->getPDG(), d.m_particle->getRelated<MCParticle>()->getPDG());
      EXPECT_EQ(MCMatching::c_Correct, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
    {
      //ok
      Decay d(-421, { -321, 321});
      d.reconstruct({ -421, { -321, 321}});
      ASSERT_EQ(Particle::c_Unflavored, d.m_particle->getFlavorType());

      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(d.m_mcparticle->getPDG(), d.m_particle->getRelated<MCParticle>()->getPDG());
      EXPECT_EQ(MCMatching::c_Correct, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
    {
      //we don't know the flavour, so this is also fine
      Decay d(421, { -321, 321});
      d.reconstruct({ -421, { -321, 321}});
      ASSERT_EQ(Particle::c_Unflavored, d.m_particle->getFlavorType());

      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::c_Correct, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
    {
      //we don't know the flavour, so this is also fine
      Decay d(-421, { -321, 321});
      d.reconstruct({421, { -321, 321}});
      ASSERT_EQ(Particle::c_Unflavored, d.m_particle->getFlavorType());

      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::c_Correct, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
  }

  //B0 -> rho+ D-
  TEST_F(MCMatchingTest, MissingResonance)
  {
    //explicitly reconstruct the rho
    {
      Decay d(511, {{ -411, { -321, 321, 211}}, {213, {211, {111, {22, 22}}}}});
      d.reconstruct({511, {{ -411, { -321, 321, 211}}, {213, {211, {111, {22, 22}}}}}});

      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::c_Correct, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
    //missing rho
    {
      Decay d(511, {{ -411, { -321, 321, 211}}, {213, {211, {111, {22, 22}}}}});
      //d.reconstruct({511, {{-411, {-321, 321, 211}}, {0, {211, {111, {22, 22}}}}}});
      Decay* piplus = &(d[1][0]);
      Decay* pi0 = &(d[1][1]);

      d.reconstruct({511, {{ -411, { -321, 321, 211}}, {211, {}, Decay::c_ReconstructFrom, piplus}, {111, {22, 22}, Decay::c_ReconstructFrom, pi0}}});

      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(MCMatching::c_MissingResonance, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
  }

  TEST_F(MCMatchingTest, MissingFSPReplacedBySecondary)
  {
    Decay d(521, {{ -421, {321, { -211, {22, -211}}}}, 211, 211, -211});
    Decay* pi = &(d[0][1][1]);
    d.reconstruct({521, {{ -421, {321, -211}}, 211, 211, {
          -211, {},
          Decay::c_ReconstructFrom, pi
        }
      }
    });
    ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
    EXPECT_EQ(MCMatching::c_MissMassiveParticle, MCMatching::getMCErrors(d.m_particle)) << d.getString();
  }
  TEST_F(MCMatchingTest, BeamBackground)
  {
    Decay d(421, {321, -211, {111, {22, 22}}});
    d.finalize();
    MCParticle* noParticle = nullptr;
    d.reconstruct({421, {321, { -211, {}, Decay::c_RelateWith, noParticle}, {111, {22, 22}}}});

    //no common mother
    ASSERT_FALSE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
    EXPECT_EQ(MCMatching::c_InternalError, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    EXPECT_EQ(nullptr, d.m_particle->getRelated<MCParticle>()) << d.getString();
  }

  TEST_F(MCMatchingTest, DuplicateTrack)
  {
    {
      Decay d(421, {321, -211, {111, {22, 22}}});
      d.finalize();
      MCParticle* kaon = d.getMCParticle(321);
      ASSERT_TRUE(kaon != nullptr);
      //tracks for the same MCParticle used twice:
      d.reconstruct({421, {321, { -211, {}, Decay::c_RelateWith, kaon}, {111, {22, 22}}}});

      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(d.m_mcparticle->getPDG(), d.m_particle->getRelated<MCParticle>()->getPDG());
      EXPECT_EQ(MCMatching::c_MissMassiveParticle | MCMatching::c_MisID, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
  }

  /** distinction between FSR/PHOTOS and real photons. */
  TEST_F(MCMatchingTest, MissingFSRMissingPHOTOS)
  {
    {
      Decay d(521, { -11, 12, 22});
      d.finalize();
      MCParticle* photon = d.getMCParticle(22);
      EXPECT_FALSE(MCMatching::isFSR(photon));

      d.reconstruct({521, { -11}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(d.m_mcparticle->getPDG(), d.m_particle->getRelated<MCParticle>()->getPDG());
      EXPECT_EQ(MCMatching::c_MissGamma | MCMatching::c_MissNeutrino, MCMatching::getMCErrors(d.m_particle)) << d.getString();

    }
    {
      //ISR flag should not trigger anything special
      Decay d(521, { -11, 12, 22});
      d.finalize();
      MCParticle* photon = d.getMCParticle(22);
      photon->setStatus(MCParticle::c_PrimaryParticle | MCParticle::c_IsISRPhoton);
      EXPECT_FALSE(MCMatching::isFSR(photon));

      d.reconstruct({521, { -11}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(d.m_mcparticle->getPDG(), d.m_particle->getRelated<MCParticle>()->getPDG());
      EXPECT_EQ(MCMatching::c_MissGamma | MCMatching::c_MissNeutrino, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }

    {
      //now try the same with FSR flag
      Decay d(521, { -11, 12, 22});
      d.finalize();
      MCParticle* photon = d.getMCParticle(22);
      photon->setStatus(MCParticle::c_PrimaryParticle | MCParticle::c_IsFSRPhoton);
      EXPECT_TRUE(MCMatching::isFSR(photon));

      d.reconstruct({521, { -11}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(d.m_mcparticle->getPDG(), d.m_particle->getRelated<MCParticle>()->getPDG());
      EXPECT_EQ(MCMatching::c_MissFSR | MCMatching::c_MissNeutrino, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }

    {
      //now try PHOTOS photon
      Decay d(521, { -11, 12, 22});
      d.finalize();
      MCParticle* photon = d.getMCParticle(22);
      photon->setStatus(MCParticle::c_PrimaryParticle | MCParticle::c_IsPHOTOSPhoton);
      EXPECT_FALSE(MCMatching::isFSR(photon));

      d.reconstruct({521, { -11}});
      ASSERT_TRUE(MCMatching::setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(d.m_mcparticle->getPDG(), d.m_particle->getRelated<MCParticle>()->getPDG());
      EXPECT_EQ(MCMatching::c_MissPHOTOS | MCMatching::c_MissNeutrino, MCMatching::getMCErrors(d.m_particle)) << d.getString();
    }
  }
}  // namespace
#endif
