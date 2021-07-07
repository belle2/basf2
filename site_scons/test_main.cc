/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/** @file
 *
 * Main function to unit test executables.
 * This file contains basically a copy of the gtest_main.cc provided by the
 * gtest framework but extends it with a custom event listener to set the
 * random state before each test to a well defined state.
 */

#include "gtest/gtest.h"
#include "framework/core/RandomNumbers.h"
#include <framework/logging/LogSystem.h>
#include <framework/logging/LogConfig.h>
#include <framework/dbobjects/MagneticField.h>
#include <framework/dbobjects/MagneticFieldComponentConstant.h>
#include <mdst/dbobjects/CollisionInvariantMass.h>
#include <mdst/dbobjects/CollisionBoostVector.h>
#include <mdst/dbobjects/BeamSpot.h>
#include <framework/database/DBStore.h>

namespace {
  /** Class to set up random numbers before running each test.
   * The random generator will be set to event independent mode with the name
   * of the test "<testcase>/<test>" as seed.
   */
  class TestEventListener: public ::testing::EmptyTestEventListener {
  public:
    /** Constructor to choose log level */
    TestEventListener(bool debug): ::testing::EmptyTestEventListener(), m_enableDebug{debug} {}
  private:
    /** Set the random state before the test is run.
     * Also provide a constant magnetic field everywhere until the tests which
     * just assume a magnetic field are fixed (BII-2657). */
    virtual void OnTestStart(const ::testing::TestInfo& test) final override
    {
      if (m_enableDebug) {
        Belle2::LogSystem::Instance().getLogConfig()->setDebugLevel(10000);
        Belle2::LogSystem::Instance().getLogConfig()->setLogLevel(Belle2::LogConfig::c_Debug);
      }
      std::string name = test.test_case_name();
      name += "/";
      name += test.name();
      Belle2::RandomNumbers::initialize(name);
      Belle2::MagneticField* field = new Belle2::MagneticField();
      field->addComponent(new Belle2::MagneticFieldComponentConstant({0, 0, 1.5 * Belle2::Unit::T}));
      Belle2::DBStore::Instance().addConstantOverride("MagneticField", field, false);
      auto* collisionInvariantMass = new Belle2::CollisionInvariantMass();
      collisionInvariantMass->setMass(10.5738932579, 0, 0);
      Belle2::DBStore::Instance().addConstantOverride("CollisionInvariantMass", collisionInvariantMass);
      auto* collisionBoostVector = new Belle2::CollisionBoostVector();
      collisionBoostVector->setBoost(TVector3(0.0414880886031, 0, 0.272492455429), TMatrixDSym(3));
      Belle2::DBStore::Instance().addConstantOverride("CollisionBoostVector", collisionBoostVector);
      auto* beamSpot = new Belle2::BeamSpot();
      Belle2::DBStore::Instance().addConstantOverride("BeamSpot", beamSpot);
    }
    /** Reset the logsytem after each test */
    virtual void OnTestEnd(const ::testing::TestInfo&) final override
    {
      Belle2::LogSystem::Instance().resetLogging();
    }

    /** If true enable all log messages */
    bool m_enableDebug;
  };
}

/** Run all tests */
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::UnitTest& unit_test = *::testing::UnitTest::GetInstance();
  bool debug{false};
  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "--debug") debug = true;
  }
  unit_test.listeners().Append(new TestEventListener(debug));
  return RUN_ALL_TESTS();
}
