#include <analysis/VariableManager/Variables.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>
#include <framework/datastore/StoreArray.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/utilities/TestHelpers.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>
#include <analysis/utility/ReferenceFrame.h>
#include <mdst/dataobjects/MCParticle.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Variable;

namespace {

  /** test kinematic Variable. */
  TEST(KinematicVariableTest, Variable)
  {

    // Connect gearbox for CMS variables

    Gearbox& gearbox = Gearbox::getInstance();
    gearbox.setBackends({std::string("file:")});
    gearbox.close();
    gearbox.open("geometry/Belle2.xml", false);

    {
      Particle p({ 0.1 , -0.4, 0.8, 1.0 }, 11);
      EXPECT_FLOAT_EQ(0.9, particleP(&p));
      EXPECT_FLOAT_EQ(1.0, particleE(&p));
      EXPECT_FLOAT_EQ(0.1, particlePx(&p));
      EXPECT_FLOAT_EQ(-0.4, particlePy(&p));
      EXPECT_FLOAT_EQ(0.8, particlePz(&p));
      EXPECT_FLOAT_EQ(0.412310562, particlePt(&p));
      EXPECT_FLOAT_EQ(0.8 / 0.9, particleCosTheta(&p));
      EXPECT_FLOAT_EQ(-1.325817664, particlePhi(&p));

      {
        UseReferenceFrame<CMSFrame> dummy;
        EXPECT_FLOAT_EQ(0.68176979, particleP(&p));
        EXPECT_FLOAT_EQ(0.80920333, particleE(&p));
        EXPECT_FLOAT_EQ(0.058562335, particlePx(&p));
        EXPECT_FLOAT_EQ(-0.40000001, particlePy(&p));
        EXPECT_FLOAT_EQ(0.54898131, particlePz(&p));
        EXPECT_FLOAT_EQ(0.40426421, particlePt(&p));
        EXPECT_FLOAT_EQ(0.80522972, particleCosTheta(&p));
        EXPECT_FLOAT_EQ(-1.4254233, particlePhi(&p));
      }

      {
        UseReferenceFrame<RestFrame> dummy(&p);
        EXPECT_ALL_NEAR(particleP(&p), 0.0, 1e-9);
        EXPECT_FLOAT_EQ(0.4358899, particleE(&p));
        EXPECT_ALL_NEAR(0.0, particlePx(&p), 1e-9);
        EXPECT_ALL_NEAR(0.0, particlePy(&p), 1e-9);
        EXPECT_ALL_NEAR(0.0, particlePz(&p), 1e-9);
        EXPECT_ALL_NEAR(0.0, particlePt(&p), 1e-9);
      }

      {
        UseReferenceFrame<LabFrame> dummy;
        EXPECT_FLOAT_EQ(0.9, particleP(&p));
        EXPECT_FLOAT_EQ(1.0, particleE(&p));
        EXPECT_FLOAT_EQ(0.1, particlePx(&p));
        EXPECT_FLOAT_EQ(-0.4, particlePy(&p));
        EXPECT_FLOAT_EQ(0.8, particlePz(&p));
        EXPECT_FLOAT_EQ(0.412310562, particlePt(&p));
        EXPECT_FLOAT_EQ(0.8 / 0.9, particleCosTheta(&p));
        EXPECT_FLOAT_EQ(-1.325817664, particlePhi(&p));
      }

      {
        Particle pinv({ -0.1 , 0.4, -0.8, 1.0 }, 11);
        UseReferenceFrame<RestFrame> dummy(&pinv);
        Particle p2({ 0.0 , 0.0, 0.0, 0.4358899}, 11);
        EXPECT_FLOAT_EQ(0.9, particleP(&p2));
        EXPECT_FLOAT_EQ(1.0, particleE(&p2));
        EXPECT_FLOAT_EQ(0.1, particlePx(&p2));
        EXPECT_FLOAT_EQ(-0.4, particlePy(&p2));
        EXPECT_FLOAT_EQ(0.8, particlePz(&p2));
        EXPECT_FLOAT_EQ(0.412310562, particlePt(&p2));
        EXPECT_FLOAT_EQ(0.8 / 0.9, particleCosTheta(&p2));
        EXPECT_FLOAT_EQ(-1.325817664, particlePhi(&p2));
      }
    }

    {
      Particle p({ 0.0 , 0.0, 0.0, 0.0 }, 11);
      EXPECT_FLOAT_EQ(0.0, particleP(&p));
      EXPECT_FLOAT_EQ(0.0, particleE(&p));
      EXPECT_FLOAT_EQ(0.0, particlePx(&p));
      EXPECT_FLOAT_EQ(0.0, particlePy(&p));
      EXPECT_FLOAT_EQ(0.0, particlePz(&p));
      EXPECT_FLOAT_EQ(0.0, particlePt(&p));
      EXPECT_FLOAT_EQ(1.0, particleCosTheta(&p));
      EXPECT_FLOAT_EQ(0.0, particlePhi(&p));

      UseReferenceFrame<CMSFrame> dummy;
      EXPECT_FLOAT_EQ(0.0, particleP(&p));
      EXPECT_FLOAT_EQ(0.0, particleE(&p));
      EXPECT_FLOAT_EQ(0.0, particlePx(&p));
      EXPECT_FLOAT_EQ(0.0, particlePy(&p));
      EXPECT_FLOAT_EQ(0.0, particlePz(&p));
      EXPECT_FLOAT_EQ(0.0, particlePt(&p));
      EXPECT_FLOAT_EQ(1.0, particleCosTheta(&p));
      EXPECT_FLOAT_EQ(0.0, particlePhi(&p));
    }

  }

