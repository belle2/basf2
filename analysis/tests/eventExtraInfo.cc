/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dataobjects/EventExtraInfo.h>

#include <gtest/gtest.h>

#include <stdexcept>

using namespace std;
using namespace Belle2;

TEST(EventExtraInfoTest, GetAddHasTest)
{

  EventExtraInfo eventExtraInfo;
  EXPECT_FALSE(eventExtraInfo.hasExtraInfo("a"));
  EXPECT_THROW(eventExtraInfo.getExtraInfo("a"), std::out_of_range);
  EXPECT_NO_THROW(eventExtraInfo.addExtraInfo("a", 3));
  EXPECT_FLOAT_EQ(eventExtraInfo.getExtraInfo("a"), 3);
  EXPECT_TRUE(eventExtraInfo.hasExtraInfo("a"));
  EXPECT_THROW(eventExtraInfo.addExtraInfo("a", 4), std::out_of_range);
  EXPECT_FLOAT_EQ(eventExtraInfo.getExtraInfo("a"), 3);

  EXPECT_FALSE(eventExtraInfo.hasExtraInfo("b"));
  EXPECT_THROW(eventExtraInfo.getExtraInfo("b"), std::out_of_range);
  EXPECT_NO_THROW(eventExtraInfo.addExtraInfo("b", 3.7));
  EXPECT_FLOAT_EQ(eventExtraInfo.getExtraInfo("b"), 3.7);
  EXPECT_TRUE(eventExtraInfo.hasExtraInfo("b"));
  EXPECT_THROW(eventExtraInfo.addExtraInfo("b", 4.6), std::out_of_range);
  EXPECT_FLOAT_EQ(eventExtraInfo.getExtraInfo("b"), 3.7);

  EXPECT_FLOAT_EQ(eventExtraInfo.getExtraInfo("a"), 3);
}
