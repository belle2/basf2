/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <calibration/example_caf_lib/TestDBAccessAlgorithm.h>

#include <memory>
#include <vector>
#include <numeric>

#include <TTree.h>
#include <TRandom.h>

using namespace Belle2;

TestDBAccessAlgorithm::TestDBAccessAlgorithm(): CalibrationAlgorithm("CaTest")
{
  setDescription(
    " -------------------------- Test Calibration Algoritm -------------------------\n"
    "                                                                               \n"
    "  Testing algorithm which accesses DBObjPtr to show how it could be done.      \n"
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

  // Don't generate payloads if this isn't our first try (even if you set it to).
  if (getIteration() > 0) {
    setGeneratePayloads(false);
  }

  // If this was our first try and you did want to generate payloads, do it.
  if (getGeneratePayloads()) {
    generateNewPayloads();
    return c_Iterate;
  } else {
    float distance = getAverageDistanceFromAnswer();
    if (distance < 1.0) {
      saveSameMeans();
      return c_OK;
    } else {
      reduceDistancesAndSave();
      return c_Iterate;
    }
  }
}

/// Saves new DB values for each run where they are a little closer to 42
void TestDBAccessAlgorithm::reduceDistancesAndSave()
{
  for (auto expRun : getRunList()) {
    // Key command to make sure your DBObjPtrs are correct
    updateDBObjPtrs(1, expRun.second, expRun.first);

    B2INFO("Mean from DB found for (Exp, Run) : ("
           << expRun.first << "," << expRun.second << ") = "
           << m_dbMean->getMean());

    float mean = m_dbMean->getMean();
    mean = ((42.0 - mean) / 3.) + mean;
    float meanError = m_dbMean->getMeanError();

    B2INFO("New Mean from DB found for (Exp, Run) : ("
           << expRun.first << "," << expRun.second << ") = "
           << mean);

    TestCalibMean* dbMean = new TestCalibMean(mean, meanError);
    saveCalibration(dbMean);
  }
}

/// Saves the same DB values for each run into a new localdb, otherwise we aren't creating a DB this iteration
void TestDBAccessAlgorithm::saveSameMeans()
{
  for (auto expRun : getRunList()) {
    // Key command to make sure your DBObjPtrs are correct
    updateDBObjPtrs(1, expRun.second, expRun.first);

    B2INFO("Mean from DB found for (Exp, Run) : ("
           << expRun.first << "," << expRun.second << ") = "
           << m_dbMean->getMean());

    float mean = m_dbMean->getMean();
    float meanError = m_dbMean->getMeanError();

    TestCalibMean* dbMean = new TestCalibMean(mean, meanError);
    saveCalibration(dbMean);
  }
}

/// Grabs DBObjects from the Database and finds out the average distance from 42.
float TestDBAccessAlgorithm::getAverageDistanceFromAnswer()
{
  std::vector<float> vecDist;
  for (auto expRun : getRunList()) {
    // Key command to make sure your DBObjPtrs are correct
    updateDBObjPtrs(1, expRun.second, expRun.first);

    B2INFO("Mean from DB found for (Exp, Run) : ("
           << expRun.first << "," << expRun.second << ") = "
           << m_dbMean->getMean());

    vecDist.push_back(42.0 - m_dbMean->getMean());
  }
  return std::accumulate(std::begin(vecDist), std::end(vecDist), 0.0) / vecDist.size();
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
    saveCalibration(dbMean, IntervalOfValidity(expRun.first, expRun.second, expRun.first, expRun.second));
  }
}