  class MetaVariableTest : public ::testing::Test {
  protected:
    /** register Particle array + ParticleExtraInfoMap object. */
    virtual void SetUp()
    {
      Gearbox& gearbox = Gearbox::getInstance();
      gearbox.setBackends({std::string("file:")});
      gearbox.close();
      gearbox.open("geometry/Belle2.xml", false);

      DataStore::Instance().setInitializeActive(true);
      StoreObjPtr<ParticleExtraInfoMap>::registerPersistent();
      StoreArray<Particle>::registerPersistent();
      StoreArray<MCParticle>::registerPersistent();
      DataStore::Instance().setInitializeActive(false);
    }

    /** clear datastore */
    virtual void TearDown()
    {
      DataStore::Instance().reset();
    }
  };

  TEST_F(MetaVariableTest, countDaughters)
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
    const Particle* p = particles.appendNew(momentum, 411, Particle::c_Unflavored, daughterIndices);

    const Manager::Var* var = Manager::Instance().getVariable("countDaughters(charge > 0)");
    ASSERT_NE(var, nullptr);
    EXPECT_DOUBLE_EQ(var->function(p), 3.0);

    var = Manager::Instance().getVariable("countDaughters(abs(charge) > 0)");
    ASSERT_NE(var, nullptr);
    EXPECT_DOUBLE_EQ(var->function(p), 6.0);

  }

  TEST_F(MetaVariableTest, useRestFrame)
  {
    Particle p({ 0.1 , -0.4, 0.8, 1.0 }, 11);
    p.setVertex(TVector3(1.0, 2.0, 2.0));

    const Manager::Var* var = Manager::Instance().getVariable("p");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.9);

    var = Manager::Instance().getVariable("E");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 1.0);

    var = Manager::Instance().getVariable("distance");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 3.0);

    var = Manager::Instance().getVariable("useRestFrame(p)");
    ASSERT_NE(var, nullptr);
    EXPECT_ALL_NEAR(var->function(&p), 0.0, 1e-9);

    var = Manager::Instance().getVariable("useRestFrame(E)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.4358899);

    var = Manager::Instance().getVariable("useRestFrame(distance)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.0);
  }

  TEST_F(MetaVariableTest, useLabFrame)
  {
    Particle p({ 0.1 , -0.4, 0.8, 1.0 }, 11);
    p.setVertex(TVector3(1.0, 2.0, 2.0));

    const Manager::Var* var = Manager::Instance().getVariable("p");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.9);

    var = Manager::Instance().getVariable("E");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 1.0);

    var = Manager::Instance().getVariable("distance");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 3.0);

    var = Manager::Instance().getVariable("useLabFrame(p)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.9);

    var = Manager::Instance().getVariable("useLabFrame(E)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 1.0);

    var = Manager::Instance().getVariable("useLabFrame(distance)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 3.0);
  }

  TEST_F(MetaVariableTest, useCMSFrame)
  {
    Particle p({ 0.1 , -0.4, 0.8, 1.0 }, 11);
    p.setVertex(TVector3(1.0, 2.0, 2.0));

    const Manager::Var* var = Manager::Instance().getVariable("p");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.9);

    var = Manager::Instance().getVariable("E");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 1.0);

    var = Manager::Instance().getVariable("distance");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 3.0);

    var = Manager::Instance().getVariable("useCMSFrame(p)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.68176979);

    var = Manager::Instance().getVariable("useCMSFrame(E)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.80920333);

    var = Manager::Instance().getVariable("useCMSFrame(distance)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 3.1853244);
  }

  TEST_F(MetaVariableTest, extraInfo)
  {
    Particle p({ 0.1 , -0.4, 0.8, 1.0 }, 11);
    p.addExtraInfo("pi", 3.14);

    const Manager::Var* var = Manager::Instance().getVariable("extraInfo(pi)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 3.14);
  }

  TEST_F(MetaVariableTest, formula)
  {
    Particle p({ 0.1 , -0.4, 0.8, 2.0 }, 11);

    const Manager::Var* var = Manager::Instance().getVariable("formula(px + py)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), -0.3);

    var = Manager::Instance().getVariable("formula(px - py)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.5);

    var = Manager::Instance().getVariable("formula(px * py)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), -0.04);

    var = Manager::Instance().getVariable("formula(py / px)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), -4.0);

    var = Manager::Instance().getVariable("formula(px ^ E)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.01);

    var = Manager::Instance().getVariable("formula(px * py + pz)");
    ASSERT_NE(var, nullptr);
    EXPECT_ALL_NEAR(var->function(&p), 0.76, 1e-6);

    var = Manager::Instance().getVariable("formula(pz + px * py)");
    ASSERT_NE(var, nullptr);
    EXPECT_ALL_NEAR(var->function(&p), 0.76, 1e-6);
  }

  TEST_F(MetaVariableTest, passesCut)
  {
    Particle p({ 0.1 , -0.4, 0.8, 2.0 }, 11);
    Particle p2({ 0.1 , -0.4, 0.8, 4.0 }, 11);

    const Manager::Var* var = Manager::Instance().getVariable("passesCut(E < 3)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 1);
    EXPECT_FLOAT_EQ(var->function(&p2), 0);
    EXPECT_FLOAT_EQ(var->function(nullptr), -999);

  }

  TEST_F(MetaVariableTest, NumberOfMCParticlesInEvent)
  {
    Particle p({ 0.1 , -0.4, 0.8, 2.0 }, 11);
    Particle p2({ 0.1 , -0.4, 0.8, 4.0 }, 11);

    StoreArray<MCParticle> mcParticles;
    auto* mcParticle = mcParticles.appendNew();
    mcParticle->setPDG(11);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    mcParticle = mcParticles.appendNew();
    mcParticle->setPDG(22);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    mcParticle = mcParticles.appendNew();
    mcParticle->setPDG(-11);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    mcParticle = mcParticles.appendNew();
    mcParticle->setPDG(11);


    const Manager::Var* var = Manager::Instance().getVariable("NumberOfMCParticlesInEvent(11)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(nullptr), 2);

  }

  TEST_F(MetaVariableTest, daughterInvariantMass)
  {
    TLorentzVector momentum;
    const int nDaughters = 6;
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    for (int i = 0; i < nDaughters; i++) {
      Particle d(TLorentzVector(2, 2, 2, 4.0), (i % 2) ? 211 : -211);
      momentum += d.get4Vector();
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices.push_back(newDaughters->getArrayIndex());
    }
    const Particle* p = particles.appendNew(momentum, 411, Particle::c_Unflavored, daughterIndices);

    const Manager::Var* var = Manager::Instance().getVariable("daughterInvariantMass(6)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), -999.0);

    var = Manager::Instance().getVariable("daughterInvariantMass(0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 2.0);

    var = Manager::Instance().getVariable("daughterInvariantMass(0, 1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 4.0);

    var = Manager::Instance().getVariable("daughterInvariantMass(0, 1, 2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 6.0);

  }

  TEST_F(MetaVariableTest, daughter)
  {
    TLorentzVector momentum;
    const int nDaughters = 6;
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    for (int i = 0; i < nDaughters; i++) {
      Particle d(TLorentzVector(i * 1.0, 1, 1, 1), (i % 2) ? 211 : -211);
      momentum += d.get4Vector();
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices.push_back(newDaughters->getArrayIndex());
    }
    const Particle* p = particles.appendNew(momentum, 411, Particle::c_Unflavored, daughterIndices);

    const Manager::Var* var = Manager::Instance().getVariable("daughter(6, px)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), -999.0);

    var = Manager::Instance().getVariable("daughter(0, px)");
    ASSERT_NE(var, nullptr);
    EXPECT_ALL_NEAR(var->function(p), 0.0, 1e-6);

    var = Manager::Instance().getVariable("daughter(1, px)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 1.0);

    var = Manager::Instance().getVariable("daughter(2, px)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 2.0);

  }

  TEST_F(MetaVariableTest, daughterProductOf)
  {
    TLorentzVector momentum;
    const int nDaughters = 4;
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    for (int i = 0; i < nDaughters; i++) {
      Particle d(TLorentzVector(1, 1, 1, i * 1.0 + 1.0), (i % 2) ? 211 : -211);
      momentum += d.get4Vector();
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices.push_back(newDaughters->getArrayIndex());
    }
    const Particle* p = particles.appendNew(momentum, 411, Particle::c_Unflavored, daughterIndices);

    const Manager::Var* var = Manager::Instance().getVariable("daughterProductOf(E)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 24.0);

  }

  TEST_F(MetaVariableTest, daughterSumOf)
  {
    TLorentzVector momentum;
    const int nDaughters = 4;
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    for (int i = 0; i < nDaughters; i++) {
      Particle d(TLorentzVector(1, 1, 1, i * 1.0 + 1.0), (i % 2) ? 211 : -211);
      momentum += d.get4Vector();
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices.push_back(newDaughters->getArrayIndex());
    }
    const Particle* p = particles.appendNew(momentum, 411, Particle::c_Unflavored, daughterIndices);

    const Manager::Var* var = Manager::Instance().getVariable("daughterSumOf(E)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 10.0);

  }

  TEST_F(MetaVariableTest, constant)
  {

    const Manager::Var* var = Manager::Instance().getVariable("constant(1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(nullptr), 1.0);

    var = Manager::Instance().getVariable("constant(0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(nullptr), 0.0);

  }

  TEST_F(MetaVariableTest, abs)
  {
    Particle p({ 0.1 , -0.4, 0.8, 2.0 }, 11);
    Particle p2({ -0.1 , -0.4, 0.8, 4.0 }, -11);

    const Manager::Var* var = Manager::Instance().getVariable("abs(px)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.1);
    EXPECT_FLOAT_EQ(var->function(&p2), 0.1);

  }

  TEST_F(MetaVariableTest, sin)
  {
    Particle p({ 3.14159265359 / 2.0 , -0.4, 0.8, 1.0}, 11);
    Particle p2({ 0.0 , -0.4, 0.8, 1.0 }, -11);

    const Manager::Var* var = Manager::Instance().getVariable("sin(px)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 1.0);
    EXPECT_ALL_NEAR(var->function(&p2), 0.0, 1e-6);

  }

  TEST_F(MetaVariableTest, cos)
  {
    Particle p({ 3.14159265359 / 2.0 , -0.4, 0.8, 1.0}, 11);
    Particle p2({ 0.0 , -0.4, 0.8, 1.0 }, -11);

    const Manager::Var* var = Manager::Instance().getVariable("cos(px)");
    ASSERT_NE(var, nullptr);
    EXPECT_ALL_NEAR(var->function(&p), 0.0, 1e-6);
    EXPECT_FLOAT_EQ(var->function(&p2), 1.0);

  }

}
