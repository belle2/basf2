/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/datastore/DataStore.h>
#include <framework/database/DBStore.h>
#include <framework/dbobjects/BeamParameters.h>
#include <framework/utilities/CalcMeanCov.h>
#include <generators/utilities/InitialParticleGeneration.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;

namespace {

  /** Fixture class to test generation of initial particles with varying beam
   * parameters */
  class InitialParticleGenerationTests : public ::testing::Test {
  protected:

    /** setup the InitialParticleGenerator */
    virtual void SetUp()
    {
      DataStore::Instance().setInitializeActive(true);
      generator.initialize();
      DataStore::Instance().setInitializeActive(false);

      // have some useful defaults for the beam parameters
      beamparams.setHER(7.004, 0.0415,  0, {2.63169e-05, 1e-5, 1e-5});
      beamparams.setLER(4.002, -0.0415, 0, {5.64063e-06, 1e-5, 1e-5});
      beamparams.setVertex({0, 1, 2}, {4.10916e-07, 0, -2.64802e-06, 0, 1.7405e-11, 0, -2.64802e-06, 0, 0.000237962});
    }

    /** reset datastore and dbstore */
    virtual void TearDown()
    {
      DataStore::Instance().reset();
      DBStore::Instance().reset();
    }

    InitialParticleGeneration generator{BeamParameters::c_smearALL};
    BeamParameters beamparams;
  };

  /** Test generation in CMS */
  TEST_F(InitialParticleGenerationTests, TestCMSGeneration)
  {
    beamparams.setGenerationFlags(BeamParameters::c_generateCMS);
    DBStore::Instance().addConstantOverride("BeamParameters", new BeamParameters(beamparams));
    auto initialCMS = generator.generate();
    // transformation should be identity
    EXPECT_EQ(initialCMS.getLabToCMS(), ROOT::Math::LorentzRotation());
    // her should be along z
    EXPECT_NEAR(initialCMS.getHER().Theta(), 0, 1e-15);
    // ler should be opposite z
    EXPECT_NEAR(initialCMS.getLER().Theta(), M_PI, 1e-15);
    // both should have the same energy
    EXPECT_NEAR(initialCMS.getHER().E(), initialCMS.getLER().E(), 1e-15);
    // and the invariant mass should of course be correct
    EXPECT_EQ(initialCMS.getMass(), beamparams.getMass());

    // and now compare it to generation in Lab
    beamparams.setGenerationFlags(0);
    DBStore::Instance().addConstantOverride("BeamParameters", new BeamParameters(beamparams));
    auto initialLAB = generator.generate();
    // same invariant mass
    EXPECT_EQ(initialLAB.getMass(), beamparams.getMass());
    // no smearing so LER and HER need to be identical to beam parameters
    EXPECT_EQ(initialLAB.getHER(), beamparams.getHER());
    EXPECT_EQ(initialLAB.getLER(), beamparams.getLER());
    // so check that the CMS beams are identical to the transformed lab ones
    EXPECT_EQ(initialCMS.getHER(), initialLAB.getLabToCMS() * initialLAB.getHER());
    EXPECT_EQ(initialCMS.getLER(), initialLAB.getLabToCMS() * initialLAB.getLER());
  }

