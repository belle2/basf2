/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <gtest/gtest.h>

#include <TMatrixFSym.h>
#include <TRandom3.h>
#include <TLorentzVector.h>
#include <TMath.h>

#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/EventExtraInfo.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/utility/ReferenceFrame.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/utilities/TestHelpers.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>

#include <analysis/variables/Variables.h>

#include "utilities/TestParticleFactory.h"
using namespace std;
using namespace Belle2;
using namespace Belle2::Variable;

namespace {


  class InclusiveVariablesTest : public ::testing::Test {
  protected:
    /** register Particle array + ParticleExtraInfoMap object. */
    void SetUp() override
    {

      DataStore::Instance().setInitializeActive(true);
      StoreArray<ECLCluster> myECLClusters;
      StoreArray<KLMCluster> myKLMClusters;
      StoreArray<TrackFitResult> myTFRs;
      StoreArray<Track> myTracks;
      StoreArray<Particle> myParticles;
      StoreArray<RestOfEvent> myROEs;
      StoreArray<PIDLikelihood> myPIDLikelihoods;
      myECLClusters.registerInDataStore();
      myKLMClusters.registerInDataStore();
      myTFRs.registerInDataStore();
      myTracks.registerInDataStore();
      myParticles.registerInDataStore();
      myROEs.registerInDataStore();
      myPIDLikelihoods.registerInDataStore();
      myParticles.registerRelationTo(myROEs);
      myTracks.registerRelationTo(myPIDLikelihoods);
      DataStore::Instance().setInitializeActive(false);

      TestUtilities::TestParticleFactory factory;
      TVector3 ipposition(0, 0, 0);
      TLorentzVector b0momentum(3, 0, 0, 5);
      factory.produceParticle(string("^B0 -> [^K_S0 -> ^pi+ ^pi-] [^pi0 -> ^gamma ^gamma] ^e+ ^e-"), b0momentum, ipposition);
    }

    /** clear datastore */
    void TearDown() override
    {
      DataStore::Instance().reset();
    }
  };
  TEST_F(InclusiveVariablesTest, nCompositeDaughters)
  {
    StoreArray<Particle> myParticles;
    auto* var = Manager::Instance().getVariable("nCompositeDaughters");
    EXPECT_EQ(std::get<int>(var->function(myParticles[8])), 2);
  }
  TEST_F(InclusiveVariablesTest, nPhotonDaughters)
  {
    StoreArray<Particle> myParticles;
    auto* var = Manager::Instance().getVariable("nDaughterPhotons");
    EXPECT_EQ(std::get<int>(var->function(myParticles[8])), 2);
  }
  TEST_F(InclusiveVariablesTest, nDaughterNeutralHadrons)
  {
    StoreArray<Particle> myParticles;
    auto* var = Manager::Instance().getVariable("nDaughterNeutralHadrons");
    EXPECT_EQ(std::get<int>(var->function(myParticles[8])), 0);
  }
  TEST_F(InclusiveVariablesTest, nChargedDaughters)
  {
    StoreArray<Particle> myParticles;
    auto* var = Manager::Instance().getVariable("nDaughterCharged()");
    EXPECT_EQ(std::get<int>(var->function(myParticles[8])), 4);
    var = Manager::Instance().getVariable("nDaughterCharged(11)");
    EXPECT_EQ(std::get<int>(var->function(myParticles[8])), 2);
    var = Manager::Instance().getVariable("nDaughterCharged(211)");
    EXPECT_EQ(std::get<int>(var->function(myParticles[8])), 2);

  }
  TEST_F(InclusiveVariablesTest, daughterAverageOf)
  {
    StoreArray<Particle> myParticles;
    auto* var = Manager::Instance().getVariable("daughterAverageOf(PDG)");
    EXPECT_FLOAT_EQ(std::get<double>(var->function(myParticles[8])), 105.25);
  }

}

