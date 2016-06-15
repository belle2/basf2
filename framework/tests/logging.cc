#include <framework/logging/Logger.h>
#include <framework/logging/LogMethod.h>
#include <framework/utilities/TestHelpers.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;

namespace {
  TEST(LoggingTest, B2ASSERT)
  {
#ifndef LOG_NO_B2ASSERT
    B2ASSERT("this shouldn't happen", true);
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
}  // namespace
