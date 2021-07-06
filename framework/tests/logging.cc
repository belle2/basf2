/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/logging/Logger.h>
#include <framework/logging/LogMethod.h>
#include <framework/utilities/TestHelpers.h>
#include <framework/logging/LogVariableStream.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;

namespace {
  TEST(LoggingTest, B2ASSERT)
  {
#ifndef LOG_NO_B2ASSERT
    B2ASSERT("this shouldn't happen", true);
    // cppcheck-suppress internalAstError ; this confuses cppcheck ...
    EXPECT_B2FATAL(B2ASSERT("This is just a test, ignore this message", false));
#else
    B2ASSERT("this should be compiled out", true);
    B2ASSERT("this should be compiled out", false);
#endif

  }

  TEST(LoggingTest, LogMethod)
  {
    B2METHOD();
  }

  //mostly to test compilation when some logging macros are compiled out.
  TEST(LoggingTest, MacroSyntax)
  {
    B2DEBUG(100, "test");
    B2WARNING("test");

    //these only work if the macros themselves don't include ';' at the end
    if (true)
      B2WARNING("test");
    else { /* ... */ }

    if (true)
      B2DEBUG(100, "test");
    else { /* .. */ }

  }

  TEST(LoggingTest, VariableLogging)
  {
    LogVariableStream lv;
    lv << "Some Text" << endl << "which is constant" << LogVar("intVar", 23) << LogVar("floatVar", 3.14) << LogVar("strVar",
        "someString");
    EXPECT_EQ("Some Text\nwhich is constant\n\tintVar = 23\n\tfloatVar = 3.1400000000000001\n\tstrVar = someString", lv.str());

    // test if copy and assignment works as expected
    LogVariableStream lv_copyconst(lv);
    LogVariableStream lv_assign;
    lv_assign = lv;
    EXPECT_EQ("Some Text\nwhich is constant\n\tintVar = 23\n\tfloatVar = 3.1400000000000001\n\tstrVar = someString", lv_assign.str());
    EXPECT_EQ("Some Text\nwhich is constant\n\tintVar = 23\n\tfloatVar = 3.1400000000000001\n\tstrVar = someString",
              lv_copyconst.str());
  }

}  // namespace
