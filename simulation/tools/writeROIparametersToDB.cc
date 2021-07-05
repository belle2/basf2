/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/database/DBImportObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Configuration.h>
#include <framework/logging/LogSystem.h>
#include <framework/utilities/FileSystem.h>

#include <simulation/dbobjects/ROIParameters.h>

#include <iostream>

//------------------------------------------------------------------------
int main(int argc, char** argv)
{
  if (argc < 4 or !(std::stoi(argv[1]) == 0 or std::stoi(argv[1]) == 1)) {
    std::cout <<
              "Please specify IOV: 0/1 {=off/on} N {=off for every Nth event; -1 if not used} experiment run [experimentEnd] [runEnd]" <<
              std::endl;
    return -1;
  }

  bool enableROI = std::stoi(argv[1]) == 1;
  int disableROIforEveryNth = std::stoi(argv[2]);

  if (!enableROI) {
    disableROIforEveryNth = -1;
    std::cout << "Parameter \"N\" does not have any function if ROI finding turned off. Neglecting." << std::endl;
  } else if (disableROIforEveryNth == 0 or disableROIforEveryNth == 1) {
    disableROIforEveryNth = -1;
    std::cout <<
              "Parameter \"N\" does not make any sense. Please use positive integer greater than 1 (or negative integer for disabling this feature)."
              << std::endl;
  } else if (disableROIforEveryNth < -1) {
    disableROIforEveryNth = -1;
  }

  int experiment = std::stoi(argv[3]);
  int run = std::stoi(argv[4]);
  int exp_end = -1;
  int run_end = -1;
  if (argc > 5) exp_end = std::stoi(argv[5]);
  if (argc > 6) run_end = std::stoi(argv[6]);

  //------------------------------------------------------------------------
  //..Specify database
  auto& conf = Belle2::Conditions::Configuration::getInstance();
  conf.prependTestingPayloadLocation("localdb/database.txt");

  //..set debug level
  Belle2::LogConfig* logging = Belle2::LogSystem::Instance().getLogConfig();
  logging->setLogLevel(Belle2::LogConfig::c_Debug);
  logging->setDebugLevel(10);

  //..create ROI parameters
  Belle2::ROIParameters roiParameters;
  roiParameters.setROIfinding(enableROI);
  roiParameters.setDisableROIforEveryNth(disableROIforEveryNth);

  //------------------------------------------------------------------------
  //..Write out to localdb
  Belle2::DBImportObjPtr<Belle2::ROIParameters> roiParametersDBPtr;
  roiParametersDBPtr.construct(roiParameters);
  roiParametersDBPtr.import(Belle2::IntervalOfValidity(experiment, run, exp_end, run_end));
  std::cout << "Successfully wrote payload ROIParameters with iov "
            << experiment << "," << run << "," << exp_end << "," << run_end << std::endl;
}

