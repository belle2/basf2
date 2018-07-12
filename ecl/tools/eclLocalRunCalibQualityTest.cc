/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 *                                                                        *
 * eclLocalRunCalibQualityTest                                            *
 *                                                                        *
 * This tool has been designed in order to check quality of local runs,   *
 * which have been stored into database                                   *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su) (BINP)         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// STL
#include <iostream>
#include <list>
// BOOST
#include <boost/program_options.hpp>
// ECL
#include <ecl/utility/ECLLocalRunCalibLoader.h>
#include <ecl/utility/ECLLocalRunCalibQuality.h>
#include <ecl/utility/ECLLocalRunCalibQualityChecker.h>
using namespace Belle2;
namespace po = boost::program_options;
struct commandOptions {
  // Ebable central database.
  bool m_centraldb = false;
  // DB tag in the central
  // database or path to a
  // local database.
  std::string m_dbname = "localdb/database.txt";
  // Path to file with run / time table.
  std::string m_timeFilePath = "timetab.cvs";
  // Experiment number.
  int m_exp;
  // Low run.
  int m_lowrun;
  // High run.
  int m_highrun;
  // Minimum allowed number of events.
  int m_minNOfEvs = 100;
  // time low count ratio
  float m_lowCountRatioTime = 0.98;
  // amplitude low count ratio
  float m_lowCountRatioAmpl = 0.98;
  // time max offset
  float m_timeMaxOffset = 2.;
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
void setOptions(po::options_description* desc,
                commandOptions* opts)
{
  desc->add_options()
  ("help", "Produce a help message.")
  ("centraldb", po::bool_switch(&(opts->m_centraldb)),
   "Use a central database.")
  ("dbname", po::value<std::string>(&(opts->m_dbname)),
   "A central database tag or path to a local database.")
  ("timetab", po::value<std::string>(&(opts->m_timeFilePath)),
   "Path to the file with time / run table.")
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
  for (const auto& el : qualities) {
    std::cout << "=====================" << std::endl;
    std::cout << "experiment: " << el.getExpNumber() << std::endl;
    std::cout << "run: " << el.getRunNumber() << std::endl;
    std::cout << "reference run: " << el.getReferenceRunNumber() << std::endl;
    std::cout << "number of events: " <<
              el.getNOfEvents() << std::endl;
    if (el.isTooSmallNOfEvs()) {
      std::cout << "[!} Number of event is too small!" << std::endl;
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
    ECLLocalRunCalibLoader loader(!opts.m_centraldb,
                                  opts.m_dbname.c_str(),
                                  opts.m_timeFilePath.c_str());
    std::list<ECLLocalRunCalibQuality> qualities;
    loader.getQuality(&qualities, &timeQChecker, &amplQChecker,
                      opts.m_exp, opts.m_lowrun, opts.m_highrun);
    printQuality(qualities);
  } else {
    std::cout << desc << std::endl;
  }
  return 0;
}
