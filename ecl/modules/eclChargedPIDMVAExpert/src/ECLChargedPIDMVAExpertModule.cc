/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marcel Hohmann (marcel.hohmann@unimelb.edu.au)           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <math.h>
#include <algorithm>
#include "TMath.h"
#include <string>
#include <iostream>

#include <ecl/modules/eclShowerShape/ECLShowerShapeModule.h>
#include <ecl/modules/eclChargedPIDMVAExpert/ECLChargedPIDMVAExpertModule.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/geometry/ECLNeighbours.h>
#include <framework/geometry/B2Vector3.h>

using namespace Belle2;
using namespace ECL;

REG_MODULE(ECLChargedPIDMVAExpert)

ECLChargedPIDMVAExpertModule::ECLChargedPIDMVAExpertModule() : Module()
{
  setDescription("");

  //setPropertyFlags(c_ParallelProcessingCertified);

  addParam("filename",
           m_filename,
           "Name of root file to which the ECL Charged PID MVA training variables will be written",
           std::string("ECLChargedPIDMVA_training_variables.root"));
  addParam("pdg_code",
           m_pdg_code,
           "PDG code for events to keep in case 'keep_only_truth_matched' is set to True",
           int(11));
  addParam("keep_only_truth_matched",
           m_keep_only_truth_matched,
           "If true write out only the truth matched candidates",
           bool(false));
}

ECLChargedPIDMVAExpertModule::~ECLChargedPIDMVAExpertModule() {}


