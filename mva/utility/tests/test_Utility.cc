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

using namespace Belle2;

namespace {

  TEST(AvailableTest, Available)
  {
    TestHelpers::TempDirCreator tmp_dir;

    EXPECT_FALSE(MVA::available("DOES_NOT_EXIST.root"));
    EXPECT_FALSE(MVA::available("DOES_NOT_EXIST.xml"));
    EXPECT_FALSE(MVA::available("DOES_NOT_EXIST"));

    MVA::Weightfile weightfile;
    MVA::Weightfile::save(weightfile, "MVAInterfaceTest.root");
    MVA::Weightfile::save(weightfile, "MVAInterfaceTest.xml");
    MVA::Weightfile::save(weightfile, "MVAInterfaceTest");

    EXPECT_TRUE(MVA::available("MVAInterfaceTest.root"));
    EXPECT_TRUE(MVA::available("MVAInterfaceTest.xml"));
    EXPECT_TRUE(MVA::available("MVAInterfaceTest"));

  }

}

