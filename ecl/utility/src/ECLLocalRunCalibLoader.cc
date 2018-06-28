/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 *                                                                        *
 * ECLLocalRunCalibLoader                                                 *
 *                                                                        *
 * ECLLocalRunCalibLoader is designed to load tree of the local           *
 * run calibration result from a database.                                *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su) (BINP)         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <iostream>
// ECL
#include <ecl/dbobjects/ECLCrystalLocalRunCalib.h>
#include <ecl/dbobjects/ECLLocalRunCalibRef.h>
#include <ecl/utility/ECLLocalRunCalibLoader.h>
#include <ecl/utility/ECLDBTool.h>
// ROOT
#include <TDatime.h>
#include <TLeaf.h>
using namespace Belle2;
// Number of cell ids.
const int
ECLLocalRunCalibLoader::c_ncellids = 8736;
const std::string
// Time payload name.
ECLLocalRunCalibLoader::c_timePayloadName =
  "ECLCalibTime";
// Amplitude payload name.
const std::string
ECLLocalRunCalibLoader::c_amplPayloadName =
  "ECLCalibAmplitude";
// Payload name of a reference mark.
const std::string
ECLLocalRunCalibLoader::c_refPayloadName =
  "ECLCalibRef";
// Constructor.
ECLLocalRunCalibLoader::ECLLocalRunCalibLoader(
  bool isLocal,
  const char* dbName,
  const char* timeFilePath):
  c_isLocal(isLocal),
  c_dbName(dbName),
  c_timeFilePath(timeFilePath)
{
}
// Destructor.
ECLLocalRunCalibLoader::~ECLLocalRunCalibLoader()
{
}
// Get experiment and run numbers for reference run
void ECLLocalRunCalibLoader::getReference(const int& exp,
                                          const int& run,
                                          int* ref_exp,
                                          int* ref_run) const
{
  ECLDBTool payload(c_isLocal,
                    c_dbName.c_str(),
                    c_refPayloadName.c_str());
  EventMetaData metadata(1, run, exp);
  TObject* obj;
  IntervalOfValidity* iov;
  payload.connect();
  payload.read(&obj, &iov, metadata);
  ECLLocalRunCalibRef* ref =
    static_cast<ECLLocalRunCalibRef*>(obj);
  *ref_exp = ref->getExp();
  *ref_run = ref->getRun();
  delete obj;
  delete iov;
}
// Get tree.
void ECLLocalRunCalibLoader::getTree(TTree** tree,
                                     const std::vector <
                                     std::pair<int, int> > & runs,
                                     bool withref) const
{
  int exp;
  int run;
  int ref_run;
  int ref_exp;
  int cellid;
  int time_count;
  float time_mean;
  float time_stddev;
  int ref_time_count;
  float ref_time_mean;
  float ref_time_stddev;
  int ampl_count;
  float ampl_mean;
  float ampl_stddev;
  int ref_ampl_count;
  float ref_ampl_mean;
  float ref_ampl_stddev;
  *tree = new TTree("calib_tree", "");
  (*tree)->Branch("cellid", &cellid, "cellid/I");
  (*tree)->Branch("run", &run, "run/I");
  (*tree)->Branch("exp", &exp, "exp/I");
  (*tree)->Branch("time_count", &time_count, "time_count/I");
  (*tree)->Branch("time_mean", &time_mean, "time_mean/F");
  (*tree)->Branch("time_stddev", &time_stddev, "time_stddev/F");
  (*tree)->Branch("ampl_count", &ampl_count, "ampl_count/I");
  (*tree)->Branch("ampl_mean", &ampl_mean, "ampl_mean/F");
  (*tree)->Branch("ampl_stddev", &ampl_stddev, "ampl_stddev/F");
  if (withref) {
    (*tree)->Branch("ref_run", &ref_run, "ref_run/I");
    (*tree)->Branch("ref_exp", &ref_exp, "ref_exp/I");
    (*tree)->Branch("ref_time_count", &ref_time_count, "ref_time_count/I");
    (*tree)->Branch("ref_time_mean", &ref_time_mean, "ref_time_mean/F");
    (*tree)->Branch("ref_time_stddev", &ref_time_stddev, "ref_time_stddev/F");
    (*tree)->Branch("ref_ampl_count", &ref_ampl_count, "ref_ampl_count/I");
    (*tree)->Branch("ref_ampl_mean", &ref_ampl_mean, "ref_ampl_mean/F");
    (*tree)->Branch("ref_ampl_stddev", &ref_ampl_stddev, "ref_ampl_stddev/F");
  }
  for (const auto& rundata : runs) {
    exp = rundata.first;
    run = rundata.second;
    ECLDBTool time_payload(c_isLocal,
                           c_dbName.c_str(),
                           c_timePayloadName.c_str());
    ECLDBTool ampl_payload(c_isLocal,
                           c_dbName.c_str(),
                           c_amplPayloadName.c_str());
    EventMetaData metadata(1, run, exp);
    TObject* obj;
    IntervalOfValidity* iov;
    time_payload.connect();
    time_payload.read(&obj, &iov, metadata);
    ECLCrystalLocalRunCalib* time_calib =
      static_cast<ECLCrystalLocalRunCalib*>(obj);
    delete iov;
    ampl_payload.connect();
    ampl_payload.read(&obj, &iov, metadata);
    ECLCrystalLocalRunCalib* ampl_calib =
      static_cast<ECLCrystalLocalRunCalib*>(obj);
    delete iov;
    ECLCrystalLocalRunCalib* time_ref = nullptr;
    ECLCrystalLocalRunCalib* ampl_ref = nullptr;
    if (withref) {
      getReference(exp, run,
                   &ref_exp, &ref_run);
      EventMetaData ref_metadata(1, ref_run, ref_exp);
      time_payload.connect();
      time_payload.read(&obj, &iov, ref_metadata);
      time_ref =
        static_cast<ECLCrystalLocalRunCalib*>(obj);
      delete iov;
      ampl_payload.connect();
      ampl_payload.read(&obj, &iov, ref_metadata);
      ampl_ref =
        static_cast<ECLCrystalLocalRunCalib*>(obj);
      delete iov;
    }
    for (int i = 0; i < c_ncellids; ++i) {
      cellid = i + 1;
      time_count = (time_calib->getNumbersOfAcceptedEvents())[i];
      time_mean = (time_calib->getCalibVector())[i];
      time_stddev = (time_calib->getCalibUncVector())[i];
      ampl_count = (ampl_calib->getNumbersOfAcceptedEvents())[i];
      ampl_mean = (ampl_calib->getCalibVector())[i];
      ampl_stddev = (ampl_calib->getCalibUncVector())[i];
      if (withref) {
        ref_time_count = (time_ref->getNumbersOfAcceptedEvents())[i];
        ref_time_mean = (time_ref->getCalibVector())[i];
        ref_time_stddev = (time_ref->getCalibUncVector())[i];
        ref_ampl_count = (ampl_ref->getNumbersOfAcceptedEvents())[i];
        ref_ampl_mean = (ampl_ref->getCalibVector())[i];
        ref_ampl_stddev = (ampl_ref->getCalibUncVector())[i];
      }
      (*tree)->Fill();
    }
    delete time_calib;
    delete ampl_calib;
    if (time_ref) {
      delete time_ref;
    }
    if (ampl_ref) {
      delete ampl_ref;
    }
  }
  (*tree)->SetDirectory(0);
}
// Get tree.
void ECLLocalRunCalibLoader::getTree(
  TTree** tree,
  const std::string& sqltime_begin,
  const std::string& sqltime_end,
  bool withref) const
{
  TDatime time_begin(sqltime_begin.c_str());
  TDatime time_end(sqltime_end.c_str());
  TTree* timeline = new TTree("timeline", "");
  timeline->ReadFile(c_timeFilePath.c_str(), "", ',');
  int run;
  int exp;
  UInt_t run_start;
  UInt_t run_end;
  int ref_run;
  int ref_exp;
  int cellid;
  int time_count;
  float time_mean;
  float time_stddev;
  int ref_time_count;
  float ref_time_mean;
  float ref_time_stddev;
  int ampl_count;
  float ampl_mean;
  float ampl_stddev;
  int ref_ampl_count;
  float ref_ampl_mean;
  float ref_ampl_stddev;
  *tree = new TTree("calib_tree", "");
  (*tree)->Branch("run_start", &run_start);
  (*tree)->Branch("run_end", &run_end);
  (*tree)->Branch("cellid", &cellid, "cellid/I");
  (*tree)->Branch("run", &run, "run/I");
  (*tree)->Branch("exp", &exp, "exp/I");
  (*tree)->Branch("time_count", &time_count, "time_count/I");
  (*tree)->Branch("time_mean", &time_mean, "time_mean/F");
  (*tree)->Branch("time_stddev", &time_stddev, "time_stddev/F");
  (*tree)->Branch("ampl_count", &ampl_count, "ampl_count/I");
  (*tree)->Branch("ampl_mean", &ampl_mean, "ampl_mean/F");
  (*tree)->Branch("ampl_stddev", &ampl_stddev, "ampl_stddev/F");
  if (withref) {
    (*tree)->Branch("ref_run", &ref_run, "ref_run/I");
    (*tree)->Branch("ref_exp", &ref_exp, "ref_exp/I");
    (*tree)->Branch("ref_time_count", &ref_time_count, "ref_time_count/I");
    (*tree)->Branch("ref_time_mean", &ref_time_mean, "ref_time_mean/F");
    (*tree)->Branch("ref_time_stddev", &ref_time_stddev, "ref_time_stddev/F");
    (*tree)->Branch("ref_ampl_count", &ref_ampl_count, "ref_ampl_count/I");
    (*tree)->Branch("ref_ampl_mean", &ref_ampl_mean, "ref_ampl_mean/F");
    (*tree)->Branch("ref_ampl_stddev", &ref_ampl_stddev, "ref_ampl_stddev/F");
  }
  int N = timeline->GetEntries();
  for (int k = 0; k < N; ++k) {
    timeline->GetEntry(k);
    run =
      *static_cast<int*>(timeline->
                         GetLeaf("run")->
                         GetValuePointer());
    exp =
      *static_cast<int*>(timeline->
                         GetLeaf("exp")->
                         GetValuePointer());
    auto run_s =
      static_cast<const char*>(timeline->
                               GetLeaf("run_start")->
                               GetValuePointer());
    auto run_e =
      static_cast<const char*>(timeline->
                               GetLeaf("run_end")->
                               GetValuePointer());
    TDatime cur_start(run_s);
    TDatime cur_end(run_e);
    if (cur_end < cur_start) {
      // TODO: raise an error here
      return;
    }
    run_start = cur_start.Convert();
    run_end = cur_end.Convert();
    if (cur_start >= time_begin &&
        cur_end <= time_end) {
      ECLDBTool time_payload(c_isLocal,
                             c_dbName.c_str(),
                             c_timePayloadName.c_str());
      ECLDBTool ampl_payload(c_isLocal,
                             c_dbName.c_str(),
                             c_amplPayloadName.c_str());
      EventMetaData metadata(1, run, exp);
      IntervalOfValidity* iov;
      TObject* obj;
      time_payload.connect();
      time_payload.read(&obj, &iov, metadata);
      ECLCrystalLocalRunCalib* time_calib =
        static_cast<ECLCrystalLocalRunCalib*>(obj);
      delete iov;
      ampl_payload.connect();
      ampl_payload.read(&obj, &iov, metadata);
      ECLCrystalLocalRunCalib* ampl_calib =
        static_cast<ECLCrystalLocalRunCalib*>(obj);
      delete iov;
      ECLCrystalLocalRunCalib* time_ref = nullptr;
      ECLCrystalLocalRunCalib* ampl_ref = nullptr;
      if (withref) {
        getReference(exp, run,
                     &ref_exp, &ref_run);
        EventMetaData ref_metadata(1, ref_run, ref_exp);
        time_payload.connect();
        time_payload.read(&obj, &iov, ref_metadata);
        time_ref =
          static_cast<ECLCrystalLocalRunCalib*>(obj);
        delete iov;
        ampl_payload.connect();
        ampl_payload.read(&obj, &iov, ref_metadata);
        ampl_ref =
          static_cast<ECLCrystalLocalRunCalib*>(obj);
        delete iov;
      }
      for (int i = 0; i < c_ncellids; ++i) {
        cellid = i + 1;
        time_count = (time_calib->getNumbersOfAcceptedEvents())[i];
        time_mean = (time_calib->getCalibVector())[i];
        time_stddev = (time_calib->getCalibUncVector())[i];
        ampl_count = (ampl_calib->getNumbersOfAcceptedEvents())[i];
        ampl_mean = (ampl_calib->getCalibVector())[i];
        ampl_stddev = (ampl_calib->getCalibUncVector())[i];
        if (withref) {
          ref_time_count = (time_ref->getNumbersOfAcceptedEvents())[i];
          ref_time_mean = (time_ref->getCalibVector())[i];
          ref_time_stddev = (time_ref->getCalibUncVector())[i];
          ref_ampl_count = (ampl_ref->getNumbersOfAcceptedEvents())[i];
          ref_ampl_mean = (ampl_ref->getCalibVector())[i];
          ref_ampl_stddev = (ampl_ref->getCalibUncVector())[i];
        }
        (*tree)->Fill();
      }
      delete time_calib;
      delete ampl_calib;
      if (time_ref) {
        delete time_ref;
      }
      if (ampl_ref) {
        delete ampl_ref;
      }
    }
  }
  (*tree)->SetDirectory(0);
  delete timeline;
}
