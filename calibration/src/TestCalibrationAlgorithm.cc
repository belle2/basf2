#include <calibration/TestCalibrationAlgorithm.h>

#include <TProfile.h>
#include <TRandom.h>
#include <TClonesArray.h>
#include <calibration/dbobjects/TestCalibObject.h>
#include <calibration/dbobjects/TestCalibMean.h>


using namespace std;
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

CalibrationAlgorithm::EResult TestCalibrationAlgorithm::calibrate()
{
//  // Pulling in data from collector output, we only use the histogram in this test
//  auto& histogram1 = getObject<TH1F>("histogram1");
//  auto& ttree = getObject<TTree>("tree");
//  auto& mille = getObject<MilleData>("test_mille");
//
//  if (histogram1.GetEntries() < 100 || ttree.GetEntries() < 100 || mille.getFiles().empty())
//    return c_NotEnoughData;
//
//  Float_t mean = histogram1.GetMean();
//  Float_t meanError = histogram1.GetMeanError();
//
//  if (meanError <= 0.)
//    return c_Failure;
//
//  // Example of saving a DBObject.
//  TestCalibMean* correction = new TestCalibMean(mean, meanError);
//  saveCalibration(correction, "TestCalibMean");
//
//  // Example of saving a Belle2 DBArray of DBObjects defined in the dbobjects directory
//  TClonesArray* exampleDBArrayConstants = new TClonesArray("Belle2::TestCalibObject", 2);
//  float val = 0.0;
//  for (int i = 0; i < 2; i++) {
//    val += 1.0;
//    new((*exampleDBArrayConstants)[i]) TestCalibObject(val);
//  }
//  saveCalibration(exampleDBArrayConstants, "TestCalibObjects");
//
//  // Iterate until we find answer to the most fundamental question...
//  B2INFO("mean: " << mean);
//  B2INFO("meanError: " << meanError);
//  if (mean - 42. >= 1.)
//    return c_Iterate;
//
  return c_OK;
}
