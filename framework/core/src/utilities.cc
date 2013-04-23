/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/utilities.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <boost/filesystem.hpp>
#include <sys/time.h>
#include <dlfcn.h>
#include <sstream>
#include <cstdlib>

using namespace std;
namespace fs = boost::filesystem;

namespace Belle2 {
  namespace FileSystem {
    bool fileExists(const string& filename)
    {
      fs::path fullPath = fs::absolute(filename, fs::initial_path<fs::path>());
      return fs::exists(fullPath);
    }

    bool fileDirExists(const string& filename)
    {
      fs::path fullPath = fs::absolute(filename, fs::initial_path<fs::path>());
      fullPath.remove_filename();
      return fs::exists(fullPath);
    }

    bool isFile(const string& filename)
    {
      fs::path fullPath = fs::absolute(filename, fs::initial_path<fs::path>());
      return (fs::exists(fullPath)) && (fs::is_regular_file(fullPath));
    }

    bool isDir(const string& filename)
    {
      fs::path fullPath = fs::absolute(filename, fs::initial_path<fs::path>());
      return (fs::exists(fullPath)) && (fs::is_directory(fullPath));
    }

    bool loadLibrary(std::string library, bool fullname)
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

    std::string findFile(const string& path)
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
      //nothing found
      else
        return string("");
    }
  }

  namespace Utils {

    double getClock()
    {
      timespec ts;
      clock_gettime(CLOCK_REALTIME, &ts);
      return (ts.tv_sec * Unit::s) + (ts.tv_nsec * Unit::ns);
    }

  }
} // Belle2 namespace

