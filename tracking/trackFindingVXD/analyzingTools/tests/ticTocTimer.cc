/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Madlener                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <gtest/gtest.h>

#include <tracking/trackFindingVXD/analyzingTools/TicTocTimer.h>

#include <iostream>
#include <chrono>
#include <string>

using namespace std::chrono;

namespace Belle2 {

  class TicTocTimerTest : public ::testing::Test {

  protected:

    void testConstructorStartsTiming(); /**< test if the constructor starts the clock */

    void testTicStartsTiming(); /**< test if tics starts the clock */

    void testTicResetsTocked(); /**< test if tic resets the tocked status */

    void testTocStopsTiming(); /**< test if toc stops the clock */
  };

  /** Test the constructor */
  TEST_F(TicTocTimerTest, testConstructor)
  {
    testConstructorStartsTiming();
    TicTocTimer tmr;
    EXPECT_EQ("ns", tmr.getUnit());

    TicTocTimer timer(1000);
    EXPECT_EQ("us", timer.getUnit());
    EXPECT_EQ("", timer.getName());

    TicTocTimer timer2(1000000, "someName");
    EXPECT_EQ(std::string("ms"), timer2.getUnit());
    EXPECT_EQ("someName", timer2.getName());

    TicTocTimer stimer(1000000000);
    EXPECT_EQ("s", stimer.getUnit());
  }

  /** test the tic toc facilities */
  TEST_F(TicTocTimerTest, testTicToc)
  {
    testTicStartsTiming();
    testTicResetsTocked();
    testTocStopsTiming();
  }

  void TicTocTimerTest::testConstructorStartsTiming()
  {
    high_resolution_clock::time_point before = high_resolution_clock::now();
    TicTocTimer timer;
    EXPECT_FALSE(timer.m_tocked);
    EXPECT_TRUE(timer.m_start < high_resolution_clock::now());
    EXPECT_TRUE(timer.m_start > before);
  }

  void TicTocTimerTest::testTicStartsTiming()
  {
    TicTocTimer timer;
    high_resolution_clock::time_point before = high_resolution_clock::now();
    timer.tic();
    EXPECT_FALSE(timer.m_tocked);
    EXPECT_TRUE(timer.m_start < high_resolution_clock::now());
    EXPECT_TRUE(timer.m_start > before);
  }

  void TicTocTimerTest::testTicResetsTocked()
  {
    TicTocTimer timer;
    timer.toc();
    ASSERT_TRUE(timer.m_tocked);
    timer.tic();
    EXPECT_FALSE(timer.m_tocked);
  }

  void TicTocTimerTest::testTocStopsTiming()
  {
    TicTocTimer timer;
    high_resolution_clock::time_point before = high_resolution_clock::now();
    timer.toc();
    EXPECT_TRUE(timer.m_tocked);
    EXPECT_TRUE(timer.m_end < high_resolution_clock::now());
    EXPECT_TRUE(timer.m_end > before);
  }
}
