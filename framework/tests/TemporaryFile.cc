#include <framework/utilities/FileSystem.h>
#include <boost/filesystem.hpp>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;
namespace fs = boost::filesystem;

namespace {
  TEST(Utilities, TemporaryFile)
  {
    std::string filename;
    {
      FileSystem::TemporaryFile temp;
      filename = temp.getName();
      ASSERT_TRUE(temp.is_open());
      ASSERT_TRUE(fs::exists(filename));
    }
    ASSERT_FALSE(fs::exists(filename));
  }
}  // namespace
