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

    {
      //multiple readonly locks ok
      FileSystem::Lock ro(filename, true);
      EXPECT_TRUE(ro.lock(1));
      EXPECT_TRUE(ro.lock(1));
      EXPECT_TRUE(ro.lock(1));
      EXPECT_TRUE(ro.lock(1));

      FileSystem::Lock rw(filename, false);
      EXPECT_FALSE(rw.lock(1));
    }

    {
      FileSystem::Lock rw(filename, false);
      EXPECT_TRUE(rw.lock(0));
      EXPECT_FALSE(rw.lock(0));
    }
  }
}  // namespace
