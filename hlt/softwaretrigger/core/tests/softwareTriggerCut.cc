/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <hlt/softwaretrigger/core/SoftwareTriggerCut.h>
#include <gtest/gtest.h>
#include <stdint.h>

using namespace std;

namespace Belle2 {
  namespace SoftwareTrigger {

    /// Base class for the cut tests.
    class SoftwareTriggerCutTest : public ::testing::Test {};

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

    /** Test simple cuts and prescales with random numbers. */
    TEST_F(SoftwareTriggerCutTest, prescalingRandom)
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
      for (unsigned int i = 0; i < 1e4; i++) {
        const auto cutResult = cut->checkPreScaled(object);
        EXPECT_NE(SoftwareTriggerCutResult::c_reject, cutResult);

        if (cutResult == SoftwareTriggerCutResult::c_accept) {
          numberOfYes++;
        } else if (cutResult == SoftwareTriggerCutResult::c_noResult) {
          numberOfNo++;
        }
      }

      // More or less 1 out of 10...
      EXPECT_EQ(numberOfYes, 1022);
      EXPECT_EQ(numberOfNo, 1e4 - 1022);
    }

    /** Test simple cuts and prescales with internal counters. */
    TEST_F(SoftwareTriggerCutTest, prescalingCounter)
    {

      SoftwareTriggerObject object;

      // We test it multiple times, just to be sure.
      auto cut = SoftwareTriggerCut::compile("1 == 1", 0, false);
      for (uint32_t i = 0; i < 1e3; i++) {
        EXPECT_EQ(SoftwareTriggerCutResult::c_noResult, cut->check(object).first);
      }

      // For reject cuts, the prescale is not allowed to have any influence
      cut = SoftwareTriggerCut::compile("1 == 1", 0, true);
      for (uint32_t i = 0; i < 1e3; i++) {
        EXPECT_EQ(SoftwareTriggerCutResult::c_reject, cut->check(object).first);
      }

      cut = SoftwareTriggerCut::compile("1 == 1", 1, true);
      for (uint32_t i = 0; i < 1e3; i++) {
        EXPECT_EQ(SoftwareTriggerCutResult::c_reject, cut->check(object).first);
      }

      cut = SoftwareTriggerCut::compile("1 == 1", 1);
      for (uint32_t i = 0; i < 1e3; i++) {
        EXPECT_EQ(SoftwareTriggerCutResult::c_accept, cut->check(object).first);
      }

      // Now let's test the counters.
      cut = SoftwareTriggerCut::compile("1 == 1", 10);
      uint32_t counter = 0;
      uint32_t numberOfYes = 0;
      uint32_t numberOfNo = 0;
      // Since the counter starts with 0, we expect (729/10)+1 yes.
      for (uint32_t i = 0; i < 729; i++) {
        const auto cutResult = cut->check(object, &counter).first;
        EXPECT_NE(SoftwareTriggerCutResult::c_reject, cutResult);
        if (cutResult == SoftwareTriggerCutResult::c_accept) {
          numberOfYes++;
        } else if (cutResult == SoftwareTriggerCutResult::c_noResult) {
          numberOfNo++;
        }
      }
      uint32_t expectedYes = (729 / 10) + 1;
      EXPECT_EQ(numberOfYes, expectedYes);
      EXPECT_EQ(numberOfNo, 729 - expectedYes);

      cut = SoftwareTriggerCut::compile("1 == 1", 7);
      counter = 1;
      numberOfYes = 0;
      numberOfNo = 0;
      // Now the counter starts with 1, so we expect (544/7) yes.
      for (uint32_t i = 1; i <= 544; i++) {
        const auto cutResult = cut->check(object, &counter).first;
        EXPECT_NE(SoftwareTriggerCutResult::c_reject, cutResult);
        if (cutResult == SoftwareTriggerCutResult::c_accept) {
          numberOfYes++;
        } else if (cutResult == SoftwareTriggerCutResult::c_noResult) {
          numberOfNo++;
        }
      }
      expectedYes = (544 / 7);
      EXPECT_EQ(numberOfYes, expectedYes);
      EXPECT_EQ(numberOfNo, 544 - expectedYes);
    }
  }
}
