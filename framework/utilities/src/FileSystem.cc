/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013-2018 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher, Thomas Kuhr, Martin Ritter       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/utilities/FileSystem.h>

#include <framework/logging/Logger.h>

#include <boost/filesystem.hpp>

#include <chrono>
#include <random>
#include <cstring>

//dlopen etc.
#include <dlfcn.h>
#include <fcntl.h>

#include <TMD5.h>

using namespace std;
using namespace Belle2;
namespace fs = boost::filesystem;

bool FileSystem::fileExists(const string& filename)
{
  fs::path fullPath = fs::absolute(filename);
  return fs::exists(fullPath);
}

bool FileSystem::fileDirExists(const string& filename)
{
  fs::path fullPath = fs::absolute(filename);
  fullPath.remove_filename();
  return fs::exists(fullPath);
}

bool FileSystem::isFile(const string& filename)
{
  fs::path fullPath = fs::absolute(filename);
  return (fs::exists(fullPath)) && (fs::is_regular_file(fullPath));
}

bool FileSystem::isDir(const string& filename)
{
  fs::path fullPath = fs::absolute(filename);
  return (fs::exists(fullPath)) && (fs::is_directory(fullPath));
}

bool FileSystem::loadLibrary(std::string library, bool fullname)
{
  if (!fullname) library = "lib" + library + ".so";

  B2DEBUG(100, "Loading shared library " << library);
  void* libPointer = dlopen(library.c_str() , RTLD_LAZY | RTLD_GLOBAL);

  if (libPointer == NULL) {
    B2ERROR("Could not open shared library file (error in dlopen) : " << dlerror());
    return false;
  }

  return true;
}

std::string FileSystem::calculateMD5(const std::string& filename)
{
  if (not isFile(filename)) return "";
  fs::path fullPath = fs::absolute(filename);
  std::unique_ptr<TMD5> md5(TMD5::FileChecksum(fullPath.c_str()));
  return md5->AsString();
}

std::string FileSystem::findFile(const string& path, bool silent)
{
  //environment doesn't change, so only done once
  static const char* localdir = getenv("BELLE2_LOCAL_DIR");
  static const char* reldir = getenv("BELLE2_RELEASE_DIR");

  //check in local directory
  string fullpath;
  if (localdir and fileExists(fullpath = (fs::path(localdir) / path).string()))
    return fullpath;
  //check in central release directory
  else if (reldir and fileExists(fullpath = (fs::path(reldir) / path).string()))
    return fullpath;
  //check if this thing exists as normal path (absolute / relative to PWD)
  else if (fileExists(fullpath = (fs::absolute(path).string())))
    return fullpath;

  //nothing found
  if (!silent)
    B2ERROR("findFile(): Could not find '" << path << "'!");
  return string("");
}


FileSystem::Lock::Lock(std::string fileName, bool readonly) :
  m_readOnly(readonly)
{
  const int mode = readonly ? O_RDONLY : O_RDWR;
  m_file = open(fileName.c_str(), mode | O_CREAT, 0640);
}

FileSystem::Lock::~Lock()
{
  if (m_file >= 0) close(m_file);
}

bool FileSystem::Lock::lock(int timeout, bool ignoreErrors)
{
  if (m_file < 0) return false;

  auto const maxtime = std::chrono::steady_clock::now() + std::chrono::seconds(timeout);
  std::default_random_engine random;
  std::uniform_int_distribution<int> uniform(1, 100);

  /* Note:
   * Previously, this used flock(), which doesn't work with GPFS.
   * fcntl() does, and also should be more likely to work on NFS.
   * If you use the 'nolock' mount option to NFS, you are on your own.
   */
  struct flock fl;
  memset(&fl, '\0', sizeof(fl));
  fl.l_type = m_readOnly ? F_RDLCK : F_WRLCK;
  //lock entire file
  fl.l_whence = SEEK_SET;
  fl.l_start = 0;
  fl.l_len = 0;

  while (true) {
    int lock = fcntl(m_file, F_SETLK, &fl);
    if (lock == 0)
      return true;
    else if (std::chrono::steady_clock::now() > maxtime)
      break;
    if (errno != EAGAIN && errno != EACCES && errno != EINTR) break;
    usleep(uniform(random) * 1000);
  }
  if (!ignoreErrors) B2ERROR("Locking failed: " << strerror(errno));
  return false;
}

FileSystem::TemporaryFile::TemporaryFile(std::ios_base::openmode mode): std::fstream()
{
  fs::path filename = fs::temp_directory_path() / fs::unique_path();
  m_filename = filename.native();
  open(m_filename.c_str(), mode);
  if (!is_open()) {
    B2ERROR("Cannot create temporary file: " << strerror(errno));
  }
}

FileSystem::TemporaryFile::~TemporaryFile()
{
  close();
  fs::remove(m_filename);
}
