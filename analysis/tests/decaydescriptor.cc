/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam Cunliffe                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <framework/logging/Logger.h>
#include <framework/utilities/TestHelpers.h>

#include <gtest/gtest.h>
#include <string>

using namespace Belle2;

namespace {
  TEST(DecayDescriptorTest, TrivialUse)
  {
    // trivial decay descriptor == particle name
    DecayDescriptor dd;
    bool initok = dd.init("K+");
    EXPECT_EQ(initok, true);
    ASSERT_NE(dd.getMother(), nullptr);
    EXPECT_EQ(dd.getMother()->getName(), "K+");
    EXPECT_EQ(dd.getMother()->getLabel(), "");
    EXPECT_EQ(dd.getNDaughters(), 0);
  }

  TEST(DecayDescriptorTest, NormalBehaviour)
  {
    DecayDescriptor dd;
    bool initok =
      dd.init("B0:cand -> K+:loose pi-:loose");

    EXPECT_EQ(initok, true);
    ASSERT_NE(dd.getMother(), nullptr);
    EXPECT_EQ(dd.getMother()->getName(), "B0");
    EXPECT_EQ(dd.getMother()->getLabel(), "cand");
    EXPECT_EQ(dd.getNDaughters(), 2);

    EXPECT_EQ(dd.getDaughter(0)->getNDaughters(), 0);
    EXPECT_EQ(dd.getDaughter(0)->getMother()->getName(), "K+");
    EXPECT_EQ(dd.getDaughter(0)->getMother()->getLabel(), "loose");

    EXPECT_EQ(dd.getDaughter(1)->getNDaughters(), 0);
    EXPECT_EQ(dd.getDaughter(1)->getMother()->getName(), "pi-");
    EXPECT_EQ(dd.getDaughter(1)->getMother()->getLabel(), "loose");

    ASSERT_EQ(dd.getDaughter(2), nullptr);
  }

  TEST(DecayDescriptorTest, BadLabelTest)
  {
    // use of illegal characters
    DecayDescriptor dd;
    bool initok = dd.init("B0:p3rvct,ly[name -> K+:loose pi-:loose");
    EXPECT_EQ(initok, false);
    EXPECT_EQ(dd.getMother()->getName(), "");
    EXPECT_EQ(dd.getMother()->getLabel(), "");
  }

  TEST(DecayDescriptorTest, BadGrammarTest)
  {
    // test bad decay descriptor grammar
  }
}
