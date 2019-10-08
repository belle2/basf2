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
  if (mean - 42. >= 1.) {
    return c_Iterate;
  } else {
    return c_OK;
  }
}
