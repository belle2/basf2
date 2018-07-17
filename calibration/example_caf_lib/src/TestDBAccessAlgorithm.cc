#include <calibration/example_caf_lib/TestDBAccessAlgorithm.h>

#include <memory>
#include <vector>
#include <numeric>
#include <algorithm>

#include <TTree.h>
#include <TRandom.h>

#include <calibration/dbobjects/TestCalibMean.h>
#include <framework/database/DBObjPtr.h>

using namespace Belle2;

TestDBAccessAlgorithm::TestDBAccessAlgorithm(): CalibrationAlgorithm("CaTest")
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

CalibrationAlgorithm::EResult TestDBAccessAlgorithm::calibrate()
{
  // Pulling in data from collector output. It now returns shared_ptr<T> so the underlying pointer
  // will delete itself automatically at the end of this scope unless you do something
  auto ttree = getObjectPtr<TTree>("MyTree");
  if (!ttree) return c_Failure;
  B2INFO("Number of Entries in MyTree was " << ttree->GetEntries());

  if (ttree->GetEntries() < 100)
    return c_NotEnoughData;

  // Iterate once.
  if (getIteration() < 1) {
    generateNewPayloads();
    return c_Iterate;
  } else {
    getAverageMean();
    generateNewPayloads();
    return c_OK;
  }
}

/// Grabs DBObjects from the Database and prints the mean of all the runs executed
void TestDBAccessAlgorithm::getAverageMean()
{
  DBObjPtr<TestCalibMean> dbMean;
  std::vector<float> vecMean, vecMeanError;
  for (auto expRun : getRunList()) {
    // Key command to make sure your DBObjPtrs are correct
    updateDBObjPtrs(1, expRun.second, expRun.first);

    B2INFO("Mean from DB found for (Exp, Run) : ("
           << expRun.first << "," << expRun.second << ") = "
           << dbMean->getMean());
    B2INFO("MeanError from DB found for (Exp, Run) : ("
           << expRun.first << "," << expRun.second << ") = "
           << dbMean->getMeanError());
  }
}

void TestDBAccessAlgorithm::generateNewPayloads()
{
  for (auto expRun : getRunList()) {
    float mean = gRandom->Gaus(42.0, 5.0);
    float meanError = gRandom->Gaus(5.0, 0.2);
    // Example of saving a DBObject.
    B2INFO("Saving new Mean for (Exp, Run) : ("
           << expRun.first << "," << expRun.second << ") = "
           << mean);
    B2INFO("Saving new MeanError for (Exp, Run) : ("
           << expRun.first << "," << expRun.second << ") = "
           << meanError);
    TestCalibMean* dbMean = new TestCalibMean(mean, meanError);
    saveCalibration(dbMean, "TestCalibMean");
  }
}
