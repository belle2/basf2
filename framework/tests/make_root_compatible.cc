/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/utilities/TestHelpers.h>
#include <framework/utilities/MakeROOTCompatible.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;

namespace {

  TEST(VariableTest, makeROOTCompatible)
  {
    EXPECT_EQ("", makeROOTCompatible(""));
    EXPECT_EQ("test", makeROOTCompatible("test"));
    EXPECT_EQ("test213", makeROOTCompatible("test213"));
    EXPECT_EQ("test_23", makeROOTCompatible("test_23"));

    EXPECT_EQ("", makeROOTCompatible(""));
    EXPECT_EQ("test", makeROOTCompatible("test"));
    EXPECT_EQ("test213", makeROOTCompatible("test213"));
    EXPECT_EQ("test_23", makeROOTCompatible("test_23"));

    EXPECT_EQ("test__bo23__bc", makeROOTCompatible("test(23)"));
    EXPECT_EQ("test__so23__sc", makeROOTCompatible("test[23]"));
    EXPECT_EQ("test__co23__cc", makeROOTCompatible("test{23}"));
    EXPECT_EQ("test__boe__pl__clgamma__cm__spM__sp__st__sp3__bc", makeROOTCompatible("test(e+:gamma, M < 3)"));
    EXPECT_EQ("__cl__bo__bc__sp__bo__bc__cm", makeROOTCompatible(":() (),"));
    EXPECT_EQ("c__cl__bo__bcb__sp__bo__bc__cma", makeROOTCompatible("c:()b (),a"));
    EXPECT_EQ("c__cl__bo__cm__pl8951aBZWVZUEOH_Rhtnsq__bcb__sp__bo__bc__cma", makeROOTCompatible("c:(,+8951aBZWVZUEOH_Rhtnsq)b (),a"));
    EXPECT_EQ("foo__bobar__cm__mi0__pt123__cm__sp94__bc", makeROOTCompatible("foo(bar,-0.123, 94)"));

    for (const auto& pair : getSubstitutionMap()) {
      EXPECT_EQ(pair.second, makeROOTCompatible(pair.first));
    }

    EXPECT_NO_B2WARNING(makeROOTCompatible("test_a"));
    EXPECT_B2WARNING(makeROOTCompatible("test__a"));
  }

  TEST(VariableTest, invertMakeROOTCompatible)
  {
    EXPECT_EQ(invertMakeROOTCompatible(""), "");
    EXPECT_EQ(invertMakeROOTCompatible("test"), "test");
    EXPECT_EQ(invertMakeROOTCompatible("test213"), "test213");
    EXPECT_EQ(invertMakeROOTCompatible("test_23"), "test_23");

    EXPECT_EQ(invertMakeROOTCompatible(""), "");
    EXPECT_EQ(invertMakeROOTCompatible("test"), "test");
    EXPECT_EQ(invertMakeROOTCompatible("test213"), "test213");
    EXPECT_EQ(invertMakeROOTCompatible("test_23"), "test_23");

    EXPECT_EQ(invertMakeROOTCompatible("test__bo23__bc"), "test(23)");
    EXPECT_EQ(invertMakeROOTCompatible("test__so23__sc"), "test[23]");
    EXPECT_EQ(invertMakeROOTCompatible("test__co23__cc"), "test{23}");
    EXPECT_EQ(invertMakeROOTCompatible("test__boe__pl__clgamma__cm__spM__sp__st__sp3__bc"), "test(e+:gamma, M < 3)");
    EXPECT_EQ(invertMakeROOTCompatible("__cl__bo__bc__sp__bo__bc__cm"), ":() (),");
    EXPECT_EQ(invertMakeROOTCompatible("c__cl__bo__bcb__sp__bo__bc__cma"), "c:()b (),a");
    EXPECT_EQ(invertMakeROOTCompatible("c__cl__bo__cm__pl8951aBZWVZUEOH_Rhtnsq__bcb__sp__bo__bc__cma"),
              "c:(,+8951aBZWVZUEOH_Rhtnsq)b (),a");
    EXPECT_EQ(invertMakeROOTCompatible("foo__bobar__cm__mi0__pt123__cm__sp94__bc"), "foo(bar,-0.123, 94)");

    for (const auto& pair : getSubstitutionMap()) {
      EXPECT_EQ(pair.first, invertMakeROOTCompatible(pair.second));
    }
  }

}
