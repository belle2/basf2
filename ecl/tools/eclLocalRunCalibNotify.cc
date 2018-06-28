/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 *                                                                        *
 * eclLocalRunCalibNotify                                                 *
 *                                                                        *
 * eclLocalRunCalibNotify has been developed in order                     *
 * to notify user about small number of accepted events,                  *
 * large ofsets of mean values or about presence of negative              *
 * amplitudes after local run calibration procedure.                      *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su) (BINP)         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// STL
#include <list>
#include <vector>
#include <utility>
#include <string>
#include <iostream>
#include <regex>
// BOOST
#include <boost/program_options.hpp>
#include <boost/format.hpp>
// ROOT
#include <TFile.h>
#include <TObject.h>
#include <TTree.h>
#include <TLeaf.h>
#include <TH1.h>
#include <TH1F.h>
#include <TGraph.h>
// ECL
#include <ecl/utility/ECLDBTool.h>
#include <ecl/dbobjects/ECLLocalRunCalibRef.h>
#include <ecl/dbobjects/ECLCrystalLocalRunCalib.h>
using namespace Belle2;
namespace po = boost::program_options;
// Program options.
struct commandOptions {
  // Notify about numbers of accepted events.
  bool m_count = false;
  // Notify about offset values.
  bool m_offset = false;
  // Notify about presence of
  // negative amplitudes.
  bool m_checkNegAmpl = false;
  // Use central database.
  bool m_centraldb = false;
  // Experiment number.
  int m_exp;
  // Run number.
  int m_run;
  // Minimum ratio between acceptd number
  // of events and total number of events
  // for time.
  float m_minTimeCountRatio = 0.98;
  // Minimum ratio between acceptd number
  // of events and total number of events
  // for amplitude.
  float m_minAmplCountRatio = 0.98;
  // Maximum time offset.
  float m_maxOffsetTime = 2.0;
  // Maximum amplitude offset.
  float m_maxOffsetAmpl = 0.002;
  // A tag of the central database
  // or path to a local database.
  std::string m_dbname =
    "localdb/database.txt";
  // Time payload name.
  static const std::string c_timePayloadName;
  // Amplitude payload name.
  static const std::string c_amplPayloadName;
  // Reference mark payload name.
  static const std::string c_refPayloadName;
  // Notify about numbers of accepted
  // events.
  bool isCountCheck() const;
  // Notify about offset values.
  bool isOffsetCheck() const;
  // Notify about presence of negative
  // amplitudes.
  bool isNegAmplCheck() const;
};
// Time payload name.
const std::string
commandOptions::c_timePayloadName = "ECLCalibTime";
// Amplitude payload name.
const std::string
commandOptions::c_amplPayloadName = "ECLCalibAmplitude";
// Reference mark payload name.
const std::string
commandOptions::c_refPayloadName = "ECLCalibRef";
// If returned value is true, then
// the program will notify user about
// numbers of accepted events.
bool commandOptions::isCountCheck() const
{
  return m_count && !m_offset && !m_checkNegAmpl;
}
// If returned value is true, then the
// program will notify user about
// offset values.
bool commandOptions::isOffsetCheck() const
{
  return m_offset && !m_count && !m_checkNegAmpl;
}
// If returned value is true, then the
// program will notify user about
// presence of negative amplitudes.
bool commandOptions::isNegAmplCheck() const
{
  return m_checkNegAmpl && !m_count && !m_offset;
}
// Set program options.
void setOptions(po::options_description* desc,
                commandOptions* opts)
{
  desc->add_options()
  ("help", "Produce a help message.")
  ("centraldb", po::bool_switch(&(opts->m_centraldb)),
   "Use a central database")
  ("negampl", po::bool_switch(&(opts->m_checkNegAmpl)),
   "Check presence of negative amplitude values")
  ("exp", po::value<int>(&(opts->m_exp)),
   "Experiment number")
  ("run", po::value<int>(&(opts->m_run)),
   "Run number")
  ("count", po::bool_switch(&(opts->m_count)),
   "Check of accepted events.")
  ("offset", po::bool_switch(&(opts->m_offset)),
   "Offset check.")
  ("timecount",
   po::value<float>(&(opts->m_minTimeCountRatio)),
   "If ratio of events, used to calculate "
   "mean time, to total number of events "
   "for a certain cellid is "
   "less than timecount, than "
   "the number of this cellid will be "
   "prited.")
  ("amplcount",
   po::value<float>(&(opts->m_minAmplCountRatio)),
   "If ratio of events, used to calculate "
   "mean amplitude, to total number of events "
   "for a certain cellid is "
   "less than amplcount, "
   "the number of this cellid will be "
   "prited.")
  ("timeoffset",
   po::value<float>(&(opts->m_maxOffsetTime)),
   "The normalized value of mean time defined as:\n"
   "a = normalized mean time = (mean time) - (reference mean time).\n"
   "If the absolute value of the offset, a - 0, for a "
   "certain cellid is greater than "
   "timeoffset, the number of this cellid will "
   "be printed.")
  ("amploffset",
   po::value<float>(&(opts->m_maxOffsetAmpl)),
   "The normalized value of mean amplitude defined as:\n"
   "a = normalized mean amplitude = (mean amplitude) / (reference mean amplitude).\n"
   "If the absolute value of the offset, a - 1, for a "
   "certain cellid is greater than "
   "amploffset, the number of this cellid will "
   "be printed.")
  ("dbname", po::value<std::string>(&(opts->m_dbname)),
   "Central database tag or path to a local database");
}
// Help message.
void help(const po::options_description& desc)
{
  std::cout << desc << std::endl;
}
// Check numbers of accepted events.
void countCheck(const commandOptions& opts,
                bool isTime)
{
  ECLDBTool* payload;
  TObject* obj;
  IntervalOfValidity* iov;
  EventMetaData ev(1, opts.m_run, opts.m_exp);
  if (isTime) {
    payload = new ECLDBTool(!opts.m_centraldb,
                            opts.m_dbname.c_str(),
                            opts.c_timePayloadName.c_str());
    std::cout <<
              "Cellids, where the percent of accepted events used "
              "to calculate TIME mean value is less than " <<
              opts.m_minTimeCountRatio << std::endl;
  } else {
    payload = new ECLDBTool(!opts.m_centraldb,
                            opts.m_dbname.c_str(),
                            opts.c_amplPayloadName.c_str());
    std::cout <<
              "Cellids, where the percent of "
              "accepted events used to calculate "
              "AMPLITUDE mean value is less than " <<
              opts.m_minAmplCountRatio << "." << std::endl;
  }
  payload->connect();
  payload->read(&obj, &iov, ev);
  auto calib =
    static_cast<ECLCrystalLocalRunCalib*>(obj);
  int Nev = calib->getNumberOfEvents();
  auto ncounts = calib->getNumbersOfAcceptedEvents();
  int cellid = 1;
  for (const auto& count : ncounts) {
    float count_ratio = static_cast<float>(count) / Nev;
    if ((count_ratio < opts.m_minTimeCountRatio &&
         isTime) ||
        (count_ratio < opts.m_minAmplCountRatio &&
         !isTime)) {
      std::cout << "cellid " << cellid << ": " <<
                count << " events of " << Nev <<
                " are accepted" << std::endl;
    }
    cellid++;
  }
  delete obj;
  delete iov;
  delete payload;
}
// Check offsets
void offsetCheck(const commandOptions& opts,
                 bool isTime)
{
  ECLDBTool* payload;
  TObject* obj;
  IntervalOfValidity* iov;
  EventMetaData ev(1, opts.m_run, opts.m_exp);
  if (isTime) {
    payload = new ECLDBTool(!opts.m_centraldb,
                            opts.m_dbname.c_str(),
                            opts.c_timePayloadName.c_str());
    std::cout <<
              "Cellids, where the TIME offset "
              "is greater than " <<
              opts.m_maxOffsetTime << "." << std::endl;
  } else {
    payload = new ECLDBTool(!opts.m_centraldb,
                            opts.m_dbname.c_str(),
                            opts.c_amplPayloadName.c_str());
    std::cout <<
              "Cellids, where the AMPLITUDE "
              "offset is greater than " <<
              opts.m_maxOffsetAmpl << "." << std::endl;
  }
  payload->connect();
  payload->read(&obj, &iov, ev);
  auto calib =
    static_cast<ECLCrystalLocalRunCalib*>(obj);
  int exp = calib->getExp();
  int run = calib->getRun();
  std::vector<float> calib_means = calib->getCalibVector();
  EventMetaData refpointer_ev(1, run, exp);
  ECLDBTool ref_payload(!opts.m_centraldb,
                        opts.m_dbname.c_str(),
                        opts.c_refPayloadName.c_str());
  delete obj;
  delete iov;
  ref_payload.connect();
  ref_payload.read(&obj, &iov, refpointer_ev);
  auto refpointer =
    static_cast<ECLLocalRunCalibRef*>(obj);
  exp = refpointer->getExp();
  run = refpointer->getRun();
  EventMetaData ref_ev(1, run, exp);
  delete obj;
  delete iov;
  payload->connect();
  payload->read(&obj, &iov, ref_ev);
  auto ref =
    static_cast<ECLCrystalLocalRunCalib*>(obj);
  std::vector<float> ref_means = ref->getCalibVector();
  delete obj;
  delete iov;
  delete payload;
  int cellid = 1;
  for (const auto& val : calib_means) {
    if (isTime) {
      float offset = val - ref_means[cellid - 1];
      if (fabs(offset) > opts.m_maxOffsetTime) {
        std::cout << "cellid " << cellid <<
                  ": " << "offset = " <<
                  offset << std::endl;
      }
    } else {
      float offset = val / ref_means[cellid - 1] - 1;
      if (fabs(offset) > opts.m_maxOffsetAmpl) {
        std::cout << "cellid " << cellid <<
                  ": " << "offset = " <<
                  offset << std::endl;
      }
    }
    cellid++;
  }
}
// Check presence of negative amplitudes
void checkNegAmpl(const commandOptions& opts)
{
  TObject* obj;
  IntervalOfValidity* iov;
  EventMetaData ev(1, opts.m_run, opts.m_exp);
  auto payload = new ECLDBTool(!opts.m_centraldb,
                               opts.m_dbname.c_str(),
                               opts.c_amplPayloadName.c_str());
  std::cout << "Checking presence of negative amplitudes..."
            << std::endl;
  payload->connect();
  payload->read(&obj, &iov, ev);
  auto calib =
    static_cast<ECLCrystalLocalRunCalib*>(obj);
  if (calib->isNegAmpl()) {
    std::cout << "There are negative amplitudes!"
              << std::endl;
  }
  delete calib;
  delete iov;
  delete payload;
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
    help(desc);
    return 0;
  }
  if (!vm.count("exp") ||
      !vm.count("run")) {
    std::cout << "[!] You need to use "
              "both of exp and run options."
              << std::endl;
    return 0;
  }
  if (opts.isCountCheck()) {
    countCheck(opts, true);
    countCheck(opts, false);
    return 0;
  }
  if (opts.isOffsetCheck()) {
    offsetCheck(opts, true);
    offsetCheck(opts, false);
    return 0;
  }
  if (opts.isNegAmplCheck()) {
    checkNegAmpl(opts);
    return 0;
  }
  return 0;
}

