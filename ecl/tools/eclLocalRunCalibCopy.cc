/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 *                                                                        *
 * eclLocalRunCalibCopy                                                   *
 *                                                                        *
 * This tool has been designed in order to copy good quality local run    *
 * calibration content from one database to another one.                  *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su) (BINP)         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <sys/stat.h>
// STL
#include <iostream>
#include <fstream>
#include <list>
#include <vector>
// BOOST
#include <boost/program_options.hpp>
// ROOT
#include <TTree.h>
#include <TLeaf.h>
// ECL
#include <ecl/utility/ECLDBTool.h>
#include <ecl/utility/ECLLocalRunCalibLoader.h>
#include <ecl/utility/ECLLocalRunCalibQuality.h>
#include <ecl/utility/ECLLocalRunCalibQualityChecker.h>
using namespace Belle2;
namespace po = boost::program_options;
struct commandOptions {
  // if m_centraldb_src is true,
  // source database will be a central
  // database
  bool m_centraldb_src = false;
  // if m_centraldb_dest is true,
  // destination database will be
  // a central database
  bool m_centraldb_dest = false;
  // Tag of the source database in
  // the case of the central database
  // ussage or path to a local source database
  // in the case of a local database
  // ussage.
  std::string m_dbname_src = "localdb/database.txt";
  // Tag of the destination database
  // in the case of the central database ussage
  // or path to a local destination database in
  // the case of a local database ussage
  std::string m_dbname_dest = "localdb_new/database.txt";
  // Path to a source run time table.
  std::string m_timeFilePath_src = "timetab.cvs";
  // Path to a destination run time table.
  std::string m_timeFilePath_dest = "timetab.cvs";
  // The destination run time table will be recreated, if
  // m_reset_table is equal to true. Otherwise the destination
  // run time table will be updated.
  bool m_reset_table = false;
  // Experiment number.
  int m_exp;
  // Low run.
  int m_lowrun;
  // Minimum allowed number of events.
  int m_minNOfEvs = 100;
  // High run.
  int m_highrun;
  // time low count ratio
  float m_lowCountRatioTime = 0.98;
  // amplitude low count ratio
  float m_lowCountRatioAmpl = 0.98;
  // time max offset
  float m_timeMaxOffset = 20.;
  // ampl max offset
  float m_amplMaxOffset = 0.003;
  // min time mean
  float m_minMeanTime = -1.5e+03;
  // min amplitude mean
  float m_minMeanAmpl = 0.;
  // max time mean
  float m_maxMeanTime = 1.5e+03;
  // max amplitude mean
  float m_maxMeanAmpl = 1.e+05;
};
struct timeTableRow {
  // experiment number
  int m_exp;
  // run number
  int m_run;
  // begin time
  std::string m_run_start;
  // end time
  std::string m_run_end;
  // number of events
  int m_evt_count;
};
void setOptions(po::options_description* desc,
                commandOptions* opts)
{
  desc->add_options()
  ("help", "Produce a help message.")
  ("src-centraldb",
   po::bool_switch(&(opts->m_centraldb_src)),
   "Use a central database as source database.")
  ("dest-centraldb",
   po::bool_switch(&(opts->m_centraldb_dest)),
   "Use a cental database as destination database.")
  ("src-dbname",
   po::value<std::string>(&(opts->m_dbname_src)),
   "Tag or path for a source database.")
  ("dest-dbname",
   po::value<std::string>(&(opts->m_dbname_dest)),
   "Tag or path for a destination database.")
  ("src-timetab",
   po::value<std::string>(&(opts->m_timeFilePath_src)),
   "Source table of run times.")
  ("dest-timetab",
   po::value<std::string>(&(opts->m_timeFilePath_dest)),
   "Destination table of run times.")
  ("dest-table-reset",
   po::bool_switch(&(opts->m_reset_table)),
   "Recreate destination run time table.")
  ("exp", po::value<int>(&opts->m_exp),
   "Experiment number.")
  ("lowrun", po::value<int>(&opts->m_lowrun),
   "Low run.")
  ("highrun", po::value<int>(&opts->m_highrun),
   "High run.")
  ("minnofevs", po::value<int>(&opts->m_minNOfEvs),
   "Minimum allowed number of events.")
  ("timelowcount", po::value<float>(&opts->m_lowCountRatioTime),
   "Low count ratio for time.")
  ("ampllowcount", po::value<float>(&opts->m_lowCountRatioAmpl),
   "Low count ratio for amplitude.")
  ("timemaxoffset", po::value<float>(&opts->m_timeMaxOffset),
   "Maximum allowed time offset.")
  ("amplmaxoffset", po::value<float>(&opts->m_amplMaxOffset),
   "Maximum allowed amplitude offset.")
  ("mintime", po::value<float>(&opts->m_minMeanTime),
   "Minimum allowed mean time value.")
  ("minampl", po::value<float>(&opts->m_minMeanAmpl),
   "Minimum allowed mean amplitude value.")
  ("maxtime", po::value<float>(&opts->m_maxMeanTime),
   "Maximum allowed mean time value.")
  ("maxampl", po::value<float>(&opts->m_maxMeanAmpl),
   "Maximum allowed mean amplitude value.");
}
bool isReady(const po::variables_map& vm)
{
  return
    vm.count("exp") &&
    vm.count("lowrun") &&
    vm.count("highrun");
}
void printQuality(const std::list<ECLLocalRunCalibQuality>& qualities)
{
  std::cout << "Bad quality runs:" << std::endl;
  for (const auto& el : qualities) {
    std::cout << "=====================" << std::endl;
    std::cout << "experiment: " << el.getExpNumber() << std::endl;
    std::cout << "run: " << el.getRunNumber() << std::endl;
    std::cout << "reference run: " << el.getReferenceRunNumber() << std::endl;
    std::cout << "number of events: " <<
              el.getNOfEvents() << std::endl;
    if (el.isTooSmallNOfEvs()) {
      std::cout << "[!] Number of event is too small!" << std::endl;
    }
    if (el.isNegAmpl()) {
      std::cout << "[!] There are negative amplitudes!" << std::endl;
    }
    std::cout << "number of channels with low time count: " <<
              el.getNOfBadTimeCountCh() << std::endl;
    std::cout << "number of channels with low amplitude count: " <<
              el.getNOfBadAmplCountCh() << std::endl;
    std::cout << "number of channels with large time offset: " <<
              el.getNOfLargeTimeOffsetCh() << std::endl;
    std::cout << "number of channels with large amplitude offset: " <<
              el.getNOfLargeAmplOffsetCh() << std::endl;
    std::cout << "number of channels, where time mean" << std::endl;
    std::cout << "value is outside of allowed limits: " <<
              el.getNOfAbsMeanTimeChOutRange() << std::endl;
    std::cout << "number of channels, where amplitude mean" << std::endl;
    std::cout << "value is outside of allowed limits: " <<
              el.getNOfAbsMeanAmplChOutRange() << std::endl;
  }
}
std::vector<timeTableRow> loadRunNumbers(const commandOptions& opts)
{
  auto tree = new TTree("old_time_table", "");
  tree->ReadFile(opts.m_timeFilePath_src.c_str(), "", ',');
  std::vector<timeTableRow> result;
  int n = tree->GetEntries();
  result.reserve(n);
  for (int i = 0; i < tree->GetEntries(); ++i) {
    tree->GetEntry(i);
    timeTableRow row;
    row.m_exp =
      *static_cast<int*>(tree->
                         GetLeaf("exp")->
                         GetValuePointer());
    row.m_run =
      *static_cast<int*>(tree->
                         GetLeaf("run")->
                         GetValuePointer());
    row.m_run_start =
      static_cast<const char*>(tree->
                               GetLeaf("run_start")->
                               GetValuePointer());
    row.m_run_end =
      static_cast<const char*>(tree->
                               GetLeaf("run_end")->
                               GetValuePointer());
    row.m_evt_count =
      *static_cast<int*>(tree->
                         GetLeaf("evt_count")->
                         GetValuePointer());
    if (opts.m_exp == row.m_exp &&
        row.m_run >= opts.m_lowrun &&
        row.m_run <= opts.m_highrun) {
      result.push_back(row);
    }
  }
  delete tree;
  return result;
}
std::vector<timeTableRow> getGoodRunTable(
  const commandOptions& opts,
  const std::list<ECLLocalRunCalibQuality>& qualities)
{
  auto runTable = loadRunNumbers(opts);
  int run;
  for (const auto& el : qualities) {
    run = el.getRunNumber();
    auto iter = std::find_if(runTable.begin(),
                             runTable.end(),
    [&run](const timeTableRow & row) {
      return row.m_run == run;
    });
    if (iter != runTable.end()) {
      runTable.erase(iter);
    }
  }
  return runTable;
}
void updateTimeTable(
  const commandOptions& opts,
  const std::vector<timeTableRow>& runTable)
{
  std::ofstream fl;
  bool exist = false;
  struct stat buffer;
  if (stat(opts.m_timeFilePath_dest.c_str(), &buffer) == 0) {
    exist = true;
  }
  if (opts.m_reset_table || !exist) {
    fl.open(opts.m_timeFilePath_dest.c_str());
    fl << "exp/I:run/I:run_start/C:run_end/C:evt_count/I" << "\n";
  } else {
    fl.open(opts.m_timeFilePath_dest.c_str(), std::ios::app);
  }
  for (const auto& row : runTable) {
    fl << row.m_exp << ", " << row.m_run <<
       ", " << row.m_run_start << ", " <<
       row.m_run_end << ", " <<
       row.m_evt_count << "\n";
  }
}
void copyGoodRuns(
  const commandOptions& opts,
  const std::vector<timeTableRow>& runTable)
{
  ECLDBTool* payloadTime_src =
    new ECLDBTool(!opts.m_centraldb_src,
                  opts.m_dbname_src.c_str(),
                  "ECLCalibTime");
  ECLDBTool* payloadAmpl_src =
    new ECLDBTool(!opts.m_centraldb_src,
                  opts.m_dbname_src.c_str(),
                  "ECLCalibAmplitude");
  ECLDBTool* payloadTime_dest =
    new ECLDBTool(!opts.m_centraldb_dest,
                  opts.m_dbname_dest.c_str(),
                  "ECLCalibTime");
  ECLDBTool* payloadAmpl_dest =
    new ECLDBTool(!opts.m_centraldb_dest,
                  opts.m_dbname_dest.c_str(),
                  "ECLCalibAmplitude");
  TObject* obj;
  IntervalOfValidity* iov;
  for (const auto& row : runTable) {
    EventMetaData ev(1,
                     row.m_run,
                     row.m_exp);
    payloadTime_src->connect();
    payloadTime_src->read(&obj, &iov, ev);
    payloadTime_dest->connect();
    payloadTime_dest->write(obj, *iov);
    delete obj;
    delete iov;
    payloadAmpl_src->connect();
    payloadAmpl_src->read(&obj, &iov, ev);
    payloadAmpl_dest->connect();
    payloadAmpl_dest->write(obj, *iov);
    delete obj;
    delete iov;
  }
  delete payloadTime_src;
  delete payloadTime_dest;
  delete payloadAmpl_src;
  delete payloadAmpl_dest;
}
int main(int argc, char* argv[])
{
  commandOptions opts;
  po::options_description desc("Allowed options:");
  setOptions(&desc, &opts);
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);
  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 0;
  }
  if (isReady(vm)) {
    ECLLocalRunCalibQualityChecker timeQChecker(true);
    timeQChecker.setMinNOfEvs(opts.m_minNOfEvs);
    timeQChecker.setBadCountRatio(opts.m_lowCountRatioTime);
    timeQChecker.setBadOffsetValue(opts.m_timeMaxOffset);
    timeQChecker.setMeanValueLimits(opts.m_minMeanTime,
                                    opts.m_maxMeanTime);
    ECLLocalRunCalibQualityChecker amplQChecker(false);
    amplQChecker.setMinNOfEvs(opts.m_minNOfEvs);
    amplQChecker.setBadCountRatio(opts.m_lowCountRatioAmpl);
    amplQChecker.setBadOffsetValue(opts.m_amplMaxOffset);
    amplQChecker.setMeanValueLimits(opts.m_minMeanAmpl,
                                    opts.m_maxMeanAmpl);
    ECLLocalRunCalibLoader loader(!opts.m_centraldb_src,
                                  opts.m_dbname_src.c_str(),
                                  opts.m_timeFilePath_src.c_str());
    std::list<ECLLocalRunCalibQuality> qualities;
    loader.getQuality(&qualities, &timeQChecker, &amplQChecker,
                      opts.m_exp, opts.m_lowrun, opts.m_highrun);
    printQuality(qualities);
    auto runTable = getGoodRunTable(opts, qualities);
    copyGoodRuns(opts, runTable);
    updateTimeTable(opts, runTable);
  } else {
    std::cout << desc << std::endl;
  }
  return 0;
}
