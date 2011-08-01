/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/FileHandler.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/core/Environment.h>
#include <framework/logging/Logger.h>

#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>

using namespace std;
namespace io = boost::iostreams;
namespace fs = boost::filesystem;

namespace Belle2 {
  namespace gearbox {

    FileContext::FileContext(const string& filename, bool compressed)
    {
      if (compressed) m_stream.push(io::gzip_decompressor());
      m_stream.push(io::file_source(filename));
    }

    FileHandler::FileHandler(const std::string& uri): InputHandler(uri), m_path(uri)
    {
      if (m_path.empty()) m_path = Environment::Instance().getDataSearchPath();
      m_path = fs::absolute(m_path, fs::initial_path<fs::path>()).native();
      B2DEBUG(300, "Created FileHandler for directory " << m_path);
    }

    InputContext* FileHandler::open(const std::string &path)
    {
      fs::path basedir(m_path);
      fs::path filename = basedir / path;
      if (fs::exists(filename)) return new FileContext(filename.string(), false);
      filename = basedir / (path + ".gz");
      if (fs::exists(filename)) return new FileContext(filename.string(), true);
      return 0;
    }

    B2_GEARBOX_REGISTER_INPUTHANDLER(FileHandler, "file");
  }
}
