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
  TEST(Utilities, FileSystem)
  {
    std::string filename;
    {
      FileSystem::TemporaryFile temp;
      filename = temp.getName();
      ASSERT_TRUE(FileSystem::fileExists(filename));
      ASSERT_FALSE(FileSystem::isDir(filename));
      ASSERT_TRUE(FileSystem::fileDirExists(filename));
    }
    ASSERT_FALSE(FileSystem::fileExists(filename));
    ASSERT_FALSE(FileSystem::isDir(filename));
    ASSERT_TRUE(FileSystem::fileDirExists(filename));

    ASSERT_TRUE(FileSystem::isDir("/"));
  }
  TEST(Utilities, Lock)
  {
    FileSystem::TemporaryFile temp;
    std::string filename = temp.getName();

    //check 0-timeout locks work
    FileSystem::Lock lk(filename);
    EXPECT_TRUE(lk.lock(0));
    //NOTE: in same process, the lock is NOT exclusive!
    EXPECT_TRUE(lk.lock(0));
  }
}  // namespace
