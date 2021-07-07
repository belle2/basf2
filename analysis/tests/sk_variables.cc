/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <gtest/gtest.h>
#include "utilities/TestParticleFactory.h"

#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/datastore/StoreArray.h>
#include <analysis/utility/PCmsLabTransform.h>

#include <analysis/variables/SpecificKinematicVariables.h>
#include <analysis/variables/Variables.h>

#include <analysis/VariableManager/Manager.h>

#include <TLorentzVector.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Variable;
namespace {
  class SpecificKinematicVariablesTest : public ::testing::Test {
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
      myECLClusters.registerInDataStore();
      myKLMClusters.registerInDataStore();
      myTFRs.registerInDataStore();
      myTracks.registerInDataStore();
      myParticles.registerInDataStore();
      DataStore::Instance().setInitializeActive(false);

      TestUtilities::TestParticleFactory factory;
      PCmsLabTransform T;
      TLorentzVector b0momentum(.20, 0., 0., 4.85);
      b0momentum = T.rotateCmsToLab() * b0momentum;
      TLorentzVector pimomentum(0.1, 0, 2.5, sqrt(0.139 * 0.139 + 2.5 * 2.5));
      TLorentzVector emomentum(0., 0, 1., 1.);
      TVector3 ipposition(0, 0, 0);
      factory.produceParticle(string("^B0 -> pi- e+"), b0momentum, ipposition);
      myParticles[0]->set4Vector(pimomentum); //pion
      myParticles[1]->set4Vector(emomentum);  //electron
    }

    /** clear datastore */
    void TearDown() override
    {
      DataStore::Instance().reset();
    }
  };

  TEST_F(SpecificKinematicVariablesTest, REC_q2BhSimple)
  {
    StoreArray<Particle> myParticles;
    B2INFO("Cos: " << cosThetaBetweenParticleAndNominalB(myParticles[2]));
    EXPECT_FLOAT_EQ(7.9083395, REC_q2BhSimple(myParticles[2])); // B-meson
  }
  TEST_F(SpecificKinematicVariablesTest, REC_q2Bh)
  {
    StoreArray<Particle> myParticles;
    EXPECT_FLOAT_EQ(7.8781433, REC_q2Bh(myParticles[2])); // B-meson
  }
  TEST_F(SpecificKinematicVariablesTest, REC_MissM2)
  {
    StoreArray<Particle> myParticles;
    EXPECT_FLOAT_EQ(0.15092255, REC_MissM2(myParticles[2])); // B-meson
  }

}
