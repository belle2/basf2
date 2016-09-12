/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <hlt/softwaretrigger/core/SoftwareTriggerCut.h>
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {
  namespace SoftwareTrigger {

    /// Base class for the cut tests.
    class SoftwareTriggerCutTest : public ::testing::Test {
      /// Set the random seed to a known value.
      void SetUp()
      {
        m_savedSeed = gRandom->GetSeed();
        gRandom->SetSeed(42);
      }

      /// Reset the random seed.
      void TearDown()
      {
        gRandom->SetSeed(m_savedSeed);
      }

      /// Internal storage for the random seed before this test.
      unsigned int m_savedSeed = 0;
    };

    /** Test simple cuts without prescales. */
    TEST_F(SoftwareTriggerCutTest, basic)
    {
      SoftwareTriggerObject object;

      auto cut = SoftwareTriggerCut::compile("1 == 1", 1, false);
      EXPECT_EQ(SoftwareTriggerCutResult::c_accept, cut->checkPreScaled(object));

      cut = SoftwareTriggerCut::compile("1 == 1", 1, true);
      EXPECT_EQ(SoftwareTriggerCutResult::c_reject, cut->checkPreScaled(object));

      cut = SoftwareTriggerCut::compile("-1 == 1", 1, false);
      EXPECT_EQ(SoftwareTriggerCutResult::c_noResult, cut->checkPreScaled(object));

      cut = SoftwareTriggerCut::compile("-1 == 1", 1, true);
      EXPECT_EQ(SoftwareTriggerCutResult::c_noResult, cut->checkPreScaled(object));
    }

    /** Test simple cuts and prescales. */
    TEST_F(SoftwareTriggerCutTest, prescaling)
    {

      SoftwareTriggerObject object;

      // As a random seed is implemented in the software trigger cut, we test it multiple times.
      auto cut = SoftwareTriggerCut::compile("1 == 1", 0, false);
      for (unsigned int i = 0; i < 1e3; i++) {
        EXPECT_EQ(SoftwareTriggerCutResult::c_noResult, cut->checkPreScaled(object));
      }

      // For reject cuts, the prescale is not allowed to have any influence
      cut = SoftwareTriggerCut::compile("1 == 1", 0, true);
      for (unsigned int i = 0; i < 1e3; i++) {
        EXPECT_EQ(SoftwareTriggerCutResult::c_reject, cut->checkPreScaled(object));
      }

      cut = SoftwareTriggerCut::compile("1 == 1", 1, true);
      for (unsigned int i = 0; i < 1e3; i++) {
        EXPECT_EQ(SoftwareTriggerCutResult::c_reject, cut->checkPreScaled(object));
      }

      cut = SoftwareTriggerCut::compile("1 == 1", 1);
      for (unsigned int i = 0; i < 1e3; i++) {
        EXPECT_EQ(SoftwareTriggerCutResult::c_accept, cut->checkPreScaled(object));
      }
      // As we set a stable seed, we know already the result quite well
      cut = SoftwareTriggerCut::compile("1 == 1", 10);

      unsigned int numberOfYes = 0;
      unsigned int numberOfNo = 0;
      for (unsigned int i = 0; i < 1e3; i++) {
        const auto cutResult = cut->checkPreScaled(object);
        EXPECT_NE(SoftwareTriggerCutResult::c_reject, cutResult);

        if (cutResult == SoftwareTriggerCutResult::c_accept) {
          numberOfYes++;
        } else if (cutResult == SoftwareTriggerCutResult::c_noResult) {
          numberOfNo++;
        }
      }

      // More or less 1 out of 10...
      EXPECT_EQ(numberOfYes, 115);
      EXPECT_EQ(numberOfNo, 885);
    }
  }
}
