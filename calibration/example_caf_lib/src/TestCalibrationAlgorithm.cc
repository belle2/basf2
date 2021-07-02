/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <calibration/example_caf_lib/TestCalibrationAlgorithm.h>

#include <memory>

#include <TTree.h>
#include <TH1F.h>
#include <TClonesArray.h>
#include <TRandom.h>


#include <alignment/dataobjects/MilleData.h>
#include <calibration/dbobjects/TestCalibObject.h>
#include <calibration/dbobjects/TestCalibMean.h>

using namespace Belle2;
using namespace Calibration;

TestCalibrationAlgorithm::TestCalibrationAlgorithm(): CalibrationAlgorithm("CaTest")
{
  setDescription(
    " -------------------------- Test Calibration Algoritm -------------------------\n"
    "                                                                               \n"
    "  Testing algorithm which just gets mean of a test histogram collected by      \n"
    "  CaTest module and provides a DB object with another histogram with one       \n"
    "  entry at calibrated value.                                                   \n"
    " ------------------------------------------------------------------------------\n"
  );
}

void TestCalibrationAlgorithm::createDebugHistogram()
{
  std::string debugFileName = (this->getPrefix()) + "debug.root";
  B2INFO("Storing histograms in " << debugFileName);

  // Save the current directory to change back later
  TDirectory* currentDir = gDirectory;
  TFile* debugFile = new TFile(debugFileName.c_str(), "RECREATE");
  debugFile->cd();
  TH1F* debugHisto = new TH1F("h1", "h1", 30, -3., 3.);
  for (int i = 0; i < 1000; ++i) {
    debugHisto->Fill(gRandom->Gaus());
  }

  debugHisto->Write();
  debugFile->Close();
  currentDir->cd();
}

CalibrationAlgorithm::EResult TestCalibrationAlgorithm::calibrate()
{
  // Some calibrations may want to pass information from one execution to the next.
  // We do this via a json object that is created like a std::map (or Python dictionary).
  // You will have set this json object up manually by calling loadInputJson(string),
  // or the Python CAF may do this for you.
  //
  // USING THIS FEATURE IS OPTIONAL!!!
  //
  // It is only for cases where you really need to pass a message from one execution of the algorithm
  // to the next. It is not supposed to be for all of your config variables. Use member variables for
  // configuring your algorithm!

  // You can pull in from the Input JSON like so
  // First you must know the string used to label the value, in this case we save the previous calculated mean
  const std::string previousMeanKey = "previous_mean";
  // Test if the key exists in our inputJson object (was it set?)
  if (inputJsonKeyExists(previousMeanKey)) {
    const float prevMean = getOutputJsonValue<float>(previousMeanKey);
    B2INFO("An input JSON object was set with the previous calculated mean = " << prevMean);
  }

  // Pulling in data from collector output. It now returns shared_ptr<T> so the underlying pointer
  // will delete itself automatically at the end of this scope unless you do something
  auto ttree = getObjectPtr<TTree>("MyTree");
  auto hist = getObjectPtr<TH1F>("MyHisto");
  auto mille = getObjectPtr<MilleData>("test_mille");
  if (!ttree) return c_Failure;
  if (!hist) return c_Failure;
  if (!mille) return c_Failure;
  B2INFO("Number of Entries in MyTree was " << ttree->GetEntries());
  B2INFO("Number of Entries in MyHisto was " << hist->GetEntries());

  float mean = hist->GetMean();
  float meanError = hist->GetMeanError();

  B2INFO("Mean of MyHisto was " << mean);
  B2INFO("Mean Error of MyHisto was " << meanError);

  // Create debugging histo if we asked for it
  if (m_debugHisto) createDebugHistogram();

  // Fail if we we're asked to. Useful for testing
  if (getForceFail()) {
    B2WARNING("We were asked to fail by the m_willFail flag. Failing");
    return c_Failure;
  }

  for (auto& fileName : mille->getFiles()) {
    B2INFO("Stored Mille binary file: " << fileName);
  }

  if (hist->GetEntries() < 100 || ttree->GetEntries() < getMinEntries() || mille->getFiles().empty())
    return c_NotEnoughData;

  // Example of saving a DBObject.
  TestCalibMean* correction = new TestCalibMean(mean, meanError);
  saveCalibration(correction, "TestCalibMean");

  // Example of saving a Belle2 DBArray of DBObjects defined in the dbobjects directory
  TClonesArray* exampleDBArrayConstants = new TClonesArray("Belle2::TestCalibObject", 2);
  float val = 0.0;
  for (int i = 0; i < 2; i++) {
    val += 1.0;
    new((*exampleDBArrayConstants)[i]) TestCalibObject(val);
  }
  saveCalibration(exampleDBArrayConstants, "TestCalibObjects");
  // Iterate until we find answer to the most fundamental question...
  B2INFO("mean: " << mean);
  B2INFO("meanError: " << meanError);

  // Remember how we optionally had an input JSON object? Well this is how you can pass out JSON information
  // to the Python CAF. You need to set the information into the object
  setOutputJsonValue<float>(previousMeanKey, mean);

  // Decide if we need to iterate or if this value was fine
  if (mean - 42. >= 1.) {
    return c_Iterate;
  } else {
    return c_OK;
  }
}

bool TestCalibrationAlgorithm::isBoundaryRequired(const ExpRun& currentRun)
{
  auto hist = getObjectPtr<TH1F>("MyHisto");
  float mean = hist->GetMean();
  // First run?
  if (!m_previousMean) {
    B2INFO("This is the first run encountered, let's say it is a boundary.");
    B2INFO("Initial mean was " << mean);
    m_previousMean.emplace(mean);
    return true;
  }
  // Shifted since last time?
  else if ((mean - m_previousMean.value()) > m_allowedMeanShift) {
    B2INFO("Histogram mean has shifted from " << m_previousMean.value()
           << " to " << mean << ". We are requesting a new payload boundary for ("
           << currentRun.first << "," << currentRun.second << ")");
    m_previousMean.emplace(mean);
    return true;
  } else {
    return false;
  }
}