  /** test beam energy smearing. Here we only smear the energy, not the directions */
  TEST_F(InitialParticleGenerationTests, TestEnergySmear)
  {
    // two beams completely head on, one with energy uncertainty so invariant
    // mass uncertainty should be exactly the same as single uncertainty. Here
    // we give variance so we expect 0.1
    beamparams.setHER(10, 0, 0, {0.01});
    beamparams.setLER(10, M_PI, 0, {0});
    beamparams.setGenerationFlags(BeamParameters::c_smearBeamEnergy);
    DBStore::Instance().addConstantOverride("BeamParameters", new BeamParameters(beamparams));

    // so let's generate events with this and store all invariant masses
    {
      CalcMeanCov<1> mean;
      for (int i = 0; i < 100000; ++i) {
        auto& initial = generator.generate();
        ASSERT_TRUE(initial.hasGenerationFlags(BeamParameters::c_smearBeamEnergy));
        mean.add(initial.getMass());
      }
      // and compare with expectation
      EXPECT_NEAR(mean.getStddev(), 0.1, 0.0005);
      EXPECT_NEAR(mean.getMean(), 20, 0.001);
    }

    // uncertainty on both so result should be sqrt(a^2 + b^2) = sqrt(2) * 0.1
    beamparams.setLER(10, M_PI, 0, {0.01});
    beamparams.setGenerationFlags(BeamParameters::c_smearBeamEnergy);
    DBStore::Instance().addConstantOverride("BeamParameters", new BeamParameters(beamparams));
    {
      CalcMeanCov<1> mean;
      for (int i = 0; i < 100000; ++i) {
        auto& initial = generator.generate();
        ASSERT_TRUE(initial.hasGenerationFlags(BeamParameters::c_smearBeamEnergy));
        mean.add(initial.getMass());
      }
      EXPECT_NEAR(mean.getStddev(), std::sqrt(2) * 0.1, 0.0005);
      EXPECT_NEAR(mean.getMean(), 20, 0.001);
    }
  }


  /** Test vertex smearing: we generate a set of events and check whether the
   * generated vertex distribution is in accordance with te mean and covariance
   * matrix actually put in */
  TEST_F(InitialParticleGenerationTests, TestVertexSmear)
  {
    beamparams.setGenerationFlags(BeamParameters::c_smearVertex);
    DBStore::Instance().addConstantOverride("BeamParameters", new BeamParameters(beamparams));

    CalcMeanCov<3> mean;
    for (int i = 0; i < 100000; ++i) {
      auto& initial = generator.generate();
      mean.add(initial.getVertex().X(), initial.getVertex().Y(), initial.getVertex().Z());
    }
    auto cov = beamparams.getCovVertex();
    auto pos = beamparams.getVertex();
    for (int i = 0; i < 3; ++i) {
      EXPECT_NEAR(mean.getMean(i), pos(i), 1e-4);
      for (int j = 0; j < 3; ++j) {
        EXPECT_NEAR(mean.getCovariance(i, j), cov(i, j), 1e-6);
      }
    }
  }

  /** Test generation with different flags.
   * This test loops through all generation flags and will check whether the
   * flags are set correctly. In addition we generate a few events with each
   * flag and will check that the generated beams and or vertex are identical
   * to the initial settings and the previous event if smearing is disabled. If
   * smearing is enabled we check that the value differs from both settings and
   * previous event. This makes sure that smearing is enabled by checking for
   * differences but will not quantify if smearing is correct */
  TEST_F(InitialParticleGenerationTests, TestFlags)
  {
    // so loop over all flags
    for (int flag = 0; flag <= BeamParameters::c_smearALL; ++flag) {
      // set the flag and overwrite dbstore
      beamparams.setGenerationFlags(flag);
      DBStore::Instance().addConstantOverride("BeamParameters", new BeamParameters(beamparams));
      // rememeber last event and set it to the settings for initialization
      MCInitialParticles last = beamparams;
      // no generate a few events and check everything
      for (int i = 0; i < 5; ++i) {
        auto& initial = generator.generate();
        EXPECT_EQ(flag, initial.getGenerationFlags());
        // create text representation of flags
        const std::string flags = initial.getGenerationFlagString();
        if (flag & BeamParameters::c_smearBeam) {
          // smearing of beam is active, check that the beams are actually
          // different to the settings and the previous event
          EXPECT_NE(initial.getHER(), beamparams.getHER()) << flags << " " << i;
          EXPECT_NE(initial.getLER(), beamparams.getLER()) << flags << " " << i;
          EXPECT_NE(initial.getHER(), last.getHER()) << flags << " " << i;
          EXPECT_NE(initial.getLER(), last.getLER()) << flags << " " << i;
        } else if (!(flag & BeamParameters::c_generateCMS)) {
          // no smearing no cms generation, so everything should stay fixed.
          EXPECT_EQ(initial.getHER(), beamparams.getHER()) << flags << " " << i;
          EXPECT_EQ(initial.getLER(), beamparams.getLER()) << flags << " " << i;
          EXPECT_EQ(initial.getHER(), last.getHER()) << flags << " " << i;
          EXPECT_EQ(initial.getLER(), last.getLER()) << flags << " " << i;
        } else {
          // we want to compare CMS initial particles to lab beam energies
          // but transformation is identity if c_generateCMS is set. Reset
          // it so that the can boost beams to CMS for comparison
          beamparams.setGenerationFlags(0);
          EXPECT_EQ(initial.getHER(), beamparams.getLabToCMS() * beamparams.getHER()) << flags << " " << i;
          EXPECT_EQ(initial.getLER(), beamparams.getLabToCMS() * beamparams.getLER()) << flags << " " << i;
          // comparison to beamparameters fails in generateCMS so we just
          // skip the first round here
          if (i > 0) {
            EXPECT_EQ(initial.getHER(), last.getHER()) << flags << " " << i;
            EXPECT_EQ(initial.getLER(), last.getLER()) << flags << " " << i;
          }
        }
        if (flag & BeamParameters::c_smearVertex) {
          //smearing of the vertex is enabled, make sure the vertex changes
          EXPECT_NE(initial.getVertex(), beamparams.getVertex());
          EXPECT_NE(initial.getVertex(), last.getVertex());
        } else {
          //otherwise make sure it doesn't change.
          EXPECT_EQ(initial.getVertex(), beamparams.getVertex());
          EXPECT_EQ(initial.getVertex(), last.getVertex());
        }
        last = initial;
      }
    }
  }

