#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/MCParticleGraph.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>

#include <analysis/utility/mcParticleMatching.h>
#include <analysis/utility/MCMatchStatus.h>
#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;

#if defined(__ICC) || defined(__INTEL_COMPILER)
namespace {
  TEST(MCMatchingTest, TestsDisabled)
  {
    EXPECT_TRUE(false) << "MC matching test disabled on intel compiler (version < 14 sp1 update2), please see https://software.intel.com/en-us/forums/topic/475378";
  }
}
#else

namespace {
  MCParticleGraph gParticleGraph;

  /** is this a final state particle? */
  bool isFSP(int pdg)
  {
    switch (abs(pdg)) {
      case 211:
      case 321:
      case 11:
      case 13:
      case 2212:
      case 22:
        return true;
      default:
        return false;
    }
  }

  void printPart(const MCParticle* p, int depth = 0)
  {
    stringstream s("");
    for (int i = 0; i < depth; i++) {
      s << "    ";
    }
    s  << p->getPDG();
    B2WARNING(s.str() << " id: " << p->getIndex() << " status: " << p->getStatus());
    for (const auto * daughter : p->getDaughters()) {
      printPart(daughter, depth + 1);
    }
  }

  void printPart(const Particle* p, int depth = 0)
  {
    stringstream s("");
    for (int i = 0; i < depth; i++) {
      s << "    ";
    }
    s  << p->getPDGCode();
    const MCParticle* mcp = p->getRelated<MCParticle>();
    int mcpdg = mcp ? mcp->getPDG() : 0;
    B2WARNING(s.str() << " " << " -> mcpart " << mcpdg << " id " << (mcp ? mcp->getIndex() : 0));
    for (const auto * daughter : p->getDaughters()) {
      printPart(daughter, depth + 1);
    }
  }

  /** Helper for constructing MCParticle trees and reconstructing them. */
  struct Decay {
  public:
    /** Controls reconstruction behaviour. */
    enum EBehavior {
      c_Default, /**< for FSPs, relate with corresponding particle in created decay. */
      c_CreateNewMCParticle, /**< Create a new MCParticle and relate this particle with it. */
      c_RelateWithFirst, /**< Relate with mcparticles[0]. */
      c_RelateWith, /**< Relate with specified MCParticle. */
      c_ReconstructFrom, /**< Create Particle from given MCParticle (and associated daughters). */
    };
    /** create MCParticles for decay of particle with 'pdg' to given daguhterPDG codes. */
    Decay(int pdg, std::vector<Decay> daughters = std::vector<Decay>()):
      m_pdg(pdg), m_daughterDecays(daughters), m_mcparticle(nullptr), m_particle(nullptr) {
      m_graphParticle = &gParticleGraph.addParticle();
      m_graphParticle->setPDG(m_pdg);
      for (Decay & d : daughters) {
        gParticleGraph.addDecay(*m_graphParticle, *d.m_graphParticle);
      }
    }
    /** add all MCParticles created so far to data store. Must be called manually if you want to check MCParticles before calling reconstruct(). */
    void finalize() {
      gParticleGraph.generateList();
      gParticleGraph.clear(); //don't add them again in the next call..

      StoreArray<MCParticle> mcparticles;
      m_mcparticle = mcparticles[m_graphParticle->getIndex() - 1];

      for (Decay & d : m_daughterDecays)
        d.finalize();
    }

    /** get first Particle with matching PDG code. */
    Particle* getParticle(int pdg) const {
      if (m_pdg == pdg and m_particle)
        return m_particle;

      for (auto & d : m_daughterDecays) {
        Particle* res = d.getParticle(pdg);
        if (res)
          return res;
      }
      return nullptr;
    }
    /** get first MCParticle with matching PDG code. */
    MCParticle* getMCParticle(int pdg) const {
      if (m_pdg == pdg and m_mcparticle)
        return m_mcparticle;

      for (auto & d : m_daughterDecays) {
        MCParticle* res = d.getMCParticle(pdg);
        if (res)
          return res;
      }
      return nullptr;
    }
    /** get first Decay with matching PDG code. */
    Decay* getDecay(int pdg) {
      if (m_pdg == pdg)
        return this;

      for (auto & d : m_daughterDecays) {
        Decay* res = d.getDecay(pdg);
        if (res)
          return res;
      }
      return nullptr;
    }

    Decay& operator[](int i) { return m_daughterDecays[i]; }

