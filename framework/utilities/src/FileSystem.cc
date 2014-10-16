/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/utilities/FileSystem.h>

#include <framework/logging/Logger.h>

#include <boost/filesystem.hpp>

//dlopen etc.
#include <dlfcn.h>

using namespace std;
using namespace Belle2;
namespace fs = boost::filesystem;

bool FileSystem::fileExists(const string& filename)
{
  fs::path fullPath = fs::absolute(filename, fs::initial_path<fs::path>());
  return fs::exists(fullPath);
}

bool FileSystem::fileDirExists(const string& filename)
{
  fs::path fullPath = fs::absolute(filename, fs::initial_path<fs::path>());
  fullPath.remove_filename();
  return fs::exists(fullPath);
}

bool FileSystem::isFile(const string& filename)
{
  fs::path fullPath = fs::absolute(filename, fs::initial_path<fs::path>());
  return (fs::exists(fullPath)) && (fs::is_regular_file(fullPath));
}

bool FileSystem::isDir(const string& filename)
{
  fs::path fullPath = fs::absolute(filename, fs::initial_path<fs::path>());
  return (fs::exists(fullPath)) && (fs::is_directory(fullPath));
}

bool FileSystem::loadLibrary(std::string library, bool fullname)
{
  if (!fullname) library = "lib" + library + ".so";

  B2DEBUG(1, "Loading shared library " << library);
  void* libPointer = dlopen(library.c_str() , RTLD_LAZY | RTLD_GLOBAL);

  if (libPointer == NULL) {
    B2ERROR("Could not open shared library file (error in dlopen) : " + string(dlerror()));
    return false;
  }

  return true;
}

std::string FileSystem::findFile(const string& path)
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
  else
    return string("");
}
