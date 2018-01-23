/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: CDC group                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/utilities/FileSystem.h>

#include <boost/iostreams/filtering_stream.hpp>

#include <string>
#include <fstream>

namespace Belle2 {
  namespace CDC {
    //TODO: merge the following two functions and rename to openFile
    /**
     * Open a file
     * @param[in] ifs input file-stream
     * @param[in] fileName0 file-name on cdc/data directory
     */
    void openFileA(std::ifstream& ifs, const std::string& fileName0);

    /**
     * Open a file using boost (to be able to read a gzipped file)
     * @param[in] ifs input file-stream
     * @param[in] fileName0 file-name on cdc/data directory
     */
    void openFileB(boost::iostreams::filtering_istream& ifs, const std::string& fileName0);

  } // end of namespace CDC
} // end of namespace Belle2
