/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Mikhail Remnev    mikhail.a.remnev@gmail.com             *
 *                                                                        *
 * This tool writes generates ECLChannelMap payload from                  *
 *  ecl/data/ecl_channels_map.txt                                         *
 * and then writes it to the database.                                    *
 *                                                                        *
 * Code is mostly based on Chris Hearty's eclElectronicPayloads           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/database/DBImportObjPtr.h>
#include <framework/database/LocalDatabase.h>
#include <framework/database/DatabaseChain.h>
#include <framework/logging/LogSystem.h>
#include <framework/utilities/FileSystem.h>
// ecl
#include <ecl/dbobjects/ECLChannelMap.h>
#include <ecl/utility/ECLChannelMapper.h>
// std
#include <iostream>

using namespace Belle2;
using namespace ECL;

//------------------------------------------------------------------------
int main(int argc, char** argv)
{
  if (argc < 2) {
    std::cout << "Usage: eclWriteChannelMap experiment run" << std::endl;
    return -1;
  }
  int experiment = std::stoi(argv[1]);
  int run = std::stoi(argv[2]);

  //------------------------------------------------------------------------
  //..Specify database
  DatabaseChain::createInstance();
  LocalDatabase::createInstance("localdb/database.txt", "", LogConfig::c_Debug);

  //..set debug level
  LogConfig* logging = LogSystem::Instance().getLogConfig();
  logging->setLogLevel(LogConfig::c_Debug);
  logging->setDebugLevel(10);

  //------------------------------------------------------------------------
  //..Calculate the new values for requested payload
  std::string map_path = FileSystem::findFile("ecl/data/ecl_channels_map.txt");
  if (!FileSystem::fileExists(map_path)) {
    B2FATAL("eclChannelMapper initialization file " << map_path << " doesn't exist");
  }
  ECLChannelMapper mapper;
  mapper.initFromFile(map_path.c_str());

  //------------------------------------------------------------------------
  //..Write out to localdb
  Belle2::DBImportObjPtr<Belle2::ECLChannelMap> importer("ECLChannelMap");
  importer.construct(mapper.getDBObject());
  importer.import(Belle2::IntervalOfValidity(experiment, run, -1, -1));
  std::cout << "Successfully wrote payload ECLChannelMap with iov " << experiment << "," << run << ",-1,-1" << std::endl;
}

