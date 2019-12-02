/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giacomo De Pietro                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle 2 headers. */
#include <framework/utilities/FileSystem.h>

/* C++ headers. */
#include <cstdlib>
#include <iostream>

/**
 * Wrapper for klm/examples/dqm/TestDQM.py steering file.
 *
 * Create the KLM DQM plots from the given input files.
 */
int main(int argc, char* argv[])
{
  if (argc < 2 or std::string(argv[1]) == "--help") {
    std::cerr << "Usage:\n\n"
              "  " << argv[0] << " <inputFiles>\n\n"
              "If raw data are used as input, please use 'raw' as first argument:\n\n"
              "  " << argv[0] << " raw <inputFiles>\n\n"
              "Note that it is possible to use the wildcard '*' to pass multiple files in input.\n";
    return 1;
  }
  std::string fullPath = Belle2::FileSystem::findFile("/klm/examples/dqm/TestDQM.py");
  std::string s = "basf2 " + fullPath;
  for (int i = 1; i < argc; ++i)
    s += " " + std::string(argv[i]);
  return system(s.c_str());
}