  /** Test the functionality of the valid flag. */
  TEST_F(InitialParticleGenerationTests, TestValidFlag)
  {
    beamparams.setGenerationFlags(0);
    DBStore::Instance().addConstantOverride("BeamParameters", new BeamParameters(beamparams));

    MCInitialParticles& initial = generator.generate();
    ROOT::Math::PxPyPzEVector her = initial.getHER();
    ROOT::Math::PxPyPzEVector ler = initial.getLER();
    TVector3 vertex = initial.getVertex();
    for (int i = 0; i < 10; ++i) {
      initial = generator.generate();
      EXPECT_EQ(her, initial.getHER());
      EXPECT_EQ(ler, initial.getLER());
      EXPECT_EQ(vertex, initial.getVertex());
    }
    beamparams.setGenerationFlags(BeamParameters::c_smearALL);
    DBStore::Instance().addConstantOverride("BeamParameters", new BeamParameters(beamparams));
    for (int i = 0; i < 10; ++i) {
      initial = generator.generate();
      EXPECT_NE(her, initial.getHER());
      EXPECT_NE(ler, initial.getLER());
      EXPECT_NE(vertex, initial.getVertex());
      her = initial.getHER();
      ler = initial.getLER();
      vertex = initial.getVertex();
    }
  }

  TEST_F(InitialParticleGenerationTests, UpdateVertex)
  {
    beamparams.setGenerationFlags(BeamParameters::c_smearBeam);
    DBStore::Instance().addConstantOverride("BeamParameters", new BeamParameters(beamparams));
    // first run but no smearing allowed, should return the nominal vertex
    TVector3 shift = generator.updateVertex();
    EXPECT_EQ(shift, TVector3(0, 1, 2));
    // create a new initial particle. Particle exists now, no smearing allowed so no change in shift
    const MCInitialParticles& initial = generator.generate();
    auto nominal = initial.getVertex();
    shift = generator.updateVertex();
    EXPECT_EQ(shift, TVector3(0, 0, 0));
    // ok, allow smearing, now we expect shift
    beamparams.setGenerationFlags(BeamParameters::c_smearALL);
    DBStore::Instance().addConstantOverride("BeamParameters", new BeamParameters(beamparams));
    shift = generator.updateVertex();
    EXPECT_NE(shift, TVector3(0, 0, 0));
    EXPECT_EQ(nominal + shift, initial.getVertex());
    // but running again should not shift again
    shift = generator.updateVertex();
    EXPECT_EQ(shift, TVector3(0, 0, 0));
    // unless we force regeneration
    auto previous = initial.getVertex();
    shift = generator.updateVertex(true);
    EXPECT_NE(shift, TVector3(0, 0, 0));
    EXPECT_EQ(previous + shift, initial.getVertex());
  }
}
