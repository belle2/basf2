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
#include <boost/algorithm/string.hpp>

#include <chrono>
#include <random>
#include <cstring>

//dlopen etc.
#include <dlfcn.h>
#include <fcntl.h>

#include <TMD5.h>
#include <zlib.h>

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

bool FileSystem::isSymLink(const string& filename)
{
  fs::path fullPath = fs::absolute(filename);
  return (fs::exists(fullPath)) && (fs::is_symlink(fullPath));
}

bool FileSystem::loadLibrary(std::string library, bool fullname)
{
  if (!fullname) library = "lib" + library + ".so";

  B2DEBUG(100, "Loading shared library " << library);
  void* libPointer = dlopen(library.c_str() , RTLD_LAZY | RTLD_GLOBAL);

  if (libPointer == nullptr) {
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

std::string FileSystem::calculateAdler32(const std::string& filename)
{
  string chksum;
  if (not isFile(filename)) return "";
  fs::path fullPath = fs::absolute(filename);
  FILE* fp = fopen(fullPath.c_str(), "rb");
  if (fp) {
    uLong i, sum = adler32(0, 0, 0);
    char hexdigest[9];
    Bytef* buf = (Bytef*) malloc(1024 * 1024 * sizeof(Bytef));
    if (!buf) {
      fclose(fp);
      return "";
    }
    while ((i = fread((void*) buf, 1, sizeof(buf), fp)) > 0) {
      sum = adler32(sum, buf, i);
    }
    fclose(fp);
    free(buf);
    // Adler32 checksums hex digests ARE zero padded although
    // HLT legacy presentation may differ.
    sprintf(hexdigest, "%08lx", sum);
    chksum = hexdigest;
  } else {
    chksum = "";
  }
  return chksum;
}

std::string FileSystem::findFile(const string& path, const std::vector<std::string>& dirs, bool silent)
{
  // check given directories
  string fullpath;
  for (auto dir : dirs) {
    if (dir.empty())
      continue;
    fullpath = (fs::path(dir) / path).string();
    if (fileExists(fullpath)) {
      if (isSymLink(fullpath) or isSymLink(dir))
        return fullpath;
      else
        return fs::canonical(fullpath).string();
    }
  }

  // check local directory
  fullpath = fs::absolute(path).string();
  if (fileExists(fullpath)) {
    if (isSymLink(fullpath))
      return fullpath;
    else
      return fs::canonical(fullpath).string();
  }

  // nothing found
  if (!silent)
    B2ERROR("findFile(): Could not find file." << LogVar("path", path));
  return string("");
}

std::string FileSystem::findFile(const string& path, bool silent)
{
  std::vector<std::string> dirs;
  if (getenv("BELLE2_LOCAL_DIR")) {
    dirs.emplace_back(getenv("BELLE2_LOCAL_DIR"));
  }
  if (getenv("BELLE2_RELEASE_DIR")) {
    dirs.emplace_back(getenv("BELLE2_RELEASE_DIR"));
  }
  return findFile(path, dirs, silent);
}

std::string FileSystem::findFile(const string& path, const std::string& dataType, bool silent)
{
  std::vector<std::string> dirs;
  std::string envVar = "BELLE2_" + boost::to_upper_copy(dataType) + "_DATA_DIR";
  if (getenv(envVar.c_str())) {
    dirs.emplace_back(getenv(envVar.c_str()));
  }
  std::string result = findFile(path, dirs, true);
  if (result.empty() && !silent)
    B2ERROR("findFile(): Could not find data file. You may want to use the 'b2install-data' tool to get the file."
            << LogVar("path", path) << LogVar("data type", dataType));
  return result;
}

FileSystem::Lock::Lock(const std::string& fileName, bool readonly) :
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
