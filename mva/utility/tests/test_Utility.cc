/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/utility/Utility.h>
#include <framework/utilities/TestHelpers.h>
#include <framework/database/Configuration.h>
#include <framework/database/Database.h>

using namespace Belle2;

namespace {

  TEST(AvailableTest, Available)
  {
    TestHelpers::TempDirCreator tmp_dir;

    auto& conf = Conditions::Configuration::getInstance();
    conf.overrideGlobalTags();
    conf.prependTestingPayloadLocation("localdb/database.txt");

    MVA::Weightfile weightfile;
    MVA::Weightfile::save(weightfile, "MVAInterfaceTest.root");
    MVA::Weightfile::save(weightfile, "MVAInterfaceTest.xml");
    MVA::Weightfile::save(weightfile, "MVAInterfaceTest");

    EXPECT_FALSE(MVA::available("DOES_NOT_EXIST.root"));
    EXPECT_FALSE(MVA::available("DOES_NOT_EXIST.xml"));
    EXPECT_FALSE(MVA::available("DOES_NOT_EXIST"));
    EXPECT_TRUE(MVA::available("MVAInterfaceTest.root"));
    EXPECT_TRUE(MVA::available("MVAInterfaceTest.xml"));
    EXPECT_TRUE(MVA::available("MVAInterfaceTest"));

    Database::reset();
  }

}

