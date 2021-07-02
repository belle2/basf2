/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Peter Kvasnicka                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/dataobjects/BackgroundMetaData.h>

#include <TFile.h>
#include <TTree.h>
#include <TError.h>

#include <boost/program_options.hpp>

#include <string>
#include <iostream>

using namespace std;
using namespace Belle2;
namespace prog = boost::program_options;

int main(int argc, char* argv[])
{
  // define command line options
  prog::options_description options("Options");
  options.add_options()
  ("help,h", "print all available options")
  ("file", prog::value<string>(), "file name")
  ("time,t", prog::value<double>(), "real time of background sample in microseconds")
  ("name,n", prog::value<string>(), "name of the background component")
  ;

  prog::positional_options_description posOptDesc;
  posOptDesc.add("file", -1);

  prog::variables_map varMap;
  prog::store(prog::command_line_parser(argc, argv).
              options(options).positional(posOptDesc).run(), varMap);
  prog::notify(varMap);

  // check for help option
  if (varMap.count("help")) {
    cout << "Usage: " << argv[0] << " [OPTIONS] [FILE]\n";
    cout << options << endl;
    return 0;
  }

  // check parameters
  for (auto param : {"file", "time"}) {
    if (!varMap.count(param)) {
      B2ERROR("The " << param << " parameter is missing.");
      return 1;
    }
  }
  bool setName = (varMap.count("name") > 0);

  // read parameters
  string fileName = varMap["file"].as<string>();
  double realTime = varMap["time"].as<double>();
  string compName;
  if (setName)
    compName = varMap["name"].as<string>();

  // open the root file
  gErrorIgnoreLevel = kError;
  TFile* file = TFile::Open(fileName.c_str(), "UPDATE");
  if (!file || !file->IsOpen()) {
    B2ERROR("Failed to open the file " << fileName);
    return 1;
  }

  // read the BackgroundMetaData object or create a new one if it doesn't exist
  BackgroundMetaData* bgMetaData = 0;
  TTree* tree = (TTree*) file->Get("persistent");
  TTree* newTree = 0;
  if (!tree) {
    bgMetaData = dynamic_cast<BackgroundMetaData*>(file->Get("BackgroundMetaData"));
    if (!bgMetaData) {
      B2WARNING("Failed to get persistent tree in the file " << fileName);
      tree = new TTree("persistent", "persistent");
      bgMetaData = new BackgroundMetaData;
      tree->Branch("BackgroundMetaData", &bgMetaData);
      newTree = tree;
    }
  } else {
    tree->SetBranchAddress("BackgroundMetaData", &bgMetaData);
    newTree = tree->CloneTree(0);
    tree->GetEntry(0);
  }

  // update the IDs and write the updated BackgroundMetaData to the file
  // cppcheck-suppress nullPointerRedundantCheck
  bgMetaData->setRealTime(realTime * Unit::us);
  if (setName)
    bgMetaData->setBackgroundType(compName);
  if (newTree) {
    newTree->Fill();
    newTree->Write();
  } else {
    bgMetaData->Write("BackgroundMetaData");
  }

  cout << "File: " << fileName << endl;
  cout << "Real time set to " << realTime << " microseconds." << endl << endl;
  if (setName)
    cout << "Background type set to " << compName.c_str() << endl;

  return 0;
}

