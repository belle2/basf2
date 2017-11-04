/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: CDC group                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
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
      std::string fileName1 = "/cdc/data/" + fileName0;
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
      std::string fileName1 = "/cdc/data/" + fileName0;
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
