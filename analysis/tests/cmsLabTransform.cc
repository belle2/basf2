/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/utility/PCmsLabTransform.h>

#include <framework/database/DBStore.h>
#include <framework/utilities/LabToCms.h>
#include <mdst/dbobjects/CollisionAxisCMS.h>
#include <mdst/dbobjects/CollisionBoostVector.h>
#include <mdst/dbobjects/CollisionInvariantMass.h>

#include <TMatrixDSym.h>
#include <TVector3.h>

#include <gtest/gtest.h>

using namespace Belle2;
using namespace ROOT::Math;

namespace {

  /** Beam parameters used to override the payloads. */
  struct BeamValues {
    /** CMS energy */
    double mass;
    /** boost vector */
    XYZVector boost;
    /** collision axis angle in the XZ plane */
    double angleXZ;
    /** collision axis angle in the YZ plane */
    double angleYZ;
  };

  /** Default values used in the unit tests (see site_scons/test_main.cc) */
  const BeamValues c_valuesA{10.579557836806245064, XYZVector(0.041488088603337358595, 0, 0.27252546681036521337), 0.0057699660560828307243, 0};
  /** Different values, as for a different run */
  const BeamValues c_valuesB{10.355, XYZVector(0.030, 0.001, 0.29), 0.0041, 0.0003};

  /** Override the invariant mass payload (the accessors are notified as for a payload update). */
  void setInvariantMass(double mass)
  {
    auto* invariantMass = new CollisionInvariantMass();
    invariantMass->setMass(mass, 0, 0);
    DBStore::Instance().addConstantOverride("CollisionInvariantMass", invariantMass);
  }

  /** Override the boost vector payload. */
  void setBoost(const XYZVector& boost)
  {
    auto* boostVector = new CollisionBoostVector();
    boostVector->setBoost(TVector3(boost.X(), boost.Y(), boost.Z()), TMatrixDSym(3));
    DBStore::Instance().addConstantOverride("CollisionBoostVector", boostVector);
  }

  /** Override the collision axis payload. */
  void setAxis(double angleXZ, double angleYZ)
  {
    auto* axisCms = new CollisionAxisCMS();
    axisCms->setAngles(angleXZ, angleYZ, TMatrixDSym(2));
    axisCms->setSpread(TMatrixDSym(2), 0, 0, 0);
    DBStore::Instance().addConstantOverride("CollisionAxisCMS", axisCms);
  }

  /** Override all three payloads. */
  void setPayloads(const BeamValues& values)
  {
    setInvariantMass(values.mass);
    setBoost(values.boost);
    setAxis(values.angleXZ, values.angleYZ);
  }

  /** Check that two four-vectors are equal (up to rounding). */
  void expectEqual(const PxPyPzEVector& expected, const PxPyPzEVector& actual)
  {
    EXPECT_NEAR(expected.Px(), actual.Px(), 1e-12);
    EXPECT_NEAR(expected.Py(), actual.Py(), 1e-12);
    EXPECT_NEAR(expected.Pz(), actual.Pz(), 1e-12);
    EXPECT_NEAR(expected.E(), actual.E(), 1e-12);
  }

  /** Check all the PCmsLabTransform getters against a direct (uncached) computation from the given values. */
  void checkTransform(const BeamValues& values)
  {
    const PCmsLabTransform T;
    const LorentzRotation labToCms = LabToCms::rotateLabToCms(values.boost, values.angleXZ, values.angleYZ);
    const LorentzRotation cmsToLab = labToCms.Inverse();

    EXPECT_DOUBLE_EQ(values.mass, T.getCMSEnergy());
    EXPECT_DOUBLE_EQ(values.boost.X(), T.getBoostVector().X());
    EXPECT_DOUBLE_EQ(values.boost.Y(), T.getBoostVector().Y());
    EXPECT_DOUBLE_EQ(values.boost.Z(), T.getBoostVector().Z());
    expectEqual(cmsToLab * PxPyPzEVector(0, 0, 0, values.mass), T.getBeamFourMomentum());

    for (const PxPyPzEVector& vector : {PxPyPzEVector(0.1, -0.2, 1.5, 2.0), PxPyPzEVector(-1.0, 0.5, -2.0, 3.0)}) {
      expectEqual(labToCms * vector, T.rotateLabToCms() * vector);
      expectEqual(cmsToLab * vector, T.rotateCmsToLab() * vector);
      expectEqual(labToCms * vector, PCmsLabTransform::labToCms(vector));
      expectEqual(cmsToLab * vector, PCmsLabTransform::cmsToLab(vector));
    }
  }

  /** The beam payloads are overridden before each test in site_scons/test_main.cc */
  class PCmsLabTransformTest : public ::testing::Test {
  protected:
    /** Restore the default values, as other tests rely on them. */
    void TearDown() override
    {
      setPayloads(c_valuesA);
    }
  };

  /** The cached values agree with a direct computation from the payloads. */
  TEST_F(PCmsLabTransformTest, MatchesPayloads)
  {
    setPayloads(c_valuesA);
    checkTransform(c_valuesA);
    // a second call uses the cached values
    checkTransform(c_valuesA);
  }

  /** A change of any of the three payloads (e.g. at a run change) invalidates the cache. */
  TEST_F(PCmsLabTransformTest, FollowsPayloadChanges)
  {
    BeamValues values = c_valuesA;
    setPayloads(values);
    checkTransform(values);

    values.mass = c_valuesB.mass;
    setInvariantMass(values.mass);
    checkTransform(values);

    values.boost = c_valuesB.boost;
    setBoost(values.boost);
    checkTransform(values);

    values.angleXZ = c_valuesB.angleXZ;
    values.angleYZ = c_valuesB.angleYZ;
    setAxis(values.angleXZ, values.angleYZ);
    checkTransform(values);

    // and back to the original values
    setPayloads(c_valuesA);
    checkTransform(c_valuesA);
  }

  /** A reset of the DBStore (e.g. between two calls of basf2.process()) invalidates the cache. */
  TEST_F(PCmsLabTransformTest, FollowsDBStoreReset)
  {
    setPayloads(c_valuesA);
    checkTransform(c_valuesA);

    DBStore::Instance().reset();
    setPayloads(c_valuesB);
    checkTransform(c_valuesB);

    DBStore::Instance().reset();
    setPayloads(c_valuesA);
    checkTransform(c_valuesA);
  }

}
