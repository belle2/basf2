/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/utilities/FileSystem.h>
#include <framework/utilities/ScopeGuard.h>
#include <boost/filesystem.hpp>

#include <gtest/gtest.h>

#include <sys/wait.h>

#include <unistd.h>

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

      {
        //check relative filenames
        auto pwd = ScopeGuard::guardWorkingDirectory("/tmp"); //don't influence other tests by changing pwd
        std::string relname = fs::relative(filename, "/tmp").string();
        ASSERT_TRUE(FileSystem::fileExists(relname));
        chdir("/");
        ASSERT_FALSE(FileSystem::fileExists(relname));
      }
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

    if (pid_t pid = fork()) {
      //mother process

      //NOTE: in same process, the lock is NOT exclusive!
      EXPECT_TRUE(lk.lock(0));
      int status = 0;
      waitpid(pid, &status, 0);
      EXPECT_TRUE(WIFEXITED(status));
      EXPECT_EQ(0, WEXITSTATUS(status));
    } else {
      //child process
      //do not use an ASSERT_TRUE or something here, or we'll run tests twice
      if (lk.lock(0))
        exit(1);

      FileSystem::Lock lk2(filename);
      if (lk2.lock(0))
        exit(1);

      exit(0);
    }
  }
}  // namespace