    /** Helper for constructing Particles. */
    struct ReconstructedDecay {
      ReconstructedDecay(int pdg, std::vector<ReconstructedDecay> daughters = std::vector<ReconstructedDecay>(), EBehavior behavior = c_Default):
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
     * TODO: things like _other_ photon being the true one?
     * TODO: additional Particles?
     */
    void reconstruct(ReconstructedDecay decay) {
      if (!m_mcparticle) {
        finalize();
      }
      if (decay.m_behavior == c_CreateNewMCParticle) {
        m_graphParticle = &gParticleGraph.addParticle();
        m_graphParticle->setPDG(decay.m_pdg);
        finalize(); //overwrites m_mcparticle with the new particle
      } else if (decay.m_behavior == c_RelateWithFirst) {
        StoreArray<MCParticle> mcparticles;
        m_mcparticle = mcparticles[0];
        ASSERT_TRUE(m_mcparticle != nullptr);
      } else if (decay.m_behavior == c_RelateWith) {
        m_mcparticle = decay.m_optMcPart;
        ASSERT_TRUE(m_mcparticle != nullptr);
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
        ASSERT_EQ(decay.m_daughterDecays.size(), mcDecay->m_daughterDecays.size());
        std::vector<int> daughterIndices;
        for (unsigned int i = 0; i < decay.m_daughterDecays.size(); i++) {
          Decay* d = &(mcDecay->m_daughterDecays[i]);
          ReconstructedDecay rd = decay.m_daughterDecays[i];
          //we must make sure that m_graphParticle always corresponds to the same thing in the reconstructed thing.
          if (rd.m_behavior == c_ReconstructFrom) {
            ASSERT_TRUE(rd.m_optDecay != nullptr);
            Decay* mcDecay = rd.m_optDecay;
            rd.m_optDecay = nullptr;
            rd.m_behavior = Decay::c_Default;
            d = mcDecay;
          }
          d->reconstruct({rd});
          if (d->m_particle)
            daughterIndices.push_back(d->m_particle->getArrayIndex());

        }
        if (decay.m_pdg != 0) {
          m_particle = particles.appendNew(TLorentzVector(), decay.m_pdg, Particle::c_Unflavored, daughterIndices);
        }
      }
    }

    string getString() { return "Particles(MCParticles,MCMatch): " + getStringInternal(); }

    int m_pdg; /**< PDG code of this MCParticle. */
    vector<Decay> m_daughterDecays; /**< decay products. */
    MCParticleGraph::GraphParticle* m_graphParticle; /**< GraphParticle (derived from MCParticle) corresponding to pdg. Linked to the specified daughters. */
    MCParticle* m_mcparticle; /**< corresponding MCParticle. if finalize() hasn't been called, = nullptr. */
    Particle* m_particle; /**< corresponding Particle. if reconstruct() hasn't been called, = nullptr. */

  private:
    /** implementation of getString(), without descriptive prefix. */
    string getStringInternal() const {
      stringstream s;

      if (m_particle)
        s << m_particle->getPDGCode();
      else
        s << "?";

      s << "(";
      if (m_mcparticle)
        s << m_mcparticle->getPDG();
      else
        s << "?";
      const MCParticle* mcMatch = nullptr;
      if (m_particle)
        mcMatch = m_particle->getRelated<MCParticle>();
      if (mcMatch)
        s << "," << mcMatch->getPDG();
      else
        s << ",?";
      s << ") ";

      if (!m_daughterDecays.empty()) {
        s << "[";
        for (const Decay & d : m_daughterDecays) {
          s << d.getStringInternal();
        }

        s << "]";
      }

      return s.str();
    }
  };

  /** Test fixture. */
  class MCMatchingTest : public ::testing::Test {
  protected:
    /** register Particle array + ParticleExtraInfoMap object. */
    virtual void SetUp() {
      DataStore::Instance().setInitializeActive(true);
      StoreObjPtr<ParticleExtraInfoMap>::registerPersistent();
      StoreArray<Particle>::registerPersistent();
      StoreArray<MCParticle>::registerPersistent();
      RelationArray::registerPersistent<Particle, MCParticle>();
      DataStore::Instance().setInitializeActive(false);
    }

