/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
#include <cdc/utilities/OpenFile.h>

#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>

using namespace std;
using namespace boost;

namespace Belle2 {
  namespace CDC {
// Open a file
    void openFileA(std::ifstream& ifs, const std::string& fileName0)
    {
      std::string fileName1 = "/data/cdc/" + fileName0;
      std::string fileName = FileSystem::findFile(fileName1, true);

      if (fileName == "") {
        fileName = FileSystem::findFile(fileName0, true);
      }

      if (fileName == "") {
        B2FATAL("CDC::openFile: " << fileName0 << " not exist!");
      } else {
        B2INFO("CDC::openFile: open " << fileName);
        ifs.open(fileName.c_str());
        if (!ifs) B2FATAL("CDC::openFile: cannot open " << fileName << " !");
      }
    }

// Open a file using boost (to be able to read a gzipped file)
    void openFileB(boost::iostreams::filtering_istream& ifs, const std::string& fileName0)
    {
      std::string fileName1 = "/data/cdc/" + fileName0;
      std::string fileName = FileSystem::findFile(fileName1, true);

      if (fileName == "") {
        fileName = FileSystem::findFile(fileName0, true);
      }

      if (fileName == "") {
        B2FATAL("CDC::openFile: " << fileName0 << " not exist!");
      } else {
        B2INFO("CDC::openFile: open " << fileName);
        if ((fileName.rfind(".gz") != string::npos) && (fileName.length() - fileName.rfind(".gz") == 3)) {
          ifs.push(boost::iostreams::gzip_decompressor());
        }
        ifs.push(boost::iostreams::file_source(fileName));
        if (!ifs) B2FATAL("CDC::openFile: cannot open " << fileName << " !");
      }
    }
  }
}
