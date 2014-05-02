#include <framework/logging/Logger.h>
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
}  // namespace