    /** clear datastore */
    virtual void TearDown() {
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
    EXPECT_EQ(mcparticles[4]->getPDG(), 22);
    EXPECT_EQ(mcparticles[5]->getPDG(), 22);
    EXPECT_EQ(mcparticles[3]->getMother(), nullptr);
    EXPECT_EQ(mcparticles[4]->getMother(), mcparticles[3]);
    EXPECT_EQ(mcparticles[5]->getMother(), mcparticles[3]);

    Decay f(211);
    f.finalize();
    EXPECT_EQ(mcparticles.getEntries(), 7);
    EXPECT_EQ(mcparticles[6]->getPDG(), 211);

    Decay g(421, {321, -211, {111, {22, 22}}});
    g.finalize();
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

    EXPECT_TRUE(d.m_particle != nullptr);
    const auto& fspParticles = d.m_particle->getFinalStateDaughters();
    EXPECT_EQ(fspParticles.size(), 4u);
    //all final state particles should have relations...
    for (const Particle * p : fspParticles) {
      EXPECT_EQ(p->getRelated<MCParticle>()->getDaughters().size(), 0u);
    }
    //composite particles don't have them
    EXPECT_TRUE(mcparticles[0] == d.m_mcparticle);
    EXPECT_TRUE(mcparticles[0]->getRelated<Particle>() == nullptr);
    EXPECT_TRUE(mcparticles[3]->getRelated<Particle>() == nullptr);

    //with no related mcparticle, we should get an error
    EXPECT_TRUE(getMCTruthStatus(d.m_particle, nullptr) == c_ERROR);

    //run MC matching (should be able to set a relation)
    ASSERT_TRUE(setMCTruth(d.m_particle)) << d.getString();

    EXPECT_EQ(getMCTruthStatus(d.m_particle, d.m_mcparticle), 0) << d.getString();

  }

