#include <analysis/VariableManager/Variables.h>
#include <analysis/VariableManager/PIDVariables.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <analysis/utility/ReferenceFrame.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/utilities/TestHelpers.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <analysis/utility/ReferenceFrame.h>
#include <analysis/dataobjects/ParticleList.h>

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

    {
      DataStore::Instance().setInitializeActive(true);
      StoreArray<Particle>::registerPersistent();
      DataStore::Instance().setInitializeActive(false);
      StoreArray<Particle> particles;
      PCmsLabTransform T;
      TLorentzVector vec0 = {0.0, 0.0, 0.0, 10.5794};
      TLorentzVector vec1 = {0.0, +0.332174566, 0.0, 5.2897};
      TLorentzVector vec2 = {0.0, -0.332174566, 0.0, 5.2897};
      Particle* p0 = particles.appendNew(Particle(T.rotateCmsToLab() * vec0, 22));
      Particle* p1 = particles.appendNew(Particle(T.rotateCmsToLab() * vec1, 22, Particle::c_Unflavored, Particle::c_Undefined, 1));
      Particle* p2 = particles.appendNew(Particle(T.rotateCmsToLab() * vec2, 22, Particle::c_Unflavored, Particle::c_Undefined, 2));

      p0->appendDaughter(p1->getArrayIndex());
      p0->appendDaughter(p2->getArrayIndex());

      EXPECT_ALL_NEAR(missingMass(p0), 7.5625e-6, 1e-7);
      EXPECT_ALL_NEAR(missingMomentum(p0), 0.0, 2e-7);
    }


  }

  TEST(VertexVariableTest, Variable)
  {

    // Connect gearbox for CMS variables

    Gearbox& gearbox = Gearbox::getInstance();
    gearbox.setBackends({std::string("file:")});
    gearbox.close();
    gearbox.open("geometry/Belle2.xml", false);

    Particle p({ 0.1 , -0.4, 0.8, 1.0 }, 11);
    p.setPValue(0.5);
    p.setVertex(TVector3(1.0, 2.0, 2.0));

    EXPECT_FLOAT_EQ(1.0, particleDX(&p));
    EXPECT_FLOAT_EQ(2.0, particleDY(&p));
    EXPECT_FLOAT_EQ(2.0, particleDZ(&p));
    EXPECT_FLOAT_EQ(std::sqrt(5.0), particleDRho(&p));
    EXPECT_FLOAT_EQ(3.0, particleDistance(&p));
    EXPECT_FLOAT_EQ(0.5, particlePvalue(&p));

    {
      UseReferenceFrame<CMSFrame> dummy;
      EXPECT_FLOAT_EQ(1.0261739, particleDX(&p));
      EXPECT_FLOAT_EQ(2.0, particleDY(&p));
      EXPECT_FLOAT_EQ(2.256825, particleDZ(&p));
      EXPECT_FLOAT_EQ(std::sqrt(2.0 * 2.0 + 1.0261739 * 1.0261739), particleDRho(&p));
      EXPECT_FLOAT_EQ(3.1853244, particleDistance(&p));
      EXPECT_FLOAT_EQ(0.5, particlePvalue(&p));
    }

    {
      Particle p2({ 0.1 , -0.4, 0.8, 1.0 }, 11);
      p2.setPValue(0.5);
      p2.setVertex(TVector3(1.0, 2.0, 2.0));

      UseReferenceFrame<RestFrame> dummy(&p2);
      EXPECT_FLOAT_EQ(0.0, particleDX(&p));
      EXPECT_FLOAT_EQ(0.0, particleDY(&p));
      EXPECT_FLOAT_EQ(0.0, particleDZ(&p));
      EXPECT_FLOAT_EQ(0.0, particleDRho(&p));
      EXPECT_FLOAT_EQ(0.0, particleDistance(&p));
      EXPECT_FLOAT_EQ(0.5, particlePvalue(&p));
    }

    /* Test with a distance between mother and daughter vertex. One
     * has to calculate the result by hand to test the code....

    {
      Particle p2({ 0.0 , 1.0, 0.0, 1.0 }, 11);
      p2.setPValue(0.5);
      p2.setVertex(TVector3(1.0, 0.0, 2.0));

      UseReferenceFrame<RestFrame> dummy(&p2);
      EXPECT_FLOAT_EQ(0.0, particleDX(&p));
      EXPECT_FLOAT_EQ(2.0, particleDY(&p));
      EXPECT_FLOAT_EQ(0.0, particleDZ(&p));
      EXPECT_FLOAT_EQ(2.0, particleDRho(&p));
      EXPECT_FLOAT_EQ(2.0, particleDistance(&p));
      EXPECT_FLOAT_EQ(0.5, particlePvalue(&p));
    }
         */

  }

  class MetaVariableTest : public ::testing::Test {
  protected:
    /** register Particle array + ParticleExtraInfoMap object. */
    virtual void SetUp()
    {
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
    Gearbox& gearbox = Gearbox::getInstance();
    gearbox.setBackends({std::string("file:")});
    gearbox.close();
    gearbox.open("geometry/Belle2.xml", false);

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
    Gearbox& gearbox = Gearbox::getInstance();
    gearbox.setBackends({std::string("file:")});
    gearbox.close();
    gearbox.open("geometry/Belle2.xml", false);

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

  TEST_F(MetaVariableTest, NBDeltaIfMissing)
  {
    DataStore::Instance().setInitializeActive(true);
    StoreArray<PIDLikelihood>::registerPersistent();
    StoreArray<PIDLikelihood> likelihood;
    StoreArray<Particle> particles;
    particles.registerRelationTo(likelihood);
    DataStore::Instance().setInitializeActive(false);

    auto* l1 = likelihood.appendNew();
    l1->setLogLikelihood(Const::TOP, Const::electron, 0.5);
    l1->setLogLikelihood(Const::ARICH, Const::electron, 0.5);
    l1->setLogLikelihood(Const::ECL, Const::electron, 0.5);
    l1->setLogLikelihood(Const::TOP, Const::pion, 0.5);
    l1->setLogLikelihood(Const::ARICH, Const::pion, 0.5);
    l1->setLogLikelihood(Const::ECL, Const::pion, 0.5);

    auto* l2 = likelihood.appendNew();
    l2->setLogLikelihood(Const::TOP, Const::electron, 0.5);
    l2->setLogLikelihood(Const::ECL, Const::electron, 0.5);
    l2->setLogLikelihood(Const::TOP, Const::pion, 0.5);
    l2->setLogLikelihood(Const::ECL, Const::pion, 0.5);

    auto* l3 = likelihood.appendNew();
    l3->setLogLikelihood(Const::TOP, Const::electron, 0.5);
    l3->setLogLikelihood(Const::TOP, Const::pion, 0.5);

    auto* l4 = likelihood.appendNew();
    l4->setLogLikelihood(Const::ECL, Const::electron, 0.5);
    l4->setLogLikelihood(Const::ECL, Const::pion, 0.5);

    auto* p1 = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p1->addRelationTo(l1);

    auto* p2 = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p2->addRelationTo(l2);

    auto* p3 = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p3->addRelationTo(l3);

    auto* p4 = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p4->addRelationTo(l4);

    EXPECT_B2FATAL(Manager::Instance().getVariable("NBDeltaIfMissing(TOP, eid_TOP, 1)"));
    EXPECT_B2FATAL(Manager::Instance().getVariable("NBDeltaIfMissing(ECL, eid_ECL)"));

    const Manager::Var* var = Manager::Instance().getVariable("NBDeltaIfMissing(TOP, eid_TOP)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p1), 0.5);
    EXPECT_FLOAT_EQ(var->function(p2), 0.5);
    EXPECT_FLOAT_EQ(var->function(p3), 0.5);
    EXPECT_FLOAT_EQ(var->function(p4), -999.0);

    var = Manager::Instance().getVariable("NBDeltaIfMissing(ARICH, eid_ARICH)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p1), 0.5);
    EXPECT_FLOAT_EQ(var->function(p2), -999.0);
    EXPECT_FLOAT_EQ(var->function(p3), -999.0);
    EXPECT_FLOAT_EQ(var->function(p4), -999.0);

  }

  TEST_F(MetaVariableTest, matchedMC)
  {
    DataStore::Instance().setInitializeActive(true);
    StoreArray<MCParticle> mcParticles;
    StoreArray<Particle> particles;
    particles.registerRelationTo(mcParticles);
    DataStore::Instance().setInitializeActive(false);

    auto* mcParticle = mcParticles.appendNew();
    mcParticle->setPDG(11);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    auto* p1 = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p1->addRelationTo(mcParticle);

    mcParticle = mcParticles.appendNew();
    mcParticle->setPDG(-11);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    auto* p2 = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p2->addRelationTo(mcParticle);

    mcParticle = mcParticles.appendNew();
    mcParticle->setPDG(22);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    auto* p3 = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p3->addRelationTo(mcParticle);

    const Manager::Var* var = Manager::Instance().getVariable("matchedMC(charge)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p1), -1);
    EXPECT_FLOAT_EQ(var->function(p2), 1);
    EXPECT_FLOAT_EQ(var->function(p3), 0);

  }

  TEST_F(MetaVariableTest, veto)
  {
    StoreArray<Particle> particles;
    DataStore::EStoreFlags flags = DataStore::c_DontWriteOut;

    const Particle* p = particles.appendNew(Particle({0.8 , 0.8 , 1.131370849898476039041351 , 1.6}, 22,
                                                     Particle::c_Unflavored, Particle::c_Undefined, 1));

    StoreObjPtr<ParticleList> outputList("pList1");
    DataStore::Instance().setInitializeActive(true);
    outputList.registerInDataStore(flags);
    DataStore::Instance().setInitializeActive(false);
    outputList.create();
    outputList->initialize(22, "pList1");

    particles.appendNew(Particle({0.5 , 0.4953406774856531014212777 , 0.5609256753154148484773173 , 0.9}, 22,
                                 Particle::c_Unflavored, Particle::c_Undefined, 2));         //m=0.135
    particles.appendNew(Particle({0.5 , 0.2 , 0.72111 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 3));    //m=0.3582
    particles.appendNew(Particle({0.4 , 0.2 , 0.78102 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 4));    //m=0.3908
    particles.appendNew(Particle({0.5 , 0.4 , 0.89443 , 1.1}, 22, Particle::c_Unflavored, Particle::c_Undefined, 5));    //m=0.2369
    particles.appendNew(Particle({0.3 , 0.3 , 0.42426 , 0.6}, 22, Particle::c_Unflavored, Particle::c_Undefined, 6));    //m=0.0036

    outputList->addParticle(1, 22, Particle::c_Unflavored);
    outputList->addParticle(2, 22, Particle::c_Unflavored);
    outputList->addParticle(3, 22, Particle::c_Unflavored);
    outputList->addParticle(4, 22, Particle::c_Unflavored);
    outputList->addParticle(5, 22, Particle::c_Unflavored);

    StoreObjPtr<ParticleList> outputList2("pList2");
    DataStore::Instance().setInitializeActive(true);
    outputList2.registerInDataStore(flags);
    DataStore::Instance().setInitializeActive(false);
    outputList2.create();
    outputList2->initialize(22, "pList2");

    particles.appendNew(Particle({0.5 , -0.4 , 0.63246 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 7));    //m=1.1353
    particles.appendNew(Particle({0.5 , 0.2 , 0.72111 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 8));     //m=0.3582
    particles.appendNew(Particle({0.4 , 0.2 , 0.78102 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 9));     //m=0.3908
    particles.appendNew(Particle({0.5 , 0.4 , 0.89443 , 1.1}, 22, Particle::c_Unflavored, Particle::c_Undefined, 10));    //m=0.2369
    particles.appendNew(Particle({0.3 , 0.3 , 0.42426 , 0.6}, 22, Particle::c_Unflavored, Particle::c_Undefined, 11));    //m=0.0036

    outputList2->addParticle(6, 22, Particle::c_Unflavored);
    outputList2->addParticle(7, 22, Particle::c_Unflavored);
    outputList2->addParticle(8, 22, Particle::c_Unflavored);
    outputList2->addParticle(9, 22, Particle::c_Unflavored);
    outputList2->addParticle(10, 22, Particle::c_Unflavored);

    const Manager::Var* var = Manager::Instance().getVariable("veto(pList1, 0.130 < M < 0.140)");
    ASSERT_NE(var, nullptr);
    EXPECT_DOUBLE_EQ(var->function(p), 1);

    var = Manager::Instance().getVariable("veto(pList2, 0.130 < M < 0.140)");
    ASSERT_NE(var, nullptr);
    EXPECT_DOUBLE_EQ(var->function(p), 0);

  }

  class PIDVariableTest : public ::testing::Test {
  protected:
    /** register Particle array + ParticleExtraInfoMap object. */
    virtual void SetUp()
    {
      DataStore::Instance().setInitializeActive(true);
      StoreObjPtr<ParticleExtraInfoMap>::registerPersistent();
      StoreArray<Particle>::registerPersistent();
      StoreArray<MCParticle>::registerPersistent();
      StoreArray<PIDLikelihood>::registerPersistent();
      StoreArray<PIDLikelihood> likelihood;
      StoreArray<Particle> particles;
      particles.registerRelationTo(likelihood);
      DataStore::Instance().setInitializeActive(false);
    }

    /** clear datastore */
    virtual void TearDown()
    {
      DataStore::Instance().reset();
    }
  };

  TEST_F(PIDVariableTest, LogLikelihood)
  {
    StoreArray<PIDLikelihood> likelihood;
    StoreArray<Particle> particles;

    auto* l1 = likelihood.appendNew();
    l1->setLogLikelihood(Const::TOP, Const::electron, 0.18);
    l1->setLogLikelihood(Const::ARICH, Const::electron, 0.16);
    l1->setLogLikelihood(Const::ECL, Const::electron, 0.14);
    l1->setLogLikelihood(Const::CDC, Const::electron, 0.12);
    l1->setLogLikelihood(Const::SVD, Const::electron, 0.1);

    l1->setLogLikelihood(Const::TOP, Const::pion, 0.2);
    l1->setLogLikelihood(Const::ARICH, Const::pion, 0.22);
    l1->setLogLikelihood(Const::ECL, Const::pion, 0.24);
    l1->setLogLikelihood(Const::CDC, Const::pion, 0.26);
    l1->setLogLikelihood(Const::SVD, Const::pion, 0.28);

    l1->setLogLikelihood(Const::TOP, Const::kaon, 0.3);
    l1->setLogLikelihood(Const::ARICH, Const::kaon, 0.32);
    l1->setLogLikelihood(Const::ECL, Const::kaon, 0.34);
    l1->setLogLikelihood(Const::CDC, Const::kaon, 0.36);
    l1->setLogLikelihood(Const::SVD, Const::kaon, 0.38);

    l1->setLogLikelihood(Const::TOP, Const::proton, 0.4);
    l1->setLogLikelihood(Const::ARICH, Const::proton, 0.42);
    l1->setLogLikelihood(Const::ECL, Const::proton, 0.44);
    l1->setLogLikelihood(Const::CDC, Const::proton, 0.46);
    l1->setLogLikelihood(Const::SVD, Const::proton, 0.48);

    l1->setLogLikelihood(Const::TOP, Const::muon, 0.5);
    l1->setLogLikelihood(Const::ARICH, Const::muon, 0.52);
    l1->setLogLikelihood(Const::ECL, Const::muon, 0.54);
    l1->setLogLikelihood(Const::CDC, Const::muon, 0.56);
    l1->setLogLikelihood(Const::SVD, Const::muon, 0.58);

    auto* electron = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    electron->addRelationTo(l1);
    auto* pion = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 211);
    pion->addRelationTo(l1);
    auto* muon = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 13);
    muon->addRelationTo(l1);
    auto* kaon = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 321);
    kaon->addRelationTo(l1);
    auto* proton = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 2212);
    proton->addRelationTo(l1);

    EXPECT_FLOAT_EQ(particleDeltaLogLElectron(electron),  0.7 - 0.7);
    EXPECT_FLOAT_EQ(particleDeltaLogLPion(electron),  0.7 - 1.2);
    EXPECT_FLOAT_EQ(particleDeltaLogLKaon(electron),  0.7 - 1.7);
    EXPECT_FLOAT_EQ(particleDeltaLogLProton(electron),  0.7 - 2.2);
    EXPECT_FLOAT_EQ(particleDeltaLogLMuon(electron),  0.7 - 2.7);

    EXPECT_FLOAT_EQ(particleDeltaLogLElectron(pion),  1.2 - 0.7);
    EXPECT_FLOAT_EQ(particleDeltaLogLPion(pion),  1.2 - 1.2);
    EXPECT_FLOAT_EQ(particleDeltaLogLKaon(pion),  1.2 - 1.7);
    EXPECT_FLOAT_EQ(particleDeltaLogLProton(pion),  1.2 - 2.2);
    EXPECT_FLOAT_EQ(particleDeltaLogLMuon(pion),  1.2 - 2.7);

    EXPECT_FLOAT_EQ(particleDeltaLogLElectron(kaon),  1.7 - 0.7);
    EXPECT_FLOAT_EQ(particleDeltaLogLPion(kaon),  1.7 - 1.2);
    EXPECT_FLOAT_EQ(particleDeltaLogLKaon(kaon),  1.7 - 1.7);
    EXPECT_FLOAT_EQ(particleDeltaLogLProton(kaon),  1.7 - 2.2);
    EXPECT_FLOAT_EQ(particleDeltaLogLMuon(kaon),  1.7 - 2.7);

    EXPECT_FLOAT_EQ(particleDeltaLogLElectron(proton), 2.2 - 0.7);
    EXPECT_FLOAT_EQ(particleDeltaLogLPion(proton), 2.2 - 1.2);
    EXPECT_FLOAT_EQ(particleDeltaLogLKaon(proton), 2.2 - 1.7);
    EXPECT_FLOAT_EQ(particleDeltaLogLProton(proton), 2.2 - 2.2);
    EXPECT_ALL_NEAR(particleDeltaLogLMuon(proton),  2.2 - 2.7, 1e-6);

    EXPECT_FLOAT_EQ(particleDeltaLogLElectron(muon),  2.7 - 0.7);
    EXPECT_FLOAT_EQ(particleDeltaLogLPion(muon),  2.7 - 1.2);
    EXPECT_FLOAT_EQ(particleDeltaLogLKaon(muon),  2.7 - 1.7);
    EXPECT_ALL_NEAR(particleDeltaLogLProton(muon),  2.7 - 2.2, 1e-6);
    EXPECT_FLOAT_EQ(particleDeltaLogLMuon(muon),  2.7 - 2.7);

    EXPECT_FLOAT_EQ(particleElectronId(electron), 1.0 / (1.0 + std::exp(1.2 - 0.7)));
    EXPECT_FLOAT_EQ(particleMuonId(muon), 1.0 / (1.0 + std::exp(1.2 - 2.7)));
    EXPECT_FLOAT_EQ(particlePionId(pion), 1.0 / (1.0 + std::exp(1.7 - 1.2)));
    EXPECT_FLOAT_EQ(particleKaonId(kaon), 1.0 / (1.0 + std::exp(1.2 - 1.7)));
    EXPECT_FLOAT_EQ(particleProtonId(proton), 1.0 / (1.0 + std::exp(1.2 - 2.2)));
    EXPECT_FLOAT_EQ(particlePionvsElectronId(pion), 1.0 / (1.0 + std::exp(0.7 - 1.2)));

    EXPECT_FLOAT_EQ(particleElectrondEdxId(electron), 1.0 / (1.0 + std::exp(0.54 - 0.22)));
    EXPECT_FLOAT_EQ(particleMuondEdxId(muon), 1.0 / (1.0 + std::exp(0.54 - 1.14)));
    EXPECT_FLOAT_EQ(particlePiondEdxId(pion), 1.0 / (1.0 + std::exp(0.74 - 0.54)));
    EXPECT_FLOAT_EQ(particleKaondEdxId(kaon), 1.0 / (1.0 + std::exp(0.54 - 0.74)));
    EXPECT_FLOAT_EQ(particleProtondEdxId(proton), 1.0 / (1.0 + std::exp(0.54 - 0.94)));
    EXPECT_FLOAT_EQ(particlePionvsElectrondEdxId(pion), 1.0 / (1.0 + std::exp(0.22 - 0.54)));

    EXPECT_FLOAT_EQ(particleElectronTOPId(electron), 1.0 / (1.0 + std::exp(0.2 - 0.18)));
    EXPECT_FLOAT_EQ(particleMuonTOPId(muon), 1.0 / (1.0 + std::exp(0.2 - 0.5)));
    EXPECT_FLOAT_EQ(particlePionTOPId(pion), 1.0 / (1.0 + std::exp(0.3 - 0.2)));
    EXPECT_FLOAT_EQ(particleKaonTOPId(kaon), 1.0 / (1.0 + std::exp(0.2 - 0.3)));
    EXPECT_FLOAT_EQ(particleProtonTOPId(proton), 1.0 / (1.0 + std::exp(0.2 - 0.4)));

    EXPECT_FLOAT_EQ(particleElectronARICHId(electron), 1.0 / (1.0 + std::exp(0.22 - 0.16)));
    EXPECT_FLOAT_EQ(particleMuonARICHId(muon), 1.0 / (1.0 + std::exp(0.22 - 0.52)));
    EXPECT_FLOAT_EQ(particlePionARICHId(pion), 1.0 / (1.0 + std::exp(0.32 - 0.22)));
    EXPECT_FLOAT_EQ(particleKaonARICHId(kaon), 1.0 / (1.0 + std::exp(0.22 - 0.32)));
    EXPECT_FLOAT_EQ(particleProtonARICHId(proton), 1.0 / (1.0 + std::exp(0.22 - 0.42)));

    EXPECT_FLOAT_EQ(particleElectronECLId(electron), 1.0 / (1.0 + std::exp(0.24 - 0.14)));

  }

  TEST_F(PIDVariableTest, MissingLikelihood)
  {
    StoreArray<PIDLikelihood> likelihood;
    StoreArray<Particle> particles;

    auto* l1 = likelihood.appendNew();
    l1->setLogLikelihood(Const::TOP, Const::electron, 0.18);
    l1->setLogLikelihood(Const::ECL, Const::electron, 0.14);
    auto* electron = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    electron->addRelationTo(l1);

    auto* l2 = likelihood.appendNew();
    l2->setLogLikelihood(Const::TOP, Const::pion, 0.2);
    l2->setLogLikelihood(Const::ARICH, Const::pion, 0.22);
    l2->setLogLikelihood(Const::ECL, Const::pion, 0.24);
    l2->setLogLikelihood(Const::CDC, Const::pion, 0.26);
    l2->setLogLikelihood(Const::SVD, Const::pion, 0.28);
    auto* pion = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 211);
    pion->addRelationTo(l2);

    auto* l3 = likelihood.appendNew();
    l3->setLogLikelihood(Const::TOP, Const::kaon, 0.3);
    l3->setLogLikelihood(Const::ARICH, Const::kaon, 0.32);
    auto* kaon = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 321);
    kaon->addRelationTo(l3);

    auto* l4 = likelihood.appendNew();
    l4->setLogLikelihood(Const::ARICH, Const::proton, 0.42);
    l4->setLogLikelihood(Const::ECL, Const::proton, 0.44);
    l4->setLogLikelihood(Const::CDC, Const::proton, 0.46);
    l4->setLogLikelihood(Const::SVD, Const::proton, 0.48);
    auto* proton = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 2212);
    proton->addRelationTo(l4);

    EXPECT_FLOAT_EQ(particleMissingTOPId(electron), 0.0);
    EXPECT_FLOAT_EQ(particleMissingTOPId(pion), 0.0);
    EXPECT_FLOAT_EQ(particleMissingTOPId(kaon), 0.0);
    EXPECT_FLOAT_EQ(particleMissingTOPId(proton), 1.0);

    EXPECT_FLOAT_EQ(particleMissingARICHId(electron), 1.0);
    EXPECT_FLOAT_EQ(particleMissingARICHId(pion), 0.0);
    EXPECT_FLOAT_EQ(particleMissingARICHId(kaon), 0.0);
    EXPECT_FLOAT_EQ(particleMissingARICHId(proton), 0.0);

    EXPECT_FLOAT_EQ(particleMissingECLId(electron), 0.0);
    EXPECT_FLOAT_EQ(particleMissingECLId(pion), 0.0);
    EXPECT_FLOAT_EQ(particleMissingECLId(kaon), 1.0);
    EXPECT_FLOAT_EQ(particleMissingECLId(proton), 0.0);

  }

}
