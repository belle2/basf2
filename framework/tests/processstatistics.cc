#include <framework/core/ProcessStatistics.h>
#include <framework/logging/Logger.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;

namespace {
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
    a.startModule();
    a.stopModule(nullptr, ModuleStatistics::c_Event);
    a.stopGlobal(ModuleStatistics::c_Init);

    b.startGlobal();
    b.stopGlobal(ModuleStatistics::c_Init);

    EXPECT_EQ(1, a.getGlobal().getCalls());
    EXPECT_EQ(1, a.getStatistics(nullptr).getCalls());
    float sum = a.getGlobal().getTimeSum();
    EXPECT_TRUE(sum > 0);

    a.merge(&b);


    EXPECT_EQ(1, a.getGlobal().getCalls());
    EXPECT_EQ(1, a.getStatistics(nullptr).getCalls());
    EXPECT_FLOAT_EQ(sum, a.getGlobal().getTimeSum());
  }
}  // namespace