void ECLChargedPIDMVAExpertModule::initialize()
{
  m_eventMetaData.isRequired();
  //m_eclShowers.registerInDataStore("ECLShowers");
  //m_eclCalDigits.registerInDataStore("ECLCalDigits");

  // Create TFile
  m_TFile = TFile::Open(m_filename.c_str(), "RECREATE");
  m_TTree = new TTree("ntuple", "Training Data for for the ECL Charged PID MVA");

  m_TTree -> Branch("E1E9", &m_E1E9, "E1E9/F");
  m_TTree -> Branch("E9E21", &m_E9E21, "E9E21/F");
  m_TTree -> Branch("E", &m_E, "E/F");
  m_TTree -> Branch("EoP", &m_EoP, "EoP/F");
  m_TTree -> Branch("EoPBremsCorrected", &m_EoPBremsCorrected, "EoPBremsCorrected/F");
  m_TTree -> Branch("Z40", &m_Z40, "Z40/F");
  m_TTree -> Branch("Z51", &m_Z51, "Z51/F");
  m_TTree -> Branch("ZMVA", &m_ZMVA, "ZMVA/F");
  m_TTree -> Branch("PSDMVA", &m_PSDMVA, "PSDMVA/F");
  m_TTree -> Branch("DeltaL", &m_DeltaL, "DeltaL/F");
  m_TTree -> Branch("LAT", &m_LAT, "LAT/F");

  m_TTree -> Branch("pBremsCorrected", &m_pBremsCorrected, "pBremsCorrected/F");
  m_TTree -> Branch("p", &m_p, "p/F");
  m_TTree -> Branch("theta", &m_theta, "theta/F");
  m_TTree -> Branch("clusterTheta", &m_clusterTheta, "clusterTheta/F");
  m_TTree -> Branch("charge", &m_charge, "charge/I");
  m_TTree -> Branch("mcPDG", &m_mcPDG, "mcPDG/I");
  m_TTree -> Branch("mcMotherPDG", &m_mcMotherPDG, "mcMotherPDG/I");

  m_TTree -> Branch("Z00", &m_Z00, "Z00/F");
  m_TTree -> Branch("Z10", &m_Z10, "Z10/F");
  m_TTree -> Branch("Z11", &m_Z11, "Z11/F");
  m_TTree -> Branch("Z20", &m_Z20, "Z20/F");
  m_TTree -> Branch("Z21", &m_Z21, "Z21/F");
  m_TTree -> Branch("Z22", &m_Z22, "Z22/F");
  m_TTree -> Branch("Z30", &m_Z30, "Z30/F");
  m_TTree -> Branch("Z31", &m_Z31, "Z31/F");
  m_TTree -> Branch("Z32", &m_Z32, "Z32/F");
  m_TTree -> Branch("Z33", &m_Z33, "Z33/F");
  //m_TTree -> Branch("Z40", &m_Z40, "Z40/F");
  m_TTree -> Branch("Z41", &m_Z41, "Z41/F");
  m_TTree -> Branch("Z42", &m_Z42, "Z42/F");
  m_TTree -> Branch("Z43", &m_Z43, "Z43/F");
  m_TTree -> Branch("Z44", &m_Z44, "Z44/F");
  m_TTree -> Branch("Z50", &m_Z50, "Z50/F");
  //m_TTree -> Branch("Z51", &m_Z51, "Z51/F");
  m_TTree -> Branch("Z52", &m_Z52, "Z52/F");
  m_TTree -> Branch("Z53", &m_Z53, "Z53/F");
  m_TTree -> Branch("Z54", &m_Z54, "Z54/F");
  m_TTree -> Branch("Z55", &m_Z55, "Z55/F");
  m_TTree -> Branch("Z60", &m_Z60, "Z60/F");
  m_TTree -> Branch("Z61", &m_Z61, "Z61/F");
  m_TTree -> Branch("Z62", &m_Z62, "Z62/F");
  m_TTree -> Branch("Z63", &m_Z63, "Z63/F");
  m_TTree -> Branch("Z64", &m_Z64, "Z64/F");
  m_TTree -> Branch("Z65", &m_Z65, "Z65/F");
  m_TTree -> Branch("Z66", &m_Z66, "Z66/F");

  m_TTree -> Branch("PSD0HadronEnergy", &m_PSD0HadronEnergy, "PSD0HadronEnergy/F");
  m_TTree -> Branch("PSD0OnlineEnergy", &m_PSD0OnlineEnergy, "PSD0OnlineEnergy/F");
  m_TTree -> Branch("PSD0HadronEnergyFraction", &m_PSD0HadronEnergyFraction, "PSD0HadronEnergyFraction/F");
  m_TTree -> Branch("PSD0DigitWeight", &m_PSD0DigitWeight, "PSD0DigitWeight/F");
  m_TTree -> Branch("PSD0DigitFitType", &m_PSD0DigitFitType, "PSD0DigitFitType/F");
  m_TTree -> Branch("PSD0R", &m_PSD0R, "PSD0R/F");
  m_TTree -> Branch("PSD0CosTheta", &m_PSD0CosTheta, "PSD0CosTheta/F");
  m_TTree -> Branch("PSD0Phi", &m_PSD0Phi, "PSD0Phi/F");

  m_TTree -> Branch("PSD1HadronEnergy", &m_PSD1HadronEnergy, "PSD1HadronEnergy/F");
  m_TTree -> Branch("PSD1OnlineEnergy", &m_PSD1OnlineEnergy, "PSD1OnlineEnergy/F");
  m_TTree -> Branch("PSD1HadronEnergyFraction", &m_PSD1HadronEnergyFraction, "PSD1HadronEnergyFraction/F");
  m_TTree -> Branch("PSD1DigitWeight", &m_PSD1DigitWeight, "PSD1DigitWeight/F");
  m_TTree -> Branch("PSD1DigitFitType", &m_PSD1DigitFitType, "PSD1DigitFitType/F");
  m_TTree -> Branch("PSD1R", &m_PSD1R, "PSD1R/F");
  m_TTree -> Branch("PSD1CosTheta", &m_PSD1CosTheta, "PSD1CosTheta/F");
  m_TTree -> Branch("PSD1Phi", &m_PSD1Phi, "PSD1Phi/F");

  m_TTree -> Branch("PSD2HadronEnergy", &m_PSD2HadronEnergy, "PSD2HadronEnergy/F");
  m_TTree -> Branch("PSD2OnlineEnergy", &m_PSD2OnlineEnergy, "PSD2OnlineEnergy/F");
  m_TTree -> Branch("PSD2HadronEnergyFraction", &m_PSD2HadronEnergyFraction, "PSD2HadronEnergyFraction/F");
  m_TTree -> Branch("PSD2DigitWeight", &m_PSD2DigitWeight, "PSD2DigitWeight/F");
  m_TTree -> Branch("PSD2DigitFitType", &m_PSD2DigitFitType, "PSD2DigitFitType/F");
  m_TTree -> Branch("PSD2R", &m_PSD2R, "PSD2R/F");
  m_TTree -> Branch("PSD2CosTheta", &m_PSD2CosTheta, "PSD2CosTheta/F");
  m_TTree -> Branch("PSD2Phi", &m_PSD2Phi, "PSD2Phi/F");

  m_TTree -> Branch("PSD3HadronEnergy", &m_PSD3HadronEnergy, "PSD3HadronEnergy/F");
  m_TTree -> Branch("PSD3OnlineEnergy", &m_PSD3OnlineEnergy, "PSD3OnlineEnergy/F");
  m_TTree -> Branch("PSD3HadronEnergyFraction", &m_PSD3HadronEnergyFraction, "PSD3HadronEnergyFraction/F");
  m_TTree -> Branch("PSD3DigitWeight", &m_PSD3DigitWeight, "PSD3DigitWeight/F");
  m_TTree -> Branch("PSD3DigitFitType", &m_PSD3DigitFitType, "PSD3DigitFitType/F");
  m_TTree -> Branch("PSD3R", &m_PSD3R, "PSD3R/F");
  m_TTree -> Branch("PSD3CosTheta", &m_PSD3CosTheta, "PSD3CosTheta/F");
  m_TTree -> Branch("PSD3Phi", &m_PSD3Phi, "PSD3Phi/F");

  m_TTree -> Branch("PSD4HadronEnergy", &m_PSD4HadronEnergy, "PSD4HadronEnergy/F");
  m_TTree -> Branch("PSD4OnlineEnergy", &m_PSD4OnlineEnergy, "PSD4OnlineEnergy/F");
  m_TTree -> Branch("PSD4HadronEnergyFraction", &m_PSD4HadronEnergyFraction, "PSD4HadronEnergyFraction/F");
  m_TTree -> Branch("PSD4DigitWeight", &m_PSD4DigitWeight, "PSD4DigitWeight/F");
  m_TTree -> Branch("PSD4DigitFitType", &m_PSD4DigitFitType, "PSD4DigitFitType/F");
  m_TTree -> Branch("PSD4R", &m_PSD4R, "PSD4R/F");
  m_TTree -> Branch("PSD4CosTheta", &m_PSD4CosTheta, "PSD4CosTheta/F");
  m_TTree -> Branch("PSD4Phi", &m_PSD4Phi, "PSD4Phi/F");

  m_TTree -> Branch("PSD5HadronEnergy", &m_PSD5HadronEnergy, "PSD5HadronEnergy/F");
  m_TTree -> Branch("PSD5OnlineEnergy", &m_PSD5OnlineEnergy, "PSD5OnlineEnergy/F");
  m_TTree -> Branch("PSD5HadronEnergyFraction", &m_PSD5HadronEnergyFraction, "PSD5HadronEnergyFraction/F");
  m_TTree -> Branch("PSD5DigitWeight", &m_PSD5DigitWeight, "PSD5DigitWeight/F");
  m_TTree -> Branch("PSD5DigitFitType", &m_PSD5DigitFitType, "PSD5DigitFitType/F");
  m_TTree -> Branch("PSD5R", &m_PSD5R, "PSD5R/F");
  m_TTree -> Branch("PSD5CosTheta", &m_PSD5CosTheta, "PSD5CosTheta/F");
  m_TTree -> Branch("PSD5Phi", &m_PSD5Phi, "PSD5Phi/F");

  m_TTree -> Branch("PSD6HadronEnergy", &m_PSD6HadronEnergy, "PSD6HadronEnergy/F");
  m_TTree -> Branch("PSD6OnlineEnergy", &m_PSD6OnlineEnergy, "PSD6OnlineEnergy/F");
  m_TTree -> Branch("PSD6HadronEnergyFraction", &m_PSD6HadronEnergyFraction, "PSD6HadronEnergyFraction/F");
  m_TTree -> Branch("PSD6DigitWeight", &m_PSD6DigitWeight, "PSD6DigitWeight/F");
  m_TTree -> Branch("PSD6DigitFitType", &m_PSD6DigitFitType, "PSD6DigitFitType/F");
  m_TTree -> Branch("PSD6R", &m_PSD6R, "PSD6R/F");
  m_TTree -> Branch("PSD6CosTheta", &m_PSD6CosTheta, "PSD6CosTheta/F");
  m_TTree -> Branch("PSD6Phi", &m_PSD6Phi, "PSD6Phi/F");

  m_TTree -> Branch("PSD7HadronEnergy", &m_PSD7HadronEnergy, "PSD7HadronEnergy/F");
  m_TTree -> Branch("PSD7OnlineEnergy", &m_PSD7OnlineEnergy, "PSD7OnlineEnergy/F");
  m_TTree -> Branch("PSD7HadronEnergyFraction", &m_PSD7HadronEnergyFraction, "PSD7HadronEnergyFraction/F");
  m_TTree -> Branch("PSD7DigitWeight", &m_PSD7DigitWeight, "PSD7DigitWeight/F");
  m_TTree -> Branch("PSD7DigitFitType", &m_PSD7DigitFitType, "PSD7DigitFitType/F");
  m_TTree -> Branch("PSD7R", &m_PSD7R, "PSD7R/F");
  m_TTree -> Branch("PSD7CosTheta", &m_PSD7CosTheta, "PSD7CosTheta/F");
  m_TTree -> Branch("PSD7Phi", &m_PSD7Phi, "PSD7Phi/F");

  m_TTree -> Branch("PSD8HadronEnergy", &m_PSD8HadronEnergy, "PSD8HadronEnergy/F");
  m_TTree -> Branch("PSD8OnlineEnergy", &m_PSD8OnlineEnergy, "PSD8OnlineEnergy/F");
  m_TTree -> Branch("PSD8HadronEnergyFraction", &m_PSD8HadronEnergyFraction, "PSD8HadronEnergyFraction/F");
  m_TTree -> Branch("PSD8DigitWeight", &m_PSD8DigitWeight, "PSD8DigitWeight/F");
  m_TTree -> Branch("PSD8DigitFitType", &m_PSD8DigitFitType, "PSD8DigitFitType/F");
  m_TTree -> Branch("PSD8R", &m_PSD8R, "PSD8R/F");
  m_TTree -> Branch("PSD8CosTheta", &m_PSD8CosTheta, "PSD8CosTheta/F");
  m_TTree -> Branch("PSD8Phi", &m_PSD8Phi, "PSD8Phi/F");

  m_TTree -> Branch("PSD9HadronEnergy", &m_PSD9HadronEnergy, "PSD9HadronEnergy/F");
  m_TTree -> Branch("PSD9OnlineEnergy", &m_PSD9OnlineEnergy, "PSD9OnlineEnergy/F");
  m_TTree -> Branch("PSD9HadronEnergyFraction", &m_PSD9HadronEnergyFraction, "PSD9HadronEnergyFraction/F");
  m_TTree -> Branch("PSD9DigitWeight", &m_PSD9DigitWeight, "PSD9DigitWeight/F");
  m_TTree -> Branch("PSD9DigitFitType", &m_PSD9DigitFitType, "PSD9DigitFitType/F");
  m_TTree -> Branch("PSD9R", &m_PSD9R, "PSD9R/F");
  m_TTree -> Branch("PSD9CosTheta", &m_PSD9CosTheta, "PSD9CosTheta/F");
  m_TTree -> Branch("PSD9Phi", &m_PSD9Phi, "PSD9Phi/F");

  m_TTree -> Branch("PSD10HadronEnergy", &m_PSD10HadronEnergy, "PSD10HadronEnergy/F");
  m_TTree -> Branch("PSD10OnlineEnergy", &m_PSD10OnlineEnergy, "PSD10OnlineEnergy/F");
  m_TTree -> Branch("PSD10HadronEnergyFraction", &m_PSD10HadronEnergyFraction, "PSD10HadronEnergyFraction/F");
  m_TTree -> Branch("PSD10DigitWeight", &m_PSD10DigitWeight, "PSD10DigitWeight/F");
  m_TTree -> Branch("PSD10DigitFitType", &m_PSD10DigitFitType, "PSD10DigitFitType/F");
  m_TTree -> Branch("PSD10R", &m_PSD10R, "PSD10R/F");
  m_TTree -> Branch("PSD10CosTheta", &m_PSD10CosTheta, "PSD10CosTheta/F");
  m_TTree -> Branch("PSD10Phi", &m_PSD10Phi, "PSD10Phi/F");

  m_TTree -> Branch("PSD11HadronEnergy", &m_PSD11HadronEnergy, "PSD11HadronEnergy/F");
  m_TTree -> Branch("PSD11OnlineEnergy", &m_PSD11OnlineEnergy, "PSD11OnlineEnergy/F");
  m_TTree -> Branch("PSD11HadronEnergyFraction", &m_PSD11HadronEnergyFraction, "PSD11HadronEnergyFraction/F");
  m_TTree -> Branch("PSD11DigitWeight", &m_PSD11DigitWeight, "PSD11DigitWeight/F");
  m_TTree -> Branch("PSD11DigitFitType", &m_PSD11DigitFitType, "PSD11DigitFitType/F");
  m_TTree -> Branch("PSD11R", &m_PSD11R, "PSD11R/F");
  m_TTree -> Branch("PSD11CosTheta", &m_PSD11CosTheta, "PSD11CosTheta/F");
  m_TTree -> Branch("PSD11Phi", &m_PSD11Phi, "PSD11Phi/F");

  m_TTree -> Branch("PSD12HadronEnergy", &m_PSD12HadronEnergy, "PSD12HadronEnergy/F");
  m_TTree -> Branch("PSD12OnlineEnergy", &m_PSD12OnlineEnergy, "PSD12OnlineEnergy/F");
  m_TTree -> Branch("PSD12HadronEnergyFraction", &m_PSD12HadronEnergyFraction, "PSD12HadronEnergyFraction/F");
  m_TTree -> Branch("PSD12DigitWeight", &m_PSD12DigitWeight, "PSD12DigitWeight/F");
  m_TTree -> Branch("PSD12DigitFitType", &m_PSD12DigitFitType, "PSD12DigitFitType/F");
  m_TTree -> Branch("PSD12R", &m_PSD12R, "PSD12R/F");
  m_TTree -> Branch("PSD12CosTheta", &m_PSD12CosTheta, "PSD12CosTheta/F");
  m_TTree -> Branch("PSD12Phi", &m_PSD12Phi, "PSD12Phi/F");

  m_TTree -> Branch("PSD13HadronEnergy", &m_PSD13HadronEnergy, "PSD13HadronEnergy/F");
  m_TTree -> Branch("PSD13OnlineEnergy", &m_PSD13OnlineEnergy, "PSD13OnlineEnergy/F");
  m_TTree -> Branch("PSD13HadronEnergyFraction", &m_PSD13HadronEnergyFraction, "PSD13HadronEnergyFraction/F");
  m_TTree -> Branch("PSD13DigitWeight", &m_PSD13DigitWeight, "PSD13DigitWeight/F");
  m_TTree -> Branch("PSD13DigitFitType", &m_PSD13DigitFitType, "PSD13DigitFitType/F");
  m_TTree -> Branch("PSD13R", &m_PSD13R, "PSD13R/F");
  m_TTree -> Branch("PSD13CosTheta", &m_PSD13CosTheta, "PSD13CosTheta/F");
  m_TTree -> Branch("PSD13Phi", &m_PSD13Phi, "PSD13Phi/F");

  m_TTree -> Branch("PSD14HadronEnergy", &m_PSD14HadronEnergy, "PSD14HadronEnergy/F");
  m_TTree -> Branch("PSD14OnlineEnergy", &m_PSD14OnlineEnergy, "PSD14OnlineEnergy/F");
  m_TTree -> Branch("PSD14HadronEnergyFraction", &m_PSD14HadronEnergyFraction, "PSD14HadronEnergyFraction/F");
  m_TTree -> Branch("PSD14DigitWeight", &m_PSD14DigitWeight, "PSD14DigitWeight/F");
  m_TTree -> Branch("PSD14DigitFitType", &m_PSD14DigitFitType, "PSD14DigitFitType/F");
  m_TTree -> Branch("PSD14R", &m_PSD14R, "PSD14R/F");
  m_TTree -> Branch("PSD14CosTheta", &m_PSD14CosTheta, "PSD14CosTheta/F");
  m_TTree -> Branch("PSD14Phi", &m_PSD14Phi, "PSD14Phi/F");

  m_TTree -> Branch("PSD15HadronEnergy", &m_PSD15HadronEnergy, "PSD15HadronEnergy/F");
  m_TTree -> Branch("PSD15OnlineEnergy", &m_PSD15OnlineEnergy, "PSD15OnlineEnergy/F");
  m_TTree -> Branch("PSD15HadronEnergyFraction", &m_PSD15HadronEnergyFraction, "PSD15HadronEnergyFraction/F");
  m_TTree -> Branch("PSD15DigitWeight", &m_PSD15DigitWeight, "PSD15DigitWeight/F");
  m_TTree -> Branch("PSD15DigitFitType", &m_PSD15DigitFitType, "PSD15DigitFitType/F");
  m_TTree -> Branch("PSD15R", &m_PSD15R, "PSD15R/F");
  m_TTree -> Branch("PSD15CosTheta", &m_PSD15CosTheta, "PSD15CosTheta/F");
  m_TTree -> Branch("PSD15Phi", &m_PSD15Phi, "PSD15Phi/F");

  m_TTree -> Branch("PSD16HadronEnergy", &m_PSD16HadronEnergy, "PSD16HadronEnergy/F");
  m_TTree -> Branch("PSD16OnlineEnergy", &m_PSD16OnlineEnergy, "PSD16OnlineEnergy/F");
  m_TTree -> Branch("PSD16HadronEnergyFraction", &m_PSD16HadronEnergyFraction, "PSD16HadronEnergyFraction/F");
  m_TTree -> Branch("PSD16DigitWeight", &m_PSD16DigitWeight, "PSD16DigitWeight/F");
  m_TTree -> Branch("PSD16DigitFitType", &m_PSD16DigitFitType, "PSD16DigitFitType/F");
  m_TTree -> Branch("PSD16R", &m_PSD16R, "PSD16R/F");
  m_TTree -> Branch("PSD16CosTheta", &m_PSD16CosTheta, "PSD16CosTheta/F");
  m_TTree -> Branch("PSD16Phi", &m_PSD16Phi, "PSD16Phi/F");

  m_TTree -> Branch("PSD17HadronEnergy", &m_PSD17HadronEnergy, "PSD17HadronEnergy/F");
  m_TTree -> Branch("PSD17OnlineEnergy", &m_PSD17OnlineEnergy, "PSD17OnlineEnergy/F");
  m_TTree -> Branch("PSD17HadronEnergyFraction", &m_PSD17HadronEnergyFraction, "PSD17HadronEnergyFraction/F");
  m_TTree -> Branch("PSD17DigitWeight", &m_PSD17DigitWeight, "PSD17DigitWeight/F");
  m_TTree -> Branch("PSD17DigitFitType", &m_PSD17DigitFitType, "PSD17DigitFitType/F");
  m_TTree -> Branch("PSD17R", &m_PSD17R, "PSD17R/F");
  m_TTree -> Branch("PSD17CosTheta", &m_PSD17CosTheta, "PSD17CosTheta/F");
  m_TTree -> Branch("PSD17Phi", &m_PSD17Phi, "PSD17Phi/F");

  m_TTree -> Branch("PSD18HadronEnergy", &m_PSD18HadronEnergy, "PSD18HadronEnergy/F");
  m_TTree -> Branch("PSD18OnlineEnergy", &m_PSD18OnlineEnergy, "PSD18OnlineEnergy/F");
  m_TTree -> Branch("PSD18HadronEnergyFraction", &m_PSD18HadronEnergyFraction, "PSD18HadronEnergyFraction/F");
  m_TTree -> Branch("PSD18DigitWeight", &m_PSD18DigitWeight, "PSD18DigitWeight/F");
  m_TTree -> Branch("PSD18DigitFitType", &m_PSD18DigitFitType, "PSD18DigitFitType/F");
  m_TTree -> Branch("PSD18R", &m_PSD18R, "PSD18R/F");
  m_TTree -> Branch("PSD18CosTheta", &m_PSD18CosTheta, "PSD18CosTheta/F");
  m_TTree -> Branch("PSD18Phi", &m_PSD18Phi, "PSD18Phi/F");

  m_TTree -> Branch("PSD19HadronEnergy", &m_PSD19HadronEnergy, "PSD19HadronEnergy/F");
  m_TTree -> Branch("PSD19OnlineEnergy", &m_PSD19OnlineEnergy, "PSD19OnlineEnergy/F");
  m_TTree -> Branch("PSD19HadronEnergyFraction", &m_PSD19HadronEnergyFraction, "PSD19HadronEnergyFraction/F");
  m_TTree -> Branch("PSD19DigitWeight", &m_PSD19DigitWeight, "PSD19DigitWeight/F");
  m_TTree -> Branch("PSD19DigitFitType", &m_PSD19DigitFitType, "PSD19DigitFitType/F");
  m_TTree -> Branch("PSD19R", &m_PSD19R, "PSD19R/F");
  m_TTree -> Branch("PSD19CosTheta", &m_PSD19CosTheta, "PSD19CosTheta/F");
  m_TTree -> Branch("PSD19Phi", &m_PSD19Phi, "PSD19Phi/F");

  m_TTree -> Branch("logL_SVD_electron", &m_logL_SVD_electron, "logL_SVD_electron/F");
  m_TTree -> Branch("logL_SVD_muon", &m_logL_SVD_muon, "logL_SVD_muon/F");
  m_TTree -> Branch("logL_SVD_pion", &m_logL_SVD_pion, "logL_SVD_pion/F");
  m_TTree -> Branch("logL_SVD_proton", &m_logL_SVD_proton, "logL_SVD_proton/F");
  m_TTree -> Branch("logL_SVD_deuteron", &m_logL_SVD_deuteron, "logL_SVD_deuteron/F");
  m_TTree -> Branch("logL_SVD_kaon", &m_logL_SVD_kaon, "logL_SVD_kaon/F");

  m_TTree -> Branch("logL_ECL_electron", &m_logL_ECL_electron, "logL_ECL_electron/F");
  m_TTree -> Branch("logL_ECL_muon", &m_logL_ECL_muon, "logL_ECL_muon/F");
  m_TTree -> Branch("logL_ECL_pion", &m_logL_ECL_pion, "logL_ECL_pion/F");
  m_TTree -> Branch("logL_ECL_proton", &m_logL_ECL_proton, "logL_ECL_proton/F");
  m_TTree -> Branch("logL_ECL_deuteron", &m_logL_ECL_deuteron, "logL_ECL_deuteron/F");
  m_TTree -> Branch("logL_ECL_kaon", &m_logL_ECL_kaon, "logL_ECL_kaon/F");

  m_TTree -> Branch("logL_CDC_electron", &m_logL_CDC_electron, "logL_CDC_electron/F");
  m_TTree -> Branch("logL_CDC_muon", &m_logL_CDC_muon, "logL_CDC_muon/F");
  m_TTree -> Branch("logL_CDC_pion", &m_logL_CDC_pion, "logL_CDC_pion/F");
  m_TTree -> Branch("logL_CDC_proton", &m_logL_CDC_proton, "logL_CDC_proton/F");
  m_TTree -> Branch("logL_CDC_deuteron", &m_logL_CDC_deuteron, "logL_CDC_deuteron/F");
  m_TTree -> Branch("logL_CDC_kaon", &m_logL_CDC_kaon, "logL_CDC_kaon/F");

  m_TTree -> Branch("logL_TOP_electron", &m_logL_TOP_electron, "logL_TOP_electron/F");
  m_TTree -> Branch("logL_TOP_muon", &m_logL_TOP_muon, "logL_TOP_muon/F");
  m_TTree -> Branch("logL_TOP_pion", &m_logL_TOP_pion, "logL_TOP_pion/F");
  m_TTree -> Branch("logL_TOP_proton", &m_logL_TOP_proton, "logL_TOP_proton/F");
  m_TTree -> Branch("logL_TOP_deuteron", &m_logL_TOP_deuteron, "logL_TOP_deuteron/F");
  m_TTree -> Branch("logL_TOP_kaon", &m_logL_TOP_kaon, "logL_TOP_kaon/F");

  m_TTree -> Branch("logL_ARICH_electron", &m_logL_ARICH_electron, "logL_ARICH_electron/F");
  m_TTree -> Branch("logL_ARICH_muon", &m_logL_ARICH_muon, "logL_ARICH_muon/F");
  m_TTree -> Branch("logL_ARICH_pion", &m_logL_ARICH_pion, "logL_ARICH_pion/F");
  m_TTree -> Branch("logL_ARICH_proton", &m_logL_ARICH_proton, "logL_ARICH_proton/F");
  m_TTree -> Branch("logL_ARICH_deuteron", &m_logL_ARICH_deuteron, "logL_ARICH_deuteron/F");
  m_TTree -> Branch("logL_ARICH_kaon", &m_logL_ARICH_kaon, "logL_ARICH_kaon/F");

  m_TTree -> Branch("logL_KLM_electron", &m_logL_KLM_electron, "logL_KLM_electron/F");
  m_TTree -> Branch("logL_KLM_muon", &m_logL_KLM_muon, "logL_KLM_muon/F");
  m_TTree -> Branch("logL_KLM_pion", &m_logL_KLM_pion, "logL_KLM_pion/F");
  m_TTree -> Branch("logL_KLM_proton", &m_logL_KLM_proton, "logL_KLM_proton/F");
  m_TTree -> Branch("logL_KLM_deuteron", &m_logL_KLM_deuteron, "logL_KLM_deuteron/F");
  m_TTree -> Branch("logL_KLM_kaon", &m_logL_KLM_kaon, "logL_KLM_kaon/F");
}

