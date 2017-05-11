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
  // Pulling in data from collector output
  auto& histogram1 = getObject<TH1F>("histogram1");
  auto& ttree = getObject<TTree>("tree");
  auto& mille = getObject<MilleData>("test_mille");

  if (histogram1.GetEntries() < 100 || ttree.GetEntries() < 100 || mille.getFiles().empty())
    return c_NotEnoughData;

  Double_t mean = histogram1.GetMean();
  Double_t meanerror = histogram1.GetMeanError();

  if (meanerror <= 0.)
    return c_Failure;

  // Example of saving a DBObject. In this case it is a DBObject derived from TH1I
  string mean_name = "TestCalibMean_";
  static int nameDistinguisher(0);  // ROOT's habit of holding onto object references forever.
  mean_name += to_string(nameDistinguisher);

  TestCalibMean* correction = new TestCalibMean(mean_name, mean);  // Not really certain whose job it is to delete this...
  saveCalibration(correction, "TestCalibMean");
  nameDistinguisher++;

  // Example of using a Belle2 DBArray of DBObjects defined in the dbobjects directory
  TClonesArray* exampleDBArrayConstants = new TClonesArray("Belle2::TestCalibObject", 2);
  float val = 0.0;
  for (int i = 0; i < 2; i++) {
    val += 1.0;
    new((*exampleDBArrayConstants)[i]) TestCalibObject(val);
  }
  saveCalibration(exampleDBArrayConstants, "TestCalibObjects");

  // Iterate until we find answer to the most fundamental question...
  B2INFO("mean: " << mean);
  if (mean - 42. >= 1.)
    return c_Iterate;

  return c_OK;
}
