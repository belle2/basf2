/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/core/ProcessStatistics.h>
#include <framework/core/Module.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;

namespace {
  class DummyModule : public Module {
  public:
    DummyModule()
    {
      setName("Dummy");
      setType("Dummy");
    }
  };

  TEST(ProcessStatisticsTest, Clear)
  {
    ProcessStatistics a;
    EXPECT_EQ(0, a.getGlobal().getCalls());
    EXPECT_EQ(0, a.getStatistics(nullptr).getCalls());
    EXPECT_EQ(0, a.getGlobal().getTimeMean());

    a.startGlobal();
    a.startModule();
    a.stopModule(nullptr, ModuleStatistics::c_Event);
    a.stopGlobal(ModuleStatistics::c_Init);

    EXPECT_EQ(1, a.getGlobal().getCalls());
    EXPECT_EQ(1, a.getStatistics(nullptr).getCalls());
    EXPECT_TRUE(a.getGlobal().getTimeMean() > 0);

    a.clear();

    EXPECT_EQ(0, a.getGlobal().getCalls());
    EXPECT_EQ(0, a.getStatistics(nullptr).getCalls());
    EXPECT_EQ(0, a.getGlobal().getTimeMean());
  }

  TEST(ProcessStatisticsTest, Merge)
  {
    ProcessStatistics a;
    ProcessStatistics b;

    a.startGlobal();
    a.stopGlobal(ModuleStatistics::c_Init);
    a.startModule();
    a.stopModule(nullptr, ModuleStatistics::c_Event);

    b.startGlobal();
    b.stopGlobal(ModuleStatistics::c_Init);
    b.startModule();
    b.stopModule(nullptr, ModuleStatistics::c_Event);

    EXPECT_EQ(1, a.getGlobal().getCalls());
    EXPECT_EQ(1, a.getStatistics(nullptr).getCalls());
    float sum = a.getGlobal().getTimeSum();
    EXPECT_TRUE(sum > 0);

    a.merge(&b); //all modules are there


    EXPECT_EQ(1, a.getGlobal().getCalls());
    EXPECT_EQ(2, a.getStatistics(nullptr).getCalls());
    EXPECT_FLOAT_EQ(sum, a.getGlobal().getTimeSum());


    //Add another module
    ProcessStatistics c;
    DummyModule dummyMod;
    c.startGlobal();
    c.stopGlobal(ModuleStatistics::c_Init);
    c.startModule();
    c.stopModule(&dummyMod, ModuleStatistics::c_Event);

    sum += c.getGlobal().getTimeSum();
    a.merge(&c); //different merge process used, global time should also increase

    EXPECT_EQ(2, a.getGlobal().getCalls());
    EXPECT_EQ(2, a.getStatistics(nullptr).getCalls());
    EXPECT_EQ(1, a.getStatistics(&dummyMod).getCalls());
    EXPECT_FLOAT_EQ(sum, a.getGlobal().getTimeSum());
  }
}  // namespace