void ECLChargedPIDMVAExpertModule::beginRun()
{
}

void ECLChargedPIDMVAExpertModule::event()
{
  // get variables and write to ttree
  // mostly https://stash.desy.de/projects/B2/repos/software/browse/ecl/modules/eclChargedPID/src/ECLChargedPIDMVAExpertModule.cc with some small changes

  for (const auto& track : m_tracks) {

    // Load the pion fit hypothesis or the hypothesis which is the closest in mass to a pion
    // (the tracking will not always successfully fit with a pion hypothesis).
    const TrackFitResult* fitRes = track.getTrackFitResultWithClosestMass(Const::pion);
    if (fitRes == nullptr) continue;
    const auto relShowers = track.getRelationsTo<ECLShower>();
    if (relShowers.size() == 0) continue;

    // run some brems correction to see if it has an impact on PID
    // Largely taken from https://stash.desy.de/projects/B2/repos/software/browse/analysis/modules/BremsCorrection/src/BremsFinderModule.cc#220
    RelationVector<ECLCluster> bremClusters = track.getRelationsFrom<ECLCluster>("", "Bremsstrahlung");
    std::vector<std::pair <double, Particle>> selectedGammas;

    unsigned j = 0;
    for (auto bremCluster = bremClusters.begin(); bremCluster != bremClusters.end(); bremCluster++, j++) {
      double weight = bremClusters.weight(j);
      if (weight > 3.0)
        continue; // default max acceptance https://software.belle2.org/sphinx/release-05-02-06/analysis/doc/MAWrappers.html?highlight=brems#modularAnalysis.correctBrems
      if (not bremCluster->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons))
        continue; // require the cluster to have a photon hypothesis
      selectedGammas.push_back(std::make_pair(weight, Particle(bremClusters.object(j), Const::photon)));
    }
    std::sort(selectedGammas.begin(), selectedGammas.end(), [](auto a, auto b) { return a.first < b.first;});
    // allow for all photons for now, otherwise could just take the one with best ranking
    auto bremsCorrected4Vec = Particle(&track, Const::electron).get4Vector();
    for (auto gamma : selectedGammas) {
      bremsCorrected4Vec += gamma.second.get4Vector();
    }

    m_pBremsCorrected = bremsCorrected4Vec.Vect().Mag();
    m_p      = fitRes->getMomentum().Mag();
    m_theta  = fitRes->getMomentum().Theta();
    m_charge = fitRes->getChargeSign();

    double shEnergy(0.0), maxEnergy(0.0);

    const ECLShower* mostEnergeticShower = nullptr;

    for (const auto& eclShower : relShowers) {

      if (eclShower.getHypothesisId() != ECLShower::c_nPhotons) continue;

      shEnergy = eclShower.getEnergy();
      if (shEnergy > maxEnergy) {
        maxEnergy = shEnergy;
        mostEnergeticShower = &eclShower;
      }
    } // ecl showers

    double showerTheta = (mostEnergeticShower) ? mostEnergeticShower->getTheta() : -999.0;
    int showerReg = (mostEnergeticShower) ? mostEnergeticShower->getDetectorRegion() : -1;

    m_clusterTheta = showerTheta;
    // These are the variables that can be used to extract PDF templates for the likelihood / for the MVA training.
    m_E1E9   = (mostEnergeticShower) ? mostEnergeticShower->getE1oE9() : -1.0;
    m_E9E21  = (mostEnergeticShower) ? mostEnergeticShower->getE9oE21() : -1.0;
    m_E      = (mostEnergeticShower) ? maxEnergy : -1.0;
    m_EoP    = (mostEnergeticShower) ? maxEnergy / m_p : -1.0;
    m_EoPBremsCorrected   = (mostEnergeticShower) ? bremsCorrected4Vec.E() / m_pBremsCorrected : -1.0;
    m_PSDMVA = (mostEnergeticShower) ? mostEnergeticShower->getPulseShapeDiscriminationMVA() : -999.0;
    m_DeltaL = (mostEnergeticShower) ? mostEnergeticShower->getTrkDepth() : -1.0;
    m_LAT    = (mostEnergeticShower) ? mostEnergeticShower->getLateralEnergy() : -999.0;

    // get the mc PDG code of the MCParticle with the highest relation to the ECL shower
    m_mcPDG = -999;
    m_mcMotherPDG = -999;
    float maxRelationsWeight = -1;

    if (mostEnergeticShower) {
      const RelationVector<MCParticle> mcParticlesRelations = mostEnergeticShower->getRelationsTo<MCParticle>();
      for (unsigned int i = 0; i < mcParticlesRelations.size(); ++i) {
        if (mcParticlesRelations.weight(i) > maxRelationsWeight) {
          maxRelationsWeight = mcParticlesRelations.weight(i);
          m_mcPDG = mcParticlesRelations.object(i)->getPDG();
          MCParticle* mcMother = mcParticlesRelations.object(i)->getMother();
          if (mcMother == nullptr) {
            m_mcMotherPDG = -999;
          } else {
            m_mcMotherPDG = mcMother->getPDG();
          }
        }
      }
    }

    if ((m_keep_only_truth_matched) && (m_mcPDG != m_pdg_code)) {
      continue;
    }

    // Zernike moments.
    // Z40, Z51, ZMVA are saved to the ecl_shower.
//     m_Z40    = (mostEnergeticShower) ? mostEnergeticShower->getAbsZernikeMoment(4, 0) : -999.0;
//     m_Z51    = (mostEnergeticShower) ? mostEnergeticShower->getAbsZernikeMoment(5, 1) : -999.0;
    m_ZMVA   = (mostEnergeticShower) ? mostEnergeticShower->getZernikeMVA() : -999.0;

    // Everything else needs to be recalculated.
    // Calculation step is basically copied from https://stash.desy.de/projects/B2/repos/software/browse/ecl/modules/eclShowerShape/src/ECLChargedPIDMVAExpertModule.cc
    m_Z00 = -999;
    m_Z10 = -999;
    m_Z11 = -999;
    m_Z20 = -999;
    m_Z21 = -999;
    m_Z22 = -999;
    m_Z30 = -999;
    m_Z31 = -999;
    m_Z32 = -999;
    m_Z33 = -999;
    m_Z40 = -999;
    m_Z41 = -999;
    m_Z42 = -999;
    m_Z43 = -999;
    m_Z44 = -999;
    m_Z50 = -999;
    m_Z51 = -999;
    m_Z52 = -999;
    m_Z53 = -999;
    m_Z54 = -999;
    m_Z55 = -999;
    m_Z60 = -999;
    m_Z61 = -999;
    m_Z62 = -999;
    m_Z63 = -999;
    m_Z64 = -999;
    m_Z65 = -999;
    m_Z66 = -999;

    if (mostEnergeticShower) {
      //Project the digits on the plane perpendicular to the shower direction
//       std::vector<ProjectedECLDigit> projectedECLDigits = projectECLDigits(*mostEnergeticShower);

      const double showerEnergy = mostEnergeticShower->getEnergy();
      //const double showerTheta = mostEnergeticShower->getTheta(); // defined above
      const double showerPhi = mostEnergeticShower->getPhi();

      //sum crystal energies
//       double sumEnergies = 0.0;
//       for (const auto& projectedECLDigit : projectedECLDigits) sumEnergies += projectedECLDigit.energy;

      // this depends on photon or neutral Hadron hypothesis but we are only using the photon hypothesis
      const double rho0 = 10 * Unit::cm;
      m_Z00 = mostEnergeticShower->getAbsZernikeMoment(0, 0);
      m_Z10 = mostEnergeticShower->getAbsZernikeMoment(1, 0);
      m_Z11 = mostEnergeticShower->getAbsZernikeMoment(1, 1);
      m_Z20 = mostEnergeticShower->getAbsZernikeMoment(2, 0);
      m_Z21 = mostEnergeticShower->getAbsZernikeMoment(2, 1);
      m_Z22 = mostEnergeticShower->getAbsZernikeMoment(2, 2);
      m_Z30 = mostEnergeticShower->getAbsZernikeMoment(3, 0);
      m_Z31 = mostEnergeticShower->getAbsZernikeMoment(3, 1);
      m_Z32 = mostEnergeticShower->getAbsZernikeMoment(3, 2);
      m_Z33 = mostEnergeticShower->getAbsZernikeMoment(3, 3);
      m_Z40 = mostEnergeticShower->getAbsZernikeMoment(4, 0);
      m_Z41 = mostEnergeticShower->getAbsZernikeMoment(4, 1);
      m_Z42 = mostEnergeticShower->getAbsZernikeMoment(4, 2);
      m_Z43 = mostEnergeticShower->getAbsZernikeMoment(4, 3);
      m_Z44 = mostEnergeticShower->getAbsZernikeMoment(4, 4);
      m_Z50 = mostEnergeticShower->getAbsZernikeMoment(5, 0);
      m_Z51 = mostEnergeticShower->getAbsZernikeMoment(5, 1);
      m_Z52 = mostEnergeticShower->getAbsZernikeMoment(5, 2);
      m_Z53 = mostEnergeticShower->getAbsZernikeMoment(5, 3);
      m_Z54 = mostEnergeticShower->getAbsZernikeMoment(5, 4);
      m_Z55 = mostEnergeticShower->getAbsZernikeMoment(5, 5);
      m_Z60 = mostEnergeticShower->getAbsZernikeMoment(6, 0);
      m_Z61 = mostEnergeticShower->getAbsZernikeMoment(6, 1);
      m_Z62 = mostEnergeticShower->getAbsZernikeMoment(6, 2);
      m_Z63 = mostEnergeticShower->getAbsZernikeMoment(6, 3);
      m_Z64 = mostEnergeticShower->getAbsZernikeMoment(6, 4);
      m_Z65 = mostEnergeticShower->getAbsZernikeMoment(6, 5);
      m_Z66 = mostEnergeticShower->getAbsZernikeMoment(6, 6);
    }

    const auto pidlikelihood = track.getRelatedTo<PIDLikelihood>();

    m_logL_SVD_electron = pidlikelihood -> getLogL(Const::ChargedStable(11), Const::PIDDetectorSet(Const::SVD));
    m_logL_SVD_muon = pidlikelihood -> getLogL(Const::ChargedStable(13), Const::PIDDetectorSet(Const::SVD));
    m_logL_SVD_pion = pidlikelihood -> getLogL(Const::ChargedStable(211), Const::PIDDetectorSet(Const::SVD));
    m_logL_SVD_proton = pidlikelihood -> getLogL(Const::ChargedStable(2212), Const::PIDDetectorSet(Const::SVD));
    m_logL_SVD_deuteron = pidlikelihood -> getLogL(Const::ChargedStable(1000010020), Const::PIDDetectorSet(Const::SVD));
    m_logL_SVD_kaon = pidlikelihood -> getLogL(Const::ChargedStable(321), Const::PIDDetectorSet(Const::SVD));

    m_logL_ECL_electron = pidlikelihood -> getLogL(Const::ChargedStable(11), Const::PIDDetectorSet(Const::ECL));
    m_logL_ECL_muon = pidlikelihood -> getLogL(Const::ChargedStable(13), Const::PIDDetectorSet(Const::ECL));
    m_logL_ECL_pion = pidlikelihood -> getLogL(Const::ChargedStable(211), Const::PIDDetectorSet(Const::ECL));
    m_logL_ECL_proton = pidlikelihood -> getLogL(Const::ChargedStable(2212), Const::PIDDetectorSet(Const::ECL));
    m_logL_ECL_deuteron = pidlikelihood -> getLogL(Const::ChargedStable(1000010020), Const::PIDDetectorSet(Const::ECL));
    m_logL_ECL_kaon = pidlikelihood -> getLogL(Const::ChargedStable(321), Const::PIDDetectorSet(Const::ECL));

    m_logL_CDC_electron = pidlikelihood -> getLogL(Const::ChargedStable(11), Const::PIDDetectorSet(Const::CDC));
    m_logL_CDC_muon = pidlikelihood -> getLogL(Const::ChargedStable(13), Const::PIDDetectorSet(Const::CDC));
    m_logL_CDC_pion = pidlikelihood -> getLogL(Const::ChargedStable(211), Const::PIDDetectorSet(Const::CDC));
    m_logL_CDC_proton = pidlikelihood -> getLogL(Const::ChargedStable(2212), Const::PIDDetectorSet(Const::CDC));
    m_logL_CDC_deuteron = pidlikelihood -> getLogL(Const::ChargedStable(1000010020), Const::PIDDetectorSet(Const::CDC));
    m_logL_CDC_kaon = pidlikelihood -> getLogL(Const::ChargedStable(321), Const::PIDDetectorSet(Const::CDC));

    m_logL_TOP_electron = pidlikelihood -> getLogL(Const::ChargedStable(11), Const::PIDDetectorSet(Const::TOP));
    m_logL_TOP_muon = pidlikelihood -> getLogL(Const::ChargedStable(13), Const::PIDDetectorSet(Const::TOP));
    m_logL_TOP_pion = pidlikelihood -> getLogL(Const::ChargedStable(211), Const::PIDDetectorSet(Const::TOP));
    m_logL_TOP_proton = pidlikelihood -> getLogL(Const::ChargedStable(2212), Const::PIDDetectorSet(Const::TOP));
    m_logL_TOP_deuteron = pidlikelihood -> getLogL(Const::ChargedStable(1000010020), Const::PIDDetectorSet(Const::TOP));
    m_logL_TOP_kaon = pidlikelihood -> getLogL(Const::ChargedStable(321), Const::PIDDetectorSet(Const::TOP));

    m_logL_ARICH_electron = pidlikelihood -> getLogL(Const::ChargedStable(11), Const::PIDDetectorSet(Const::ARICH));
    m_logL_ARICH_muon = pidlikelihood -> getLogL(Const::ChargedStable(13), Const::PIDDetectorSet(Const::ARICH));
    m_logL_ARICH_pion = pidlikelihood -> getLogL(Const::ChargedStable(211), Const::PIDDetectorSet(Const::ARICH));
    m_logL_ARICH_proton = pidlikelihood -> getLogL(Const::ChargedStable(2212), Const::PIDDetectorSet(Const::ARICH));
    m_logL_ARICH_deuteron = pidlikelihood -> getLogL(Const::ChargedStable(1000010020), Const::PIDDetectorSet(Const::ARICH));
    m_logL_ARICH_kaon = pidlikelihood -> getLogL(Const::ChargedStable(321), Const::PIDDetectorSet(Const::ARICH));

    m_logL_KLM_electron = pidlikelihood -> getLogL(Const::ChargedStable(11), Const::PIDDetectorSet(Const::KLM));
    m_logL_KLM_muon = pidlikelihood -> getLogL(Const::ChargedStable(13), Const::PIDDetectorSet(Const::KLM));
    m_logL_KLM_pion = pidlikelihood -> getLogL(Const::ChargedStable(211), Const::PIDDetectorSet(Const::KLM));
    m_logL_KLM_proton = pidlikelihood -> getLogL(Const::ChargedStable(2212), Const::PIDDetectorSet(Const::KLM));
    m_logL_KLM_deuteron = pidlikelihood -> getLogL(Const::ChargedStable(1000010020), Const::PIDDetectorSet(Const::KLM));
    m_logL_KLM_kaon = pidlikelihood -> getLogL(Const::ChargedStable(321), Const::PIDDetectorSet(Const::KLM));


    // copied from PSD https://stash.desy.de/projects/B2/repos/software/browse/ecl/modules/eclClusterPSD/src/ECLClusterPSD.cc
    //geometry for cell id position
    ECL::ECLGeometryPar* geometry = ECL::ECLGeometryPar::Instance();



    m_PSD0HadronEnergy = 0;
    m_PSD0OnlineEnergy = 0;
    m_PSD0HadronEnergyFraction = 0;
    m_PSD0DigitWeight = 0;
    m_PSD0DigitFitType = -1;
    m_PSD0R = 0;
    m_PSD0CosTheta = 0;
    m_PSD0Phi = 0;

    m_PSD1HadronEnergy = 0;
    m_PSD1OnlineEnergy = 0;
    m_PSD1HadronEnergyFraction = 0;
    m_PSD1DigitWeight = 0;
    m_PSD1DigitFitType = -1;
    m_PSD1R = 0;
    m_PSD1CosTheta = 0;
    m_PSD1Phi = 0;

    m_PSD2HadronEnergy = 0;
    m_PSD2OnlineEnergy = 0;
    m_PSD2HadronEnergyFraction = 0;
    m_PSD2DigitWeight = 0;
    m_PSD2DigitFitType = -1;
    m_PSD2R = 0;
    m_PSD2CosTheta = 0;
    m_PSD2Phi = 0;

    m_PSD3HadronEnergy = 0;
    m_PSD3OnlineEnergy = 0;
    m_PSD3HadronEnergyFraction = 0;
    m_PSD3DigitWeight = 0;
    m_PSD3DigitFitType = -1;
    m_PSD3R = 0;
    m_PSD3CosTheta = 0;
    m_PSD3Phi = 0;

    m_PSD4HadronEnergy = 0;
    m_PSD4OnlineEnergy = 0;
    m_PSD4HadronEnergyFraction = 0;
    m_PSD4DigitWeight = 0;
    m_PSD4DigitFitType = -1;
    m_PSD4R = 0;
    m_PSD4CosTheta = 0;
    m_PSD4Phi = 0;

    m_PSD5HadronEnergy = 0;
    m_PSD5OnlineEnergy = 0;
    m_PSD5HadronEnergyFraction = 0;
    m_PSD5DigitWeight = 0;
    m_PSD5DigitFitType = -1;
    m_PSD5R = 0;
    m_PSD5CosTheta = 0;
    m_PSD5Phi = 0;

    m_PSD6HadronEnergy = 0;
    m_PSD6OnlineEnergy = 0;
    m_PSD6HadronEnergyFraction = 0;
    m_PSD6DigitWeight = 0;
    m_PSD6DigitFitType = -1;
    m_PSD6R = 0;
    m_PSD6CosTheta = 0;
    m_PSD6Phi = 0;

    m_PSD7HadronEnergy = 0;
    m_PSD7OnlineEnergy = 0;
    m_PSD7HadronEnergyFraction = 0;
    m_PSD7DigitWeight = 0;
    m_PSD7DigitFitType = -1;
    m_PSD7R = 0;
    m_PSD7CosTheta = 0;
    m_PSD7Phi = 0;

    m_PSD8HadronEnergy = 0;
    m_PSD8OnlineEnergy = 0;
    m_PSD8HadronEnergyFraction = 0;
    m_PSD8DigitWeight = 0;
    m_PSD8DigitFitType = -1;
    m_PSD8R = 0;
    m_PSD8CosTheta = 0;
    m_PSD8Phi = 0;

    m_PSD9HadronEnergy = 0;
    m_PSD9OnlineEnergy = 0;
    m_PSD9HadronEnergyFraction = 0;
    m_PSD9DigitWeight = 0;
    m_PSD9DigitFitType = -1;
    m_PSD9R = 0;
    m_PSD9CosTheta = 0;
    m_PSD9Phi = 0;

    m_PSD10HadronEnergy = 0;
    m_PSD10OnlineEnergy = 0;
    m_PSD10HadronEnergyFraction = 0;
    m_PSD10DigitWeight = 0;
    m_PSD10DigitFitType = -1;
    m_PSD10R = 0;
    m_PSD10CosTheta = 0;
    m_PSD10Phi = 0;

    m_PSD11HadronEnergy = 0;
    m_PSD11OnlineEnergy = 0;
    m_PSD11HadronEnergyFraction = 0;
    m_PSD11DigitWeight = 0;
    m_PSD11DigitFitType = -1;
    m_PSD11R = 0;
    m_PSD11CosTheta = 0;
    m_PSD11Phi = 0;

    m_PSD12HadronEnergy = 0;
    m_PSD12OnlineEnergy = 0;
    m_PSD12HadronEnergyFraction = 0;
    m_PSD12DigitWeight = 0;
    m_PSD12DigitFitType = -1;
    m_PSD12R = 0;
    m_PSD12CosTheta = 0;
    m_PSD12Phi = 0;

    m_PSD13HadronEnergy = 0;
    m_PSD13OnlineEnergy = 0;
    m_PSD13HadronEnergyFraction = 0;
    m_PSD13DigitWeight = 0;
    m_PSD13DigitFitType = -1;
    m_PSD13R = 0;
    m_PSD13CosTheta = 0;
    m_PSD13Phi = 0;

    m_PSD14HadronEnergy = 0;
    m_PSD14OnlineEnergy = 0;
    m_PSD14HadronEnergyFraction = 0;
    m_PSD14DigitWeight = 0;
    m_PSD14DigitFitType = -1;
    m_PSD14R = 0;
    m_PSD14CosTheta = 0;
    m_PSD14Phi = 0;

    m_PSD15HadronEnergy = 0;
    m_PSD15OnlineEnergy = 0;
    m_PSD15HadronEnergyFraction = 0;
    m_PSD15DigitWeight = 0;
    m_PSD15DigitFitType = -1;
    m_PSD15R = 0;
    m_PSD15CosTheta = 0;
    m_PSD15Phi = 0;

    m_PSD16HadronEnergy = 0;
    m_PSD16OnlineEnergy = 0;
    m_PSD16HadronEnergyFraction = 0;
    m_PSD16DigitWeight = 0;
    m_PSD16DigitFitType = -1;
    m_PSD16R = 0;
    m_PSD16CosTheta = 0;
    m_PSD16Phi = 0;

    m_PSD17HadronEnergy = 0;
    m_PSD17OnlineEnergy = 0;
    m_PSD17HadronEnergyFraction = 0;
    m_PSD17DigitWeight = 0;
    m_PSD17DigitFitType = -1;
    m_PSD17R = 0;
    m_PSD17CosTheta = 0;
    m_PSD17Phi = 0;

    m_PSD18HadronEnergy = 0;
    m_PSD18OnlineEnergy = 0;
    m_PSD18HadronEnergyFraction = 0;
    m_PSD18DigitWeight = 0;
    m_PSD18DigitFitType = -1;
    m_PSD18R = 0;
    m_PSD18CosTheta = 0;
    m_PSD18Phi = 0;

    m_PSD19HadronEnergy = 0;
    m_PSD19OnlineEnergy = 0;
    m_PSD19HadronEnergyFraction = 0;
    m_PSD19DigitWeight = 0;
    m_PSD19DigitFitType = -1;
    m_PSD19R = 0;
    m_PSD19CosTheta = 0;
    m_PSD19Phi = 0;

    if (mostEnergeticShower) {
      std::vector<std::tuple<double, unsigned int>> EnergyToSort;
      auto relatedDigits = mostEnergeticShower->getRelationsTo<ECLCalDigit>();

      //EnergyToSort vector is used for sorting digits by offline two component energy

      for (unsigned int iRel = 0; iRel < relatedDigits.size(); iRel++) {

        const auto caldigit = relatedDigits.object(iRel);

        //exclude digits without waveforms
        const double digitChi2 = caldigit->getTwoComponentChi2();
        if (digitChi2 < 0)  continue;

        ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();

        //exclude digits digits with poor chi2
        if (digitFitType1 == ECLDsp::poorChi2) continue;

        //exclude digits with diode-crossing fits
        if (digitFitType1 == ECLDsp::photonDiodeCrossing) continue;

        EnergyToSort.emplace_back(caldigit->getTwoComponentTotalEnergy(), iRel);

      }

      //sorting by energy
      std::sort(EnergyToSort.begin(), EnergyToSort.end(), std::greater<>());

      //get cluster position information
      const double showerR = mostEnergeticShower->getR();
      //const double showerTheta = cluster->getTheta();
      const double showerPhi = mostEnergeticShower->getPhi();

      B2Vector3D showerPosition;
      showerPosition.SetMagThetaPhi(showerR, showerTheta, showerPhi);

      // this is incredibly dumb
      if (0 < EnergyToSort.size()) {
        unsigned int digit = 0;
        const auto [digitEnergy, next] = EnergyToSort[digit];
        const auto caldigit = relatedDigits.object(next);
        m_PSD0HadronEnergy = caldigit->getTwoComponentHadronEnergy();
        m_PSD0OnlineEnergy = caldigit->getEnergy();
        m_PSD0HadronEnergyFraction = (m_PSD0HadronEnergy / digitEnergy);
        m_PSD0DigitWeight  = relatedDigits.weight(next);
        ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();
        m_PSD0DigitFitType = digitFitType1;
        const int cellId = caldigit->getCellId();
        B2Vector3D calDigitPosition = geometry->GetCrystalPos(cellId - 1);
        TVector3 tempP = showerPosition - calDigitPosition;
        m_PSD0R = tempP.Mag();
        m_PSD0CosTheta = tempP.CosTheta();
        m_PSD0Phi = tempP.Phi();
      }

      if (1 < EnergyToSort.size()) {
        unsigned int digit = 1;
        const auto [digitEnergy, next] = EnergyToSort[digit];
        const auto caldigit = relatedDigits.object(next);
        m_PSD1HadronEnergy = caldigit->getTwoComponentHadronEnergy();
        m_PSD1OnlineEnergy = caldigit->getEnergy();
        m_PSD1HadronEnergyFraction = (m_PSD1HadronEnergy / digitEnergy);
        m_PSD1DigitWeight  = relatedDigits.weight(next);
        ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();
        m_PSD1DigitFitType = digitFitType1;
        const int cellId = caldigit->getCellId();
        B2Vector3D calDigitPosition = geometry->GetCrystalPos(cellId - 1);
        TVector3 tempP = showerPosition - calDigitPosition;
        m_PSD1R = tempP.Mag();
        m_PSD1CosTheta = tempP.CosTheta();
        m_PSD1Phi = tempP.Phi();
      }

      if (2 < EnergyToSort.size()) {
        unsigned int digit = 2;
        const auto [digitEnergy, next] = EnergyToSort[digit];
        const auto caldigit = relatedDigits.object(next);
        m_PSD2HadronEnergy = caldigit->getTwoComponentHadronEnergy();
        m_PSD2OnlineEnergy = caldigit->getEnergy();
        m_PSD2HadronEnergyFraction = (m_PSD2HadronEnergy / digitEnergy);
        m_PSD2DigitWeight  = relatedDigits.weight(next);
        ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();
        m_PSD2DigitFitType = digitFitType1;
        const int cellId = caldigit->getCellId();
        B2Vector3D calDigitPosition = geometry->GetCrystalPos(cellId - 1);
        TVector3 tempP = showerPosition - calDigitPosition;
        m_PSD2R = tempP.Mag();
        m_PSD2CosTheta = tempP.CosTheta();
        m_PSD2Phi = tempP.Phi();
      }

      if (3 < EnergyToSort.size()) {
        unsigned int digit = 3;
        const auto [digitEnergy, next] = EnergyToSort[digit];
        const auto caldigit = relatedDigits.object(next);
        m_PSD3HadronEnergy = caldigit->getTwoComponentHadronEnergy();
        m_PSD3OnlineEnergy = caldigit->getEnergy();
        m_PSD3HadronEnergyFraction = (m_PSD3HadronEnergy / digitEnergy);
        m_PSD3DigitWeight  = relatedDigits.weight(next);
        ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();
        m_PSD3DigitFitType = digitFitType1;
        const int cellId = caldigit->getCellId();
        B2Vector3D calDigitPosition = geometry->GetCrystalPos(cellId - 1);
        TVector3 tempP = showerPosition - calDigitPosition;
        m_PSD3R = tempP.Mag();
        m_PSD3CosTheta = tempP.CosTheta();
        m_PSD3Phi = tempP.Phi();
      }

      if (4 < EnergyToSort.size()) {
        unsigned int digit = 4;
        const auto [digitEnergy, next] = EnergyToSort[digit];
        const auto caldigit = relatedDigits.object(next);
        m_PSD4HadronEnergy = caldigit->getTwoComponentHadronEnergy();
        m_PSD4OnlineEnergy = caldigit->getEnergy();
        m_PSD4HadronEnergyFraction = (m_PSD4HadronEnergy / digitEnergy);
        m_PSD4DigitWeight  = relatedDigits.weight(next);
        ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();
        m_PSD4DigitFitType = digitFitType1;
        const int cellId = caldigit->getCellId();
        B2Vector3D calDigitPosition = geometry->GetCrystalPos(cellId - 1);
        TVector3 tempP = showerPosition - calDigitPosition;
        m_PSD4R = tempP.Mag();
        m_PSD4CosTheta = tempP.CosTheta();
        m_PSD4Phi = tempP.Phi();
      }

      if (5 < EnergyToSort.size()) {
        unsigned int digit = 5;
        const auto [digitEnergy, next] = EnergyToSort[digit];
        const auto caldigit = relatedDigits.object(next);
        m_PSD5HadronEnergy = caldigit->getTwoComponentHadronEnergy();
        m_PSD5OnlineEnergy = caldigit->getEnergy();
        m_PSD5HadronEnergyFraction = (m_PSD5HadronEnergy / digitEnergy);
        m_PSD5DigitWeight  = relatedDigits.weight(next);
        ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();
        m_PSD5DigitFitType = digitFitType1;
        const int cellId = caldigit->getCellId();
        B2Vector3D calDigitPosition = geometry->GetCrystalPos(cellId - 1);
        TVector3 tempP = showerPosition - calDigitPosition;
        m_PSD5R = tempP.Mag();
        m_PSD5CosTheta = tempP.CosTheta();
        m_PSD5Phi = tempP.Phi();
      }

      if (6 < EnergyToSort.size()) {
        unsigned int digit = 6;
        const auto [digitEnergy, next] = EnergyToSort[digit];
        const auto caldigit = relatedDigits.object(next);
        m_PSD6HadronEnergy = caldigit->getTwoComponentHadronEnergy();
        m_PSD6OnlineEnergy = caldigit->getEnergy();
        m_PSD6HadronEnergyFraction = (m_PSD6HadronEnergy / digitEnergy);
        m_PSD6DigitWeight  = relatedDigits.weight(next);
        ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();
        m_PSD6DigitFitType = digitFitType1;
        const int cellId = caldigit->getCellId();
        B2Vector3D calDigitPosition = geometry->GetCrystalPos(cellId - 1);
        TVector3 tempP = showerPosition - calDigitPosition;
        m_PSD6R = tempP.Mag();
        m_PSD6CosTheta = tempP.CosTheta();
        m_PSD6Phi = tempP.Phi();
      }

      if (7 < EnergyToSort.size()) {
        unsigned int digit = 7;
        const auto [digitEnergy, next] = EnergyToSort[digit];
        const auto caldigit = relatedDigits.object(next);
        m_PSD7HadronEnergy = caldigit->getTwoComponentHadronEnergy();
        m_PSD7OnlineEnergy = caldigit->getEnergy();
        m_PSD7HadronEnergyFraction = (m_PSD7HadronEnergy / digitEnergy);
        m_PSD7DigitWeight  = relatedDigits.weight(next);
        ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();
        m_PSD7DigitFitType = digitFitType1;
        const int cellId = caldigit->getCellId();
        B2Vector3D calDigitPosition = geometry->GetCrystalPos(cellId - 1);
        TVector3 tempP = showerPosition - calDigitPosition;
        m_PSD7R = tempP.Mag();
        m_PSD7CosTheta = tempP.CosTheta();
        m_PSD7Phi = tempP.Phi();
      }

      if (8 < EnergyToSort.size()) {
        unsigned int digit = 8;
        const auto [digitEnergy, next] = EnergyToSort[digit];
        const auto caldigit = relatedDigits.object(next);
        m_PSD8HadronEnergy = caldigit->getTwoComponentHadronEnergy();
        m_PSD8OnlineEnergy = caldigit->getEnergy();
        m_PSD8HadronEnergyFraction = (m_PSD8HadronEnergy / digitEnergy);
        m_PSD8DigitWeight  = relatedDigits.weight(next);
        ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();
        m_PSD8DigitFitType = digitFitType1;
        const int cellId = caldigit->getCellId();
        B2Vector3D calDigitPosition = geometry->GetCrystalPos(cellId - 1);
        TVector3 tempP = showerPosition - calDigitPosition;
        m_PSD8R = tempP.Mag();
        m_PSD8CosTheta = tempP.CosTheta();
        m_PSD8Phi = tempP.Phi();
      }

      if (9 < EnergyToSort.size()) {
        unsigned int digit = 9;
        const auto [digitEnergy, next] = EnergyToSort[digit];
        const auto caldigit = relatedDigits.object(next);
        m_PSD9HadronEnergy = caldigit->getTwoComponentHadronEnergy();
        m_PSD9OnlineEnergy = caldigit->getEnergy();
        m_PSD9HadronEnergyFraction = (m_PSD9HadronEnergy / digitEnergy);
        m_PSD9DigitWeight  = relatedDigits.weight(next);
        ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();
        m_PSD9DigitFitType = digitFitType1;
        const int cellId = caldigit->getCellId();
        B2Vector3D calDigitPosition = geometry->GetCrystalPos(cellId - 1);
        TVector3 tempP = showerPosition - calDigitPosition;
        m_PSD9R = tempP.Mag();
        m_PSD9CosTheta = tempP.CosTheta();
        m_PSD9Phi = tempP.Phi();
      }

      if (10 < EnergyToSort.size()) {
        unsigned int digit = 10;
        const auto [digitEnergy, next] = EnergyToSort[digit];
        const auto caldigit = relatedDigits.object(next);
        m_PSD10HadronEnergy = caldigit->getTwoComponentHadronEnergy();
        m_PSD10OnlineEnergy = caldigit->getEnergy();
        m_PSD10HadronEnergyFraction = (m_PSD10HadronEnergy / digitEnergy);
        m_PSD10DigitWeight  = relatedDigits.weight(next);
        ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();
        m_PSD10DigitFitType = digitFitType1;
        const int cellId = caldigit->getCellId();
        B2Vector3D calDigitPosition = geometry->GetCrystalPos(cellId - 1);
        TVector3 tempP = showerPosition - calDigitPosition;
        m_PSD10R = tempP.Mag();
        m_PSD10CosTheta = tempP.CosTheta();
        m_PSD10Phi = tempP.Phi();
      }

      if (11 < EnergyToSort.size()) {
        unsigned int digit = 11;
        const auto [digitEnergy, next] = EnergyToSort[digit];
        const auto caldigit = relatedDigits.object(next);
        m_PSD11HadronEnergy = caldigit->getTwoComponentHadronEnergy();
        m_PSD11OnlineEnergy = caldigit->getEnergy();
        m_PSD11HadronEnergyFraction = (m_PSD11HadronEnergy / digitEnergy);
        m_PSD11DigitWeight  = relatedDigits.weight(next);
        ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();
        m_PSD11DigitFitType = digitFitType1;
        const int cellId = caldigit->getCellId();
        B2Vector3D calDigitPosition = geometry->GetCrystalPos(cellId - 1);
        TVector3 tempP = showerPosition - calDigitPosition;
        m_PSD11R = tempP.Mag();
        m_PSD11CosTheta = tempP.CosTheta();
        m_PSD11Phi = tempP.Phi();
      }

      if (12 < EnergyToSort.size()) {
        unsigned int digit = 12;
        const auto [digitEnergy, next] = EnergyToSort[digit];
        const auto caldigit = relatedDigits.object(next);
        m_PSD12HadronEnergy = caldigit->getTwoComponentHadronEnergy();
        m_PSD12OnlineEnergy = caldigit->getEnergy();
        m_PSD12HadronEnergyFraction = (m_PSD12HadronEnergy / digitEnergy);
        m_PSD12DigitWeight  = relatedDigits.weight(next);
        ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();
        m_PSD12DigitFitType = digitFitType1;
        const int cellId = caldigit->getCellId();
        B2Vector3D calDigitPosition = geometry->GetCrystalPos(cellId - 1);
        TVector3 tempP = showerPosition - calDigitPosition;
        m_PSD12R = tempP.Mag();
        m_PSD12CosTheta = tempP.CosTheta();
        m_PSD12Phi = tempP.Phi();
      }

      if (13 < EnergyToSort.size()) {
        unsigned int digit = 13;
        const auto [digitEnergy, next] = EnergyToSort[digit];
        const auto caldigit = relatedDigits.object(next);
        m_PSD13HadronEnergy = caldigit->getTwoComponentHadronEnergy();
        m_PSD13OnlineEnergy = caldigit->getEnergy();
        m_PSD13HadronEnergyFraction = (m_PSD13HadronEnergy / digitEnergy);
        m_PSD13DigitWeight  = relatedDigits.weight(next);
        ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();
        m_PSD13DigitFitType = digitFitType1;
        const int cellId = caldigit->getCellId();
        B2Vector3D calDigitPosition = geometry->GetCrystalPos(cellId - 1);
        TVector3 tempP = showerPosition - calDigitPosition;
        m_PSD13R = tempP.Mag();
        m_PSD13CosTheta = tempP.CosTheta();
        m_PSD13Phi = tempP.Phi();
      }

      if (14 < EnergyToSort.size()) {
        unsigned int digit = 14;
        const auto [digitEnergy, next] = EnergyToSort[digit];
        const auto caldigit = relatedDigits.object(next);
        m_PSD14HadronEnergy = caldigit->getTwoComponentHadronEnergy();
        m_PSD14OnlineEnergy = caldigit->getEnergy();
        m_PSD14HadronEnergyFraction = (m_PSD14HadronEnergy / digitEnergy);
        m_PSD14DigitWeight  = relatedDigits.weight(next);
        ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();
        m_PSD14DigitFitType = digitFitType1;
        const int cellId = caldigit->getCellId();
        B2Vector3D calDigitPosition = geometry->GetCrystalPos(cellId - 1);
        TVector3 tempP = showerPosition - calDigitPosition;
        m_PSD14R = tempP.Mag();
        m_PSD14CosTheta = tempP.CosTheta();
        m_PSD14Phi = tempP.Phi();
      }

      if (15 < EnergyToSort.size()) {
        unsigned int digit = 15;
        const auto [digitEnergy, next] = EnergyToSort[digit];
        const auto caldigit = relatedDigits.object(next);
        m_PSD15HadronEnergy = caldigit->getTwoComponentHadronEnergy();
        m_PSD15OnlineEnergy = caldigit->getEnergy();
        m_PSD15HadronEnergyFraction = (m_PSD15HadronEnergy / digitEnergy);
        m_PSD15DigitWeight  = relatedDigits.weight(next);
        ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();
        m_PSD15DigitFitType = digitFitType1;
        const int cellId = caldigit->getCellId();
        B2Vector3D calDigitPosition = geometry->GetCrystalPos(cellId - 1);
        TVector3 tempP = showerPosition - calDigitPosition;
        m_PSD15R = tempP.Mag();
        m_PSD15CosTheta = tempP.CosTheta();
        m_PSD15Phi = tempP.Phi();
      }

      if (16 < EnergyToSort.size()) {
        unsigned int digit = 16;
        const auto [digitEnergy, next] = EnergyToSort[digit];
        const auto caldigit = relatedDigits.object(next);
        m_PSD16HadronEnergy = caldigit->getTwoComponentHadronEnergy();
        m_PSD16OnlineEnergy = caldigit->getEnergy();
        m_PSD16HadronEnergyFraction = (m_PSD16HadronEnergy / digitEnergy);
        m_PSD16DigitWeight  = relatedDigits.weight(next);
        ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();
        m_PSD16DigitFitType = digitFitType1;
        const int cellId = caldigit->getCellId();
        B2Vector3D calDigitPosition = geometry->GetCrystalPos(cellId - 1);
        TVector3 tempP = showerPosition - calDigitPosition;
        m_PSD16R = tempP.Mag();
        m_PSD16CosTheta = tempP.CosTheta();
        m_PSD16Phi = tempP.Phi();
      }

      if (17 < EnergyToSort.size()) {
        unsigned int digit = 17;
        const auto [digitEnergy, next] = EnergyToSort[digit];
        const auto caldigit = relatedDigits.object(next);
        m_PSD17HadronEnergy = caldigit->getTwoComponentHadronEnergy();
        m_PSD17OnlineEnergy = caldigit->getEnergy();
        m_PSD17HadronEnergyFraction = (m_PSD17HadronEnergy / digitEnergy);
        m_PSD17DigitWeight  = relatedDigits.weight(next);
        ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();
        m_PSD17DigitFitType = digitFitType1;
        const int cellId = caldigit->getCellId();
        B2Vector3D calDigitPosition = geometry->GetCrystalPos(cellId - 1);
        TVector3 tempP = showerPosition - calDigitPosition;
        m_PSD17R = tempP.Mag();
        m_PSD17CosTheta = tempP.CosTheta();
        m_PSD17Phi = tempP.Phi();
      }

      if (18 < EnergyToSort.size()) {
        unsigned int digit = 18;
        const auto [digitEnergy, next] = EnergyToSort[digit];
        const auto caldigit = relatedDigits.object(next);
        m_PSD18HadronEnergy = caldigit->getTwoComponentHadronEnergy();
        m_PSD18OnlineEnergy = caldigit->getEnergy();
        m_PSD18HadronEnergyFraction = (m_PSD18HadronEnergy / digitEnergy);
        m_PSD18DigitWeight  = relatedDigits.weight(next);
        ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();
        m_PSD18DigitFitType = digitFitType1;
        const int cellId = caldigit->getCellId();
        B2Vector3D calDigitPosition = geometry->GetCrystalPos(cellId - 1);
        TVector3 tempP = showerPosition - calDigitPosition;
        m_PSD18R = tempP.Mag();
        m_PSD18CosTheta = tempP.CosTheta();
        m_PSD18Phi = tempP.Phi();
      }

      if (19 < EnergyToSort.size()) {
        unsigned int digit = 19;
        const auto [digitEnergy, next] = EnergyToSort[digit];
        const auto caldigit = relatedDigits.object(next);
        m_PSD19HadronEnergy = caldigit->getTwoComponentHadronEnergy();
        m_PSD19OnlineEnergy = caldigit->getEnergy();
        m_PSD19HadronEnergyFraction = (m_PSD19HadronEnergy / digitEnergy);
        m_PSD19DigitWeight  = relatedDigits.weight(next);
        ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();
        m_PSD19DigitFitType = digitFitType1;
        const int cellId = caldigit->getCellId();
        B2Vector3D calDigitPosition = geometry->GetCrystalPos(cellId - 1);
        TVector3 tempP = showerPosition - calDigitPosition;
        m_PSD19R = tempP.Mag();
        m_PSD19CosTheta = tempP.CosTheta();
        m_PSD19Phi = tempP.Phi();
      }
    }



    ///////////////
    if (!m_keep_only_truth_matched) {
      m_TTree -> Fill();
    }
    if ((m_keep_only_truth_matched) && (m_mcPDG == m_pdg_code)) {
      m_TTree -> Fill();
    }

  } // loop m_tracks
} // event

void ECLChargedPIDMVAExpertModule::endRun()
{
}

void ECLChargedPIDMVAExpertModule::terminate()
{
  //Write out root file
  m_TFile -> cd();
  m_TTree -> Write();
  m_TFile -> Write();
  m_TFile -> Close();
}