/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/TMVAInterface/Method.h>
#include <analysis/TMVAInterface/Config.h>
#include <analysis/TMVAInterface/Expert.h>
#include <analysis/TMVAMockPlugin/MethodMockPlugin.h>
#include <analysis/TMVAMockPluginInspector/MockPluginInspector.h>
#include <analysis/dataobjects/Particle.h>
#include <framework/utilities/FileSystem.h>
#include <framework/utilities/TestHelpers.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <framework/datastore/StoreObjPtr.h>

#include <gtest/gtest.h>

#include <TRandom3.h>

#include <fstream>
#include <vector>

using namespace Belle2;
using namespace Belle2::TMVAInterface;

namespace {


  TEST(ExpertTest, RawOutputOfClassifierIsCorrect)
  {
    //network only expects 'eid' and 'p' as input and muid, M as spectator
    //eid isn't set (-> 0.5)
    Particle p({ 0.0 , 1.0, 0.0, 1.1 }, 11);
    MockPluginInspector& inspector = MockPluginInspector::GetInstance();

    ExpertConfig config("TMVA", FileSystem::findFile("/analysis/TMVAInterface/tests/"), "MockPlugin", 1);
    Expert expert(config, false);

    // no transformation
    inspector.SetMvaValue(0.9);
    EXPECT_FLOAT_EQ(0.9, expert.analyse(&p));
    inspector.SetMvaValue(1.9);
    EXPECT_FLOAT_EQ(1.9, expert.analyse(&p));
    inspector.SetMvaValue(-0.9);
    EXPECT_FLOAT_EQ(-0.9, expert.analyse(&p));
  }

  TEST(ExpertTest, ReceivedFeatureVariablesAreCorrect)
  {
    //network only expects 'eid' and 'p' as input and muid, M as spectator
    //eid isn't set (-> 0.5)
    Particle p({ 0.0 , 1.0, 0.0, 1.1 }, 11);
    MockPluginInspector& inspector = MockPluginInspector::GetInstance();

    ExpertConfig config("TMVA", FileSystem::findFile("/analysis/TMVAInterface/tests/"), "MockPlugin", 1);
    Expert expert(config, false);

    // no transformation
    inspector.SetMvaValue(0.9);
    expert.analyse(&p);

    // Correct input values
    std::vector<float> test_event = inspector.GetTestEvent();
    EXPECT_FLOAT_EQ(0.5, test_event[0]); // eid
    EXPECT_FLOAT_EQ(1.0, test_event[1]); // p

  }

  TEST(ExpertTest, TransformToProbabilityOfClassifierWithTrainingSignalFraction)
  {
    //network only expects 'eid' and 'p' as input and muid, M as spectator
    //eid isn't set (-> 0.5)
    Particle p({ 0.0 , 1.0, 0.0, 1.1 }, 11);
    MockPluginInspector& inspector = MockPluginInspector::GetInstance();
    inspector.SetMvaValue(0.0);

    ExpertConfig config("TMVA", FileSystem::findFile("/analysis/TMVAInterface/tests/"), "MockPlugin", 1);
    Expert expert(config, true);
    // Test probability transformation of mva output. Training was done with constant mva output == 0
    // therefore we expect probability == S/(S+B)== signalFraction
    // signalToBackgroundRatio >= 0 -> Use given signalFraction
    EXPECT_FLOAT_EQ(0.2, expert.analyse(&p));
  }

  TEST(ExpertTest, TransformToProbabilityOfClassifierWithUserSignalFraction)
  {
    //network only expects 'eid' and 'p' as input and muid, M as spectator
    //eid isn't set (-> 0.5)
    Particle p({ 0.0 , 1.0, 0.0, 1.1 }, 11);
    MockPluginInspector& inspector = MockPluginInspector::GetInstance();
    inspector.SetMvaValue(0.0);

    ExpertConfig config("TMVA", FileSystem::findFile("/analysis/TMVAInterface/tests/"), "MockPlugin", 1, 0.1);
    Expert expert(config, true);
    EXPECT_FLOAT_EQ(0.1, expert.analyse(&p));
  }

  TEST(ExpertTest, ErrorIfSignalFractionIsTooHigh)
  {
    //network only expects 'eid' and 'p' as input and muid, M as spectator
    //eid isn't set (-> 0.5)
    Particle p({ 0.0 , 1.0, 0.0, 1.1 }, 11);
    MockPluginInspector& inspector = MockPluginInspector::GetInstance();
    inspector.SetMvaValue(0.0);

    ExpertConfig config("TMVA", FileSystem::findFile("/analysis/TMVAInterface/tests/"), "MockPlugin", 1, 1.1);
    EXPECT_B2ERROR(Expert expert(config, true));

  }


  TEST(ExpertTest, WarningIf999IsMVAOutputRawOutput)
  {
    //network only expects 'eid' and 'p' as input
    //eid isn't set (-> 0.5)
    Particle p({ 0.0 , std::numeric_limits<float>::signaling_NaN(), 0.0, std::numeric_limits<float>::infinity() }, 11);

    ExpertConfig config("TMVA", FileSystem::findFile("/analysis/TMVAInterface/tests/"), "MockPlugin", 1);
    Expert expert(config, false);

    EXPECT_B2WARNING(expert.analyse(&p))
    EXPECT_FLOAT_EQ(-999, expert.analyse(&p));
  }

  TEST(ExpertTest, WarningIf999IsMVAOutputTransformedOutput)
  {
    //network only expects 'eid' and 'p' as input
    //eid isn't set (-> 0.5)
    Particle p({ 0.0 , std::numeric_limits<float>::signaling_NaN(), 0.0, std::numeric_limits<float>::infinity() }, 11);

    ExpertConfig config("TMVA", FileSystem::findFile("/analysis/TMVAInterface/tests/"), "MockPlugin", 1);
    Expert expert(config, true);

    EXPECT_B2WARNING(expert.analyse(&p))
    EXPECT_FLOAT_EQ(-999, expert.analyse(&p));
  }

}
