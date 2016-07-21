/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <skim/softwaretrigger/core/SoftwareTriggerCut.h>
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {
  namespace SoftwareTrigger {

    class SoftwareTriggerCutTest : public ::testing::Test {
      void SetUp()
      {
        m_savedSeed = gRandom->GetSeed();
        gRandom->SetSeed(42);
      }

      void TearDown()
      {
        gRandom->SetSeed(m_savedSeed);
      }

      unsigned int m_savedSeed = 0;
    };

    /** Test simple cuts and prescales. */
    TEST_F(SoftwareTriggerCutTest, prescaling)
    {

      SoftwareTriggerObject object;

      // As a random seed is implemented in the software trigger cut, we test it multiple times.
      auto cut = SoftwareTriggerCut::compile("1 == 1", 0);
      for (unsigned int i = 0; i < 1e3; i++) {
        EXPECT_FALSE(cut->checkPreScaled(object));
      }

      cut = SoftwareTriggerCut::compile("1 == 1", 1);
      for (unsigned int i = 0; i < 1e3; i++) {
        EXPECT_TRUE(cut->checkPreScaled(object));
      }

      // Test default value
      cut = SoftwareTriggerCut::compile("1 == 1");
      for (unsigned int i = 0; i < 1e3; i++) {
        EXPECT_TRUE(cut->checkPreScaled(object));
      }

      // As we set a stable seed, we know already the result quite well
      cut = SoftwareTriggerCut::compile("1 == 1", 10);

      unsigned int numberOfYes = 0;
      unsigned int numberOfNo = 0;
      for (unsigned int i = 0; i < 1e3; i++) {
        if (cut->checkPreScaled(object)) {
          numberOfYes++;
        } else {
          numberOfNo++;
        }
      }

      // More or less 1 out of 10...
      EXPECT_EQ(numberOfYes, 115);
      EXPECT_EQ(numberOfNo, 885);
    }
  }
}