  /** test misID flag. */
  TEST_F(MCMatchingTest, MisID)
  {
    {
      Decay d(421, {321, -211, {111, {22, 22}}});
      d.reconstruct({421, {211, -211, {111, {22, 22}}}});
      ASSERT_TRUE(setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(d.m_mcparticle->getPDG(), d.m_particle->getRelated<MCParticle>()->getPDG());
      EXPECT_EQ(getMCTruthStatus(d.m_particle, d.m_mcparticle), c_MisID) << d.getString();
    }
    {
      Decay d(421, {321, -211, {111, {22, 22}}});
      d.reconstruct({413, {321, -13, {111, {22, 22}}}});
      ASSERT_TRUE(setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(getMCTruthStatus(d.m_particle, d.m_mcparticle), c_MisID) << d.getString();
    }
    {
      Decay d(421, {321, -211, {111, {22, 22}}});
      d.reconstruct({421, {321, 13, {111, {22, 22}}}});
      ASSERT_TRUE(setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(getMCTruthStatus(d.m_particle, d.m_mcparticle), c_MisID) << d.getString();
    }
    {
      Decay d(421, {321, -211, {111, {22, 22}}});
      d.reconstruct({421, {211, 13, {111, {22, 22}}}});
      ASSERT_TRUE(setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(getMCTruthStatus(d.m_particle, d.m_mcparticle), c_MisID) << d.getString();
    }
    {
      //pion and kaon switched
      Decay d(421, {321, -211, {111, {22, 22}}});
      d.reconstruct({421, { -211, 321, {111, {22, 22}}}});
      ASSERT_TRUE(setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(getMCTruthStatus(d.m_particle, d.m_mcparticle), c_MisID) << d.getString();
    }
  }

  /** missing particles. */
  TEST_F(MCMatchingTest, MissingParticles)
  {
    {
      Decay d(421, {321, -211, {111, {22, 22}}});
      d.reconstruct({421, {321, -211, {0}}});
      ASSERT_TRUE(setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(getMCTruthStatus(d.m_particle, d.m_mcparticle), c_MissMassiveParticle) << d.getString();
    }
    {
      Decay d(421, {321, -211, {111, {22, 22}}});
      d.reconstruct({421, {321, 0, {111, {22, 22}}}});
      ASSERT_TRUE(setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(getMCTruthStatus(d.m_particle, d.m_mcparticle), c_MissMassiveParticle) << d.getString();
    }
    {
      Decay d(421, {321, -211, {111, {22, 22}}});
      d.reconstruct({421, {0, -211, {111, {22, 22}}}});
      ASSERT_TRUE(setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(getMCTruthStatus(d.m_particle, d.m_mcparticle), c_MissMassiveParticle) << d.getString();
    }
    {
      Decay d(421, {321, -211, {111, {22, 22}}});
      d.reconstruct({421, {0, -13, {111, {22, 22}}}});
      ASSERT_TRUE(setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(getMCTruthStatus(d.m_particle, d.m_mcparticle), c_MissMassiveParticle | c_MisID) << d.getString();
    }
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
      ASSERT_TRUE(setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(getMCTruthStatus(d.m_particle, d.m_mcparticle), c_MissGamma) << d.getString();
    }
    {
      Decay d(421, {321, -211, {111, {22, 22}}});
      d.reconstruct({421, {321, 0, {111, {0, 22}}}});
      ASSERT_TRUE(setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(getMCTruthStatus(d.m_particle, d.m_mcparticle), c_MissMassiveParticle | c_MissGamma) << d.getString();
    }
  }
  /** misIDed particle combined into something else. */
  TEST_F(MCMatchingTest, MisIDedPionInDReconstruction)
  {
    {
      Decay d(521, {211, {421, {321, -211, {111, {22, 22}}}}});
      d.reconstruct({521, {211, {421, {321, -211, {111, {22, {22, {}, Decay::c_RelateWithFirst}}}}}}});
      //result: pi0 gets MC match 521
      const Particle* p = d.getParticle(421);
      const MCParticle* mc = d.getMCParticle(421);
      Decay* d0decay = d.getDecay(421);
      ASSERT_TRUE(setMCTruth(p)) << d0decay->getString();
      EXPECT_EQ(c_MisID , getMCTruthStatus(p, mc)) << d0decay->getString();
    }
  }

  /** pi+ decays into muon, pi+ track is found. */
  TEST_F(MCMatchingTest, DecayInFlightCorrect)
  {
    {
      Decay d(421, {321, { -211, {13}}, {111, {22, 22}}});
      d.reconstruct({421, {321, -211, {111, {22, 22}}}});

      ASSERT_TRUE(setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(d.m_mcparticle->getPDG(), d.m_particle->getRelated<MCParticle>()->getPDG());
      EXPECT_EQ(0, getMCTruthStatus(d.m_particle, d.m_mcparticle)) << d.getString();
    }
  }
  /** pi+ decays into muon, muon track with pi+ hypothesis in reconstruction. */
  TEST_F(MCMatchingTest, DecayInFlight)
  {
    {
      Decay d(-211, {13});
      d.finalize();
      MCParticle* muon = d.getMCParticle(13);
      ASSERT_TRUE(muon != nullptr);
      d.reconstruct({ -211, {}, Decay::c_RelateWith, muon});

      ASSERT_TRUE(setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(d.m_mcparticle->getPDG(), d.m_particle->getRelated<MCParticle>()->getPDG());
      EXPECT_EQ(c_MisID, getMCTruthStatus(d.m_particle, d.m_mcparticle)) << d.getString();
    }
    {
      Decay d(421, {321, { -211, {13}}, {111, {22, 22}}});
      d.finalize();
      MCParticle* muon = d.getMCParticle(13);
      ASSERT_TRUE(muon != nullptr);
      d.reconstruct({421, {321, { -211, {}, Decay::c_RelateWith, muon}, {111, {22, 22}}}});


      ASSERT_TRUE(setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(d.m_mcparticle->getPDG(), d.m_particle->getRelated<MCParticle>()->getPDG());
      EXPECT_EQ(c_MisID, getMCTruthStatus(d.m_particle, d.m_mcparticle)) << d.getString();
    }
  }
  /** we reconstrcut D*+ -> D0 pi+, but it's actually D+ pi0. */
  TEST_F(MCMatchingTest, CorrectFSPsWrongDecay)
  {
    {
      Decay d(-413, {{ -411, {321, -211, -211}}, {111, {22, 22}}});
      d.finalize();

      Decay& pi0 = d[1];
      Decay& pi2 = d[0][2];

      ASSERT_TRUE(d.getDecay(111) ==  &pi0);

      d.reconstruct({ -413, {{ -421, {321, -211, {111, {22, 22}, Decay::c_ReconstructFrom, &pi0}}}, { -211, {}, Decay::c_ReconstructFrom, &pi2}}});


      ASSERT_TRUE(setMCTruth(d.m_particle)) << d.getString();
      EXPECT_EQ(d.m_mcparticle->getPDG(), d.m_particle->getRelated<MCParticle>()->getPDG());
      EXPECT_EQ(c_MisID, getMCTruthStatus(d.m_particle, d.m_mcparticle)) << d.getString();
    }
  }

  //what's going on here?
//[ERROR] isSignal=1, status=0, for |PDG| = 421, idx 4: 413 [ 421 [ -321 [ ] 211 [ ] 111 [ 22 22 ] ] 211 [ ] ]   { module: VariablesToNtuple }

}  // namespace
#endif
