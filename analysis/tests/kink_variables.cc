/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <gtest/gtest.h>

#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>

#include <analysis/variables/KinkVariables.h>

#include <mdst/dataobjects/Kink.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <framework/datastore/StoreArray.h>

#include <TRandom3.h>
#include <Math/Cartesian2D.h>
#include <Math/Vector3D.h>

using namespace Belle2;
using namespace Belle2::Variable;
using namespace ROOT::Math;

namespace {
  class KinkVariablesTest : public ::testing::Test {
  protected:
    /** register Particle array + ParticleExtraInfoMap object. */
    void SetUp() override
    {
      DataStore::Instance().setInitializeActive(true);
      StoreArray<Kink> myKinks;
      StoreArray<TrackFitResult> myTFRs;
      StoreArray<Track> myTracks;
      StoreArray<Particle> myParticles;
      StoreArray<MCParticle> myMCParticles;
      StoreObjPtr<ParticleExtraInfoMap>().registerInDataStore();
      myKinks.registerInDataStore();
      myTFRs.registerInDataStore();
      myTracks.registerInDataStore();
      myParticles.registerInDataStore();
      myMCParticles.registerInDataStore();
      myParticles.registerRelationTo(myTFRs);
      myParticles.registerRelationTo(myMCParticles);
      myTracks.registerRelationTo(myMCParticles);
      DataStore::Instance().setInitializeActive(false);

      TRandom3 generator;

      const float pValueMotherTrack = 0.5;
      const float pValueDaughterTrack = 0.6;
      const float ndfMotherTrack = 15;
      const float ndfDaughterTrack = 5;
      const double bField = Belle2::BFieldManager::getFieldInTesla(XYZVector(0, 0, 0)).Z();
      const int charge = 1;
      TMatrixDSym cov6(6);
      cov6.Zero();
      cov6(0, 0) = 0.05;
      cov6(1, 1) = 0.2;
      cov6(2, 2) = 0.4;
      cov6(3, 3) = 0.01;
      cov6(4, 4) = 0.04;
      cov6(5, 5) = 0.00875;

      // Generate a random put orthogonal pair of vectors in the r-phi plane
      Cartesian2D d(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
      Cartesian2D pt(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
      d.SetXY(d.X(), -(d.X()*pt.X()) / pt.Y());

      // Add a random z component
      XYZVector position(d.X(), d.Y(), generator.Uniform(-1, 1));
      XYZVector momentum(pt.X(), pt.Y(), generator.Uniform(-1, 1));

      auto CDCValue = static_cast<unsigned long long int>(0x300000000000000);

      TrackFitResult* myMotherTrackFitResult = myTFRs.appendNew(position, momentum, cov6, charge, Const::muon, pValueMotherTrack, bField,
                                                                CDCValue, 16777215, ndfMotherTrack);
      Track myMotherTrack;
      myMotherTrack.setTrackFitResultIndex(Const::muon, 0);
      Track* muonTrack = myTracks.appendNew(myMotherTrack);

      TrackFitResult* myDaughterTrackFitResult = myTFRs.appendNew(position, momentum, cov6, charge, Const::electron, pValueDaughterTrack,
                                                                  bField, CDCValue, 16777215, ndfDaughterTrack);
      Track myDaughterTrack;
      myDaughterTrack.setTrackFitResultIndex(Const::electron, 1);
      Track* electronTrack = myTracks.appendNew(myDaughterTrack);

      Particle* myMuon = myParticles.appendNew(muonTrack, Const::muon);
      Particle* myKinkMuon = myParticles.appendNew(myMuon->get4Vector(), Const::muon.getPDGCode(), Particle::c_Flavored,
                                                   Particle::EParticleSourceObject::c_Kink, 0);
      myKinkMuon->addRelationTo(myMotherTrackFitResult);
      myKinkMuon->writeExtraInfo("kinkDaughterPDGCode", Const::electron.getPDGCode());

      myKinks.appendNew(std::make_pair(muonTrack, std::make_pair(myMotherTrackFitResult, myMotherTrackFitResult)),
                        std::make_pair(electronTrack, myDaughterTrackFitResult), 1, 1, 2, 11100);

      auto* true_muon = myMCParticles.appendNew(MCParticle());
      true_muon->setPDG(Const::muon.getPDGCode());
      muonTrack->addRelationTo(true_muon);
      myKinkMuon->addRelationTo(true_muon);

      auto* true_electron = myMCParticles.appendNew(MCParticle());
      true_electron->setPDG(Const::electron.getPDGCode());
      electronTrack->addRelationTo(true_electron);
    }

    /** clear datastore */
    void TearDown() override
    {
      DataStore::Instance().reset();
    }
  };

  /*
   * Test kink vertex variables
   */
  TEST_F(KinkVariablesTest, KinkVertex)
  {
    StoreArray<Particle> myParticles{};
    auto part = myParticles[1];

    EXPECT_FLOAT_EQ(kinkVertexX(part), 1);
    EXPECT_FLOAT_EQ(kinkVertexY(part), 1);
    EXPECT_FLOAT_EQ(kinkVertexZ(part), 2);
  }

  /*
   * Test kink flag variables
   */
  TEST_F(KinkVariablesTest, KinkFlag)
  {
    StoreArray<Particle> myParticles{};
    auto part = myParticles[1];

    EXPECT_FLOAT_EQ(kinkCombinedFitResultFlag(part), 10);
    EXPECT_FLOAT_EQ(kinkCombinedFitResultFlagBit1(part), 0);
    EXPECT_FLOAT_EQ(kinkCombinedFitResultFlagBit2(part), 2);
    EXPECT_FLOAT_EQ(kinkCombinedFitResultFlagBit3(part), 0);
    EXPECT_FLOAT_EQ(kinkCombinedFitResultFlagBit4(part), 8);
  }

  /*
   * Test KinkDaughterTrack variables
   */
  TEST_F(KinkVariablesTest, KinkDaughterTrack)
  {
    StoreArray<Particle> myParticles{};
    auto part = myParticles[1];

    auto* var = Manager::Instance().getVariable("kinkDaughterTrack(nCDCHits)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 3);

    var = Manager::Instance().getVariable("kinkDaughterTrack(nSVDHits)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 24);

    var = Manager::Instance().getVariable("kinkDaughterTrack(nPXDHits)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 12);

    var = Manager::Instance().getVariable("kinkDaughterTrack(nVXDHits)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 36);

    var = Manager::Instance().getVariable("kinkDaughterTrack(firstSVDLayer)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 3);

    var = Manager::Instance().getVariable("kinkDaughterTrack(firstPXDLayer)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 1);

    var = Manager::Instance().getVariable("kinkDaughterTrack(firstCDCLayer)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), -1);

    var = Manager::Instance().getVariable("kinkDaughterTrack(lastCDCLayer)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), -1);

    var = Manager::Instance().getVariable("kinkDaughterTrack(pValue)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 0.6);

    var = Manager::Instance().getVariable("kinkDaughterTrack(ndf)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 5);

    var = Manager::Instance().getVariable("kinkDaughterTrack(chi2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 3.6554995);

    var = Manager::Instance().getVariable("kinkDaughterTrack(d0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 1.5421079);

    var = Manager::Instance().getVariable("kinkDaughterTrack(phi0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), -0.45246184);

    var = Manager::Instance().getVariable("kinkDaughterTrack(omega)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 0.0045218822);

    var = Manager::Instance().getVariable("kinkDaughterTrack(z0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), -0.5366869);

    var = Manager::Instance().getVariable("kinkDaughterTrack(tanLambda)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), -0.030219816);

    var = Manager::Instance().getVariable("kinkDaughterTrack(d0Err)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 0.41392139);

    var = Manager::Instance().getVariable("kinkDaughterTrack(phi0Err)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 0.18485548);

    var = Manager::Instance().getVariable("kinkDaughterTrack(omegaErr)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 0.00057035277);

    var = Manager::Instance().getVariable("kinkDaughterTrack(z0Err)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 0.63257021);

    var = Manager::Instance().getVariable("kinkDaughterTrack(tanLambdaErr)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 0.09413857);
  }

  /*
   * Test KinkDaughterInitTrack variables
   */
  TEST_F(KinkVariablesTest, KinkDaughterInitTrack)
  {
    StoreArray<Particle> myParticles{};
    auto part = myParticles[1];

    auto* var = Manager::Instance().getVariable("kinkDaughterInitTrack(pValue)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 0.6);

    var = Manager::Instance().getVariable("kinkDaughterInitTrack(ndf)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 5);

    var = Manager::Instance().getVariable("kinkDaughterInitTrack(chi2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 3.6554995);
  }

  /*
   * Test KinkMotherInitTrack variables
   */
  TEST_F(KinkVariablesTest, KinkMotherInitTrack)
  {
    StoreArray<Particle> myParticles{};
    auto part = myParticles[1];

    auto* var = Manager::Instance().getVariable("kinkMotherInitTrack(pValue)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 0.5);

    var = Manager::Instance().getVariable("kinkMotherInitTrack(ndf)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 15);

    var = Manager::Instance().getVariable("kinkMotherInitTrack(chi2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 14.33886);
  }

  /*
   * Test KinkPairDaughterMC variables
   */
  TEST_F(KinkVariablesTest, KinkPairDaughterMC)
  {
    StoreArray<Particle> myParticles;
    auto part = myParticles[1];

    auto* var = Manager::Instance().getVariable("kinkPairDaughterMC(mcPDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 11);

    var = Manager::Instance().getVariable("kinkPairDaughterMC(genMotherPDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 0.0);

    var = Manager::Instance().getVariable("kinkPairDaughterMC(mcSecPhysProc)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 0.0);

    var = Manager::Instance().getVariable("kinkPairDaughterMC(x)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 0.0);

    var = Manager::Instance().getVariable("kinkPairDaughterMC(y)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 0.0);

    var = Manager::Instance().getVariable("kinkPairDaughterMC(z)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 0.0);

    var = Manager::Instance().getVariable("kinkPairDaughterMC(px)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 0.0);

    var = Manager::Instance().getVariable("kinkPairDaughterMC(py)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 0.0);

    var = Manager::Instance().getVariable("kinkPairDaughterMC(pz)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 0.0);

    var = Manager::Instance().getVariable("kinkPairDaughterMC(pt)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 0.0);

    var = Manager::Instance().getVariable("kinkPairDaughterMC(p)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 0.0);

    var = Manager::Instance().getVariable("kinkPairDaughterMC(E)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 0.0);
  }

  /*
   * Test KinkPairMotherMC variables
   */
  TEST_F(KinkVariablesTest, KinkPairMotherMC)
  {
    StoreArray<Particle> myParticles{};
    auto part = myParticles[1];

    auto* var = Manager::Instance().getVariable("kinkPairMotherMC(px)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 0.0);

    var = Manager::Instance().getVariable("kinkPairMotherMC(py)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 0.0);

    var = Manager::Instance().getVariable("kinkPairMotherMC(pz)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 0.0);

    var = Manager::Instance().getVariable("kinkPairMotherMC(pt)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 0.0);

    var = Manager::Instance().getVariable("kinkPairMotherMC(p)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 0.0);

    var = Manager::Instance().getVariable("kinkPairMotherMC(E)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(std::get<double>(var->function(part)), 0.0);
  }
}
