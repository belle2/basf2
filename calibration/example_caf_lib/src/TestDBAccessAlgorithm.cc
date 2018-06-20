#include <calibration/example_caf_lib/TestDBAccessAlgorithm.h>

#include <memory>

#include <TTree.h>
#include <TH1F.h>
#include <TRandom.h>
#include <TClonesArray.h>

#include <alignment/dataobjects/MilleData.h>
#include <calibration/dbobjects/TestCalibObject.h>
#include <calibration/dbobjects/TestCalibMean.h>

#include <framework/datastore/StoreObjPtr.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dataobjects/WireID.h>
#include <cdc/dbobjects/CDCGeometry.h>
#include <framework/gearbox/Gearbox.h>
#include <vector>
#include <numeric>

using namespace std;
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

  /***************************/
  /* Here's the key DB setup */
  /***************************/

  // Setup Gearbox
  Gearbox& gearbox = Gearbox::getInstance();

  std::vector<std::string> backends = {"file:"};
  gearbox.setBackends(backends);

  B2INFO("Start open gearbox.");
  gearbox.open("geometry/Belle2.xml");  // You may want to investigate if this is really the xml you want to open...
  B2INFO("Finished open gearbox.");

  // Construct an EventMetaData object in the Datastore so that the DB objects in CDCGeometryPar can work
  StoreObjPtr<EventMetaData> evtPtr;
  DataStore::Instance().setInitializeActive(true);
  evtPtr.registerInDataStore();
  DataStore::Instance().setInitializeActive(false);
  evtPtr.construct(0, 0,
                   1);  // Don't worry about the value, we'll change it later, though this will cause some warnings if there isn't a DB payload for this (exp,run)

  // Construct a CDCGeometryPar object which will update to the correct DB values when we change the EventMetaData and update
  // the Database instance
  GearDir cdcGearDir = Gearbox::getInstance().getDetectorComponent("CDC");
  CDCGeometry cdcGeometry;
  cdcGeometry.read(cdcGearDir);
  CDC::CDCGeometryPar::Instance(&cdcGeometry);

  // From this point on we can access the CDCGeometryPar instance, but we need to be careful
  // to update the exp/run numbers and database instance whenever necessary.

  // Some examples of what you could do...
//  saveNewT0ForEachRunFromTTree(ttree, evtPtr);
//  saveNewT0ForEachRunFromRunRange(evtPtr);
  saveNewT0FromAverageT0(evtPtr);

  // Closing gearbox may not be necessary in some situations but is probably good practice
  B2INFO("Closing Gearbox.");
  gearbox.close();
  // It's possible that clearing/resetting the Datastore is also sometimes necessary but I'm not sure


  // Iterate once.
  if (getIteration() < 1) {
    return c_Iterate;
  } else {
    return c_OK;
  }
}


void TestDBAccessAlgorithm::saveNewT0ForEachRunFromTTree(std::shared_ptr<TTree> ttree, StoreObjPtr<EventMetaData>& evtPtr)
{
  /* This function will use the exp and run numbers saved in the TTree from the collector
   * to update the Database as we iterate through the TTree. Then it creates and saves a new
   * CDCTimeZeroes payload  each time a run change is observed. BEWARE it does not account for the
   * same run appearing multiple times out of order.*/
  static CDC::CDCGeometryPar& cdcgeo = CDC::CDCGeometryPar::Instance();
  // Only do it for one wire as I'm lazy
  WireID wireid(0, 0);

  // Here we will be updating the DB objects as we loop over the entries encountering new (exp,run)
  int evt;
  int run;
  int exp;
  int old_run = -10;
  int old_exp = -10;
  ttree->SetBranchAddress("event", &evt);
  ttree->SetBranchAddress("run", &run);
  ttree->SetBranchAddress("exp", &exp);
  int t0;
  for (int entry = 0; entry < ttree->GetEntries(); ++entry) {
    ttree->GetEntry(entry);
    if (run != old_run || exp != old_exp) {
      // We changed run so we'd better update the EventMetaData and Database
      B2INFO("Changed ExpRun to: " << exp << " " << run);
      evtPtr->setExperiment(exp);
      evtPtr->setRun(run);
      DBStore::Instance().update();
      old_run = run;
      old_exp = exp;
      // Do anything that requires the constants in CDCGeometryPar to be correct below here!
      t0 = cdcgeo.getT0(wireid);
      // Create a new T0 that will be noticeable as we iterate
      int new_t0 = t0 + getIteration() + (run * 10);
      B2INFO("T0 found for ilay=0, iwire=0: " << t0);
      B2INFO("Setting T0 for ilay=0, iwire=0: to " << new_t0);
      CDCTimeZeros* tz = new CDCTimeZeros();
      tz->setT0(wireid, new_t0);
      // Note that we are explicitly saving with an IntervalOfValidity for the run we're looking at, this has NO EFFECT on the DB Access I'm showing off
      // DON'T DO THIS UNLESS YOU REALLY WANT IT! The Python CAF will execute over single runs for you instead if requested.
      // By default the IntervalOfValidity is set to the same as the one your algorithm was executed over
      saveCalibration(tz, "CDCTimeZeros", IntervalOfValidity(exp, run, exp, run));
    }
  }
}

