/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/database/DBImportObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/LocalDatabase.h>
#include <framework/database/DatabaseChain.h>
#include <framework/logging/LogSystem.h>
#include <framework/utilities/FileSystem.h>

#include <simulation/dbobjects/ROIParameters.h>

#include <iostream>

//------------------------------------------------------------------------
int main(int argc, char** argv)
{
  if (argc != 0 and argc != 1) {
    std::cout << "Please specify IOV: 0/1 experiment run experimentEnd runEnd" << std::endl;
    return -1;
  }
  int experiment = std::stoi(argv[0]);
  int run = std::stoi(argv[1]);
  int exp_end = std::stoi(argv[2]);
  int run_end = std::stoi(argv[3]);

  //------------------------------------------------------------------------
  //..Specify database
  Belle2::DatabaseChain::createInstance();
  Belle2::LocalDatabase::createInstance("localdb/database.txt", "", Belle2::LogConfig::c_Debug);

  //..set debug level
  Belle2::LogConfig* logging = Belle2::LogSystem::Instance().getLogConfig();
  logging->setLogLevel(Belle2::LogConfig::c_Debug);
  logging->setDebugLevel(10);

  //..create ROI parameters
  Belle2::ROIParameters roiParameters;
  roiParameters.setROIfinding(argc);

  //------------------------------------------------------------------------
  //..Write out to localdb
  Belle2::DBImportObjPtr<Belle2::ROIParameters> roiParametersDBPtr;
  roiParametersDBPtr.construct(roiParameters);
  roiParametersDBPtr.import(Belle2::IntervalOfValidity(experiment, run, exp_end, run_end));
  std::cout << "Successfully wrote payload ROIParameters with iov "
            << experiment << "," << run << "," << exp_end << "," << run_end << std::endl;
}

