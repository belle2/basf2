/**
 * Author : Thomas Keck, Moritz Gelb
 * Date : July - 2016
*/

#include <framework/core/Environment.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/utilities/FileSystem.h>

#include <framework/logging/Logger.h>

#define BELLE_TARGET_H "belle-x86_64-unknown-linux-gnu-g++.h"
#include "belle_legacy/panther/panther.h"
#include "belle_legacy/panther/panther_group.h"
#include "belle_legacy/tables/belletdf.h"

#include <boost/program_options.hpp>
#include <iostream>
#include <cstdlib>
#include <algorithm>


using namespace std;
using namespace Belle2;
namespace po = boost::program_options;


int main(int argc, char* argv[])
{

  std::string filename;
  po::options_description description("Options");
  description.add_options()
  ("filename", po::value<std::string>(&filename), "belle 1 mdst file");
  po::variables_map vm;
  po::positional_options_description p;
  p.add("filename", -1);

  try {
    po::parsed_options parsed = po::command_line_parser(argc, argv).options(description).positional(p).run();
    po::store(parsed, vm);

    if (vm.count("help")) {
      std::cout << description << std::endl;
      return 1;
    }
    po::notify(vm);
  } catch (po::error& err) {
    std::cerr << "Error: " << err.what() << "\n";
    return 1;
  }

  // check environment
  const char* table_dir = getenv("PANTHER_TABLE_DIR");
  if (!table_dir or !FileSystem::isDir(table_dir)) {
    string fixed_table_dir = Environment::Instance().getExternalsPath() + "/share/belle_legacy/panther";
    B2WARNING("PANTHER_TABLE_DIR environment variable not set correctly. This is a known problem with externals v00-05-09, using " <<
              fixed_table_dir << " instead.");
    if (!FileSystem::isDir(fixed_table_dir))
      B2FATAL("Path " << fixed_table_dir << " does not exist, your externals setup seems broken.");
    setenv("PANTHER_TABLE_DIR", fixed_table_dir.c_str(), 1); //overwrite existing value
  }

  // Initialize Panther
  BsInit(0);
  // delete existing FileIO
  BsClrTab(BBS_CLEAR_ALL);
  // Open data file
  Belle::Panther_FileIO* fd = new Belle::Panther_FileIO(filename.c_str(), BBS_READ);

  // Read first record (does not contain event data)
  if (fd->read() == -1) {
    B2FATAL("Couldn't read file '" << filename << "'!");
  }

  unsigned int nevt = 0;

  // read event
  int rectype = -1;
  while (rectype > -2) {
    rectype = -1;
    while (rectype < 0 && rectype != -2) {
      //clear all previous event data before reading!
      BsClrTab(BBS_CLEAR);
      rectype = fd->read();
      if (rectype == -1) {
        B2ERROR("Error while reading panther tables! Record skipped.");
      }
      nevt++;
    }
  }

  delete fd;

  std::cout << static_cast<int>(nevt) - 2 << std::endl;
  if (rectype == -2) { // EoF detected
    return 0;
  }

}