void TestDBAccessAlgorithm::saveNewT0ForEachRunFromRunRange(StoreObjPtr<EventMetaData>& evtPtr)
{
  /* This function will use the exp and run numbers saved in the RunRange from the collector
   * to update the Database. This is always available even if you didn't save the exp, run from the
   * collector explicitly. Basically is a set of (exp,run) pairs that the
   * Algorithm was called over. Then it creates and saves a new CDCTimeZeroes payload for each run */
  static CDC::CDCGeometryPar& cdcgeo = CDC::CDCGeometryPar::Instance();
  WireID wireid(0, 0);

  // Here we will be updating the DB objects as we loop over the requested (exp,run) pairs for this execution of the algorithm
  // No need to have saved the Exp,Run in the collector
  int t0;
  for (auto exprun : getRunList()) {
    // New run so we'd better update the EventMetaData and Database
    B2INFO("Changed ExpRun to: " << exprun.first << " " << exprun.second);
    evtPtr->setExperiment(exprun.first);
    evtPtr->setRun(exprun.second);
    DBStore::Instance().update();
    // Do anything that requires the constants in CDCGeometryPar to be correct below here!
    t0 = cdcgeo.getT0(wireid);
    int new_t0 = t0 + getIteration() + (10 * exprun.second);
    B2INFO("T0 found for ilay=0, iwire=0: " << t0);
    B2INFO("Setting T0 for ilay=0, iwire=0: to " << new_t0);
    CDCTimeZeros* tz = new CDCTimeZeros();
    tz->setT0(wireid, new_t0);
    // Note that we are explicitly saving with an IntervalOfValidity, this has NO EFFECT on the DB Access I'm showing off
    // DON'T DO THIS UNLESS YOU REALLY WANT IT!
    // By default the IntervalOfValidity is set to the same as the one your algorithm was executed over
    saveCalibration(tz, "CDCTimeZeros", IntervalOfValidity(exprun.first, exprun.second, exprun.first, exprun.second));
  }
}


void TestDBAccessAlgorithm::saveNewT0FromAverageT0(StoreObjPtr<EventMetaData>& evtPtr)
{
  /* Same as saveNewT0ForEachRunFromRunRange() but instead creates an average T0 of all runs and them creates a single
   * new T0 for the overall IoV. */
  static CDC::CDCGeometryPar& cdcgeo = CDC::CDCGeometryPar::Instance();
  WireID wireid(0, 0);

  std::vector<int> vec_t0;
  // Here we will be updating the DB objects as we loop over the requested (exp,run) pairs for this execution of the algorithm
  // No need to have saved the Exp,Run in the collector
  int t0;
  for (auto exprun : getRunList()) {
    // New run so we'd better update the EventMetaData and Database
    B2INFO("Changed ExpRun to: " << exprun.first << " " << exprun.second);
    evtPtr->setExperiment(exprun.first);
    evtPtr->setRun(exprun.second);
    DBStore::Instance().update();
    // Do anything that requires the constants in CDCGeometryPar to be correct below here!
    t0 = cdcgeo.getT0(wireid);
    B2INFO("T0 found for ilay=0, iwire=0, Exp=" << exprun.first << ", Run=" << exprun.second << ": " << t0);
    vec_t0.push_back(t0);
  }
  int avg_t0 = std::accumulate(vec_t0.begin(), vec_t0.end(), 0) / vec_t0.size();
  B2INFO("Average T0 found for ilay=0, iwire=0: " << avg_t0);
  int new_t0 = avg_t0 + 10 + getIteration();
  B2INFO("Setting T0 for ilay=0, iwire=0: to " << new_t0);
  CDCTimeZeros* tz = new CDCTimeZeros();
  tz->setT0(wireid, new_t0);
  // Note that we are NOT explicitly saving with an IntervalOfValidity, this has NO EFFECT on the DB Access I'm showing off
  // By default the IntervalOfValidity is set to the same as the one your algorithm was executed over
  saveCalibration(tz, "CDCTimeZeros");
}
