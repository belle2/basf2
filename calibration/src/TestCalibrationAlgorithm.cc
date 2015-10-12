#include <calibration/TestCalibrationAlgorithm.h>

#include <TProfile.h>
#include <TRandom.h>

using namespace std;
using namespace Belle2;
using namespace calibration;


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

CalibrationAlgorithm::E_Result TestCalibrationAlgorithm::calibrate()
{
  auto& histogram1 = getObject<TH1F>("histogram1");
  auto& ttree = getObject<TTree>("tree");
  auto& mille = getObject<MilleData>("test_mille");

  if (histogram1.GetEntries() < 100 || ttree.GetEntries() < 100 || mille.getFiles().empty())
    return calibration::CalibrationAlgorithm::c_NotEnoughData;

  Double_t mean = histogram1.GetMean();
  Double_t meanerror = histogram1.GetMeanError();

  if (meanerror <= 0.)
    return Belle2::calibration::CalibrationAlgorithm::c_Failure;

  static int nameDistinguisher(0);
  TH1I* correction = new TH1I(TString(string("constant-in-histo") + to_string(nameDistinguisher)),
                              "Histogram should have one entry at mean value of calibration test histo", 100, 0, 100);
  nameDistinguisher++;
  correction->Fill((int)mean);

  saveCalibration(correction, "test_constant");

  // Iterate until we find answer to the most fundamental question...
  if (mean - 42. >= 1.)
    return calibration::CalibrationAlgorithm::c_Iterate;

  return calibration::CalibrationAlgorithm::c_OK;
}