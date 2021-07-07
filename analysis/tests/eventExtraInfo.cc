/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
