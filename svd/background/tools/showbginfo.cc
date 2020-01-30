/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Peter Kvasnicka                                              *
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
  // Define command line options
  prog::options_description options("Options");
  options.add_options()
  ("help,h", "print available options")
  ("file,f", prog::value<string>(), "local file name")
  ;

  prog::positional_options_description posOptDesc;
  posOptDesc.add("file", -1);

  prog::variables_map varMap;
  prog::store(prog::command_line_parser(argc, argv).
              options(options).positional(posOptDesc).run(), varMap);
  prog::notify(varMap);

  //Check for help option
  if (varMap.count("help")) {
    cout << "Usage: " << argv[0] << " [OPTIONS] [FILE]\n";
    cout << options << endl;
    return 0;
  }

  BackgroundMetaData metaData;
  BackgroundMetaData* metaDataPtr = &metaData;

  //Check for file option
  if (varMap.count("file")) {
    gErrorIgnoreLevel = kError;
    string fileName = varMap["file"].as<string>();
    TFile* file = TFile::Open(fileName.c_str(), "READ");
    if (!file || !file->IsOpen()) {
      B2ERROR("Couldn't open file " << fileName);
      return 1;
    }
    TTree* tree = (TTree*) file->Get("persistent");
    if (!tree) {
      B2ERROR("No tree persistent found in " << fileName);
      return 1;
    }
    TBranch* branch = tree->GetBranch("BackgroundMetaData");
    if (!branch) {
      B2ERROR("No meta data found in " << fileName);
      return 1;
    }
    metaDataPtr = 0;
    branch->SetAddress(&metaDataPtr);
    tree->GetEntry(0);

  } else {
    B2ERROR("Please specify a file name.");
    return 1;
  }

  cout << "BackgroundMetaData in file " <<  varMap["file"].as<string>() << ": " << endl;
  cout << "Background type : " << metaDataPtr->getBackgroundType() << endl;
  cout << "Real time       : " << metaDataPtr->getRealTime() / Unit::us << " us" << endl << endl;

  return 0;
}

