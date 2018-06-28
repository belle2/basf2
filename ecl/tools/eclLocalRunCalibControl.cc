/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 *                                                                        *
 * eclLocalRunControl                                                     *
 *                                                                        *
 * eclLocalRunCalibControl is a tool, which provide to user possibility   *
 * to add reference marks, see the information about local run            *
 * calibration payloads and change validity intervals (make copies        *
 * of existing payloads with new validity intervals)                      *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su) (BINP)         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// STL
#include <iostream>
#include <string>
#include <limits>
// BOOST
#include <boost/program_options.hpp>
// ECL
#include <ecl/utility/ECLDBTool.h>
#include <ecl/dbobjects/ECLLocalRunCalibRef.h>
#include <ecl/dbobjects/ECLCrystalLocalRunCalib.h>
using namespace Belle2;
namespace po = boost::program_options;
// Program options.
struct cmdOptions {
  // Choose time payloads.
  bool m_calibtime = false;
  // Choose amplitude payloads.
  bool m_calibampl = false;
  // Low run of validity interval.
  int m_lowrun = -1;
  // High run of validity interval.
  int m_highrun = -1;
  // Used to change experiment number.
  int m_newexp;
  // Run number.
  int m_run;
  // Experiment number.
  int m_exp = -1;
  // Add reference mark.
  bool m_addref = false;
  // Show last payload for the
  // current experiment.
  bool m_showlast = false;
  // Show payload corresponding
  // to certain experiment and
  // run numbers.
  bool m_show = false;
  // Use central database.
  bool m_centraldb = false;
  // Change the validity interval
  // of the previous payload.
  bool m_changeprev = false;
  // Set interval of validity.
  bool m_setiov = false;
  // A tag in the central database
  // or path to a local database.
  std::string m_dbname =
    "localdb/database.txt";
  // If true, the information about
  // the last payload
  // in current experiment will
  // be shown.
  bool isShowLast() const;
  // If true, the information about
  // the payload
  // corresponding to certain
  // experiment and run numbers
  // will be shown.
  bool isShow() const;
  // If ture, the new reference
  // mark will be set.
  bool isAddRef() const;
  // If true, the validity interval
  // will be changed.
  bool isSetIoV() const;
  // Check reference mode.
  bool isRef() const;
};
// Check reference mode.
bool cmdOptions::isRef() const
{
  return
    !m_calibtime &&
    !m_calibampl;
}
// If true, show the last payload
bool cmdOptions::isShowLast() const
{
  return
    m_showlast && !m_addref &&
    !m_show && !m_changeprev &&
    !m_setiov &&
    (m_exp > 0);
}
// If true, show the payload corresponding
// to certain experiment and run
// numbers.
bool cmdOptions::isShow() const
{
  return
    m_show && !m_showlast &&
    !m_addref && !m_changeprev &&
    !m_setiov &&
    (m_exp > 0) &&
    (m_run > 0);
}
// If true, add reference mark.
bool cmdOptions::isAddRef() const
{
  return
    m_addref && !m_showlast &&
    !m_show && !m_setiov &&
    !m_calibtime &&
    !m_calibampl &&
    (m_lowrun > 0) &&
    (m_exp > 0) &&
    (m_run > 0);
}
// If true, change validity interval.
bool cmdOptions::isSetIoV() const
{
  return
    m_setiov && !m_addref &&
    !m_showlast && !m_show &&
    !m_changeprev &&
    (m_run > 0) &&
    (m_exp > 0) &&
    (m_lowrun > 0) &&
    (m_newexp > 0);
}
// Print payload information.
template<class T>
void printPayloadInfo(const T* const refobj,
                      const IntervalOfValidity* const iov)
{
  std::cout << "====================" << std::endl;
  if (dynamic_cast<const ECLLocalRunCalibRef* const>(refobj)) {
    std::cout << "Reference run: " << std::endl;
  } else {
    std::cout << "Calibration run: " << std::endl;
  }
  std::cout << "exp = " << refobj->getExp() << std::endl;
  std::cout << "run = " << refobj->getRun() << std::endl;
  std::cout << "--------------------" << std::endl;
  std::cout << "Interval of Validity:" << std::endl;
  std::cout << "exp low = " <<
            iov->getExperimentLow() << std::endl;
  std::cout << "exp high = " <<
            iov->getExperimentHigh() << std::endl;
  std::cout << "run low = " <<
            iov->getRunLow() << std::endl;
  std::cout << "run high = " <<
            iov->getRunHigh() << std::endl;
  std::cout << "====================" << std::endl;
}
// Set program options.
void setOptions(po::options_description* desc,
                cmdOptions* opts)
{
  desc->add_options()
  ("help", "Produce help message.")
  ("dbname", po::value<std::string>(&(opts->m_dbname)), "Database name.")
  ("centraldb", po::bool_switch(&(opts->m_centraldb)), "Use central database.")
  ("calibtime", po::bool_switch(&(opts->m_calibtime)), "Select time calibration payloads")
  ("calibampl", po::bool_switch(&(opts->m_calibampl)), "Select amplitude calibration payloads")
  ("changeprev", po::bool_switch(&(opts->m_changeprev)), "Change interval "
   "of validity of previous payload")
  ("setiov", po::bool_switch(&(opts->m_setiov)), "Change selected interval of validity")
  ("lowrun", po::value<int>(&(opts->m_lowrun)), "Low run number")
  ("highrun", po::value<int>(&(opts->m_highrun)), "High run number")
  ("newexp", po::value<int>(&(opts->m_newexp)), "New experiment number")
  ("exp", po::value<int>(&(opts->m_exp)), "Experiment number.")
  ("run", po::value<int>(&(opts->m_run)), "Run number.")
  ("addref", po::bool_switch(&(opts->m_addref)), "Add reference "
   "(exp, run, lowexp, lowrun are neaded).")
  ("showlast", po::bool_switch(&(opts->m_showlast)), "Show last reference "
   "run (exp is required).")
  ("show", po::bool_switch(&(opts->m_show)), "Show refference run, "
   "which contains (exp, run)");
}
// Help message.
void help(const po::options_description& desc)
{
  std::cout << desc << std::endl;
}
// Show information about the
// last payload.
template<class T>
void showLast(const ECLDBTool& payload,
              const cmdOptions& opts)
{
  EventMetaData ev(1,
                   std::numeric_limits<int>::max(),
                   opts.m_exp);
  TObject* obj;
  IntervalOfValidity* iov;
  payload.connect();
  payload.read(&obj, &iov, ev);
  T* refobj = static_cast<T*>(obj);
  printPayloadInfo<T>(refobj, iov);
  delete obj;
  delete iov;
}
// Show information about a certain payload.
template<class T>
void show(const ECLDBTool& payload,
          const cmdOptions& opts)
{
  EventMetaData ev(1, opts.m_run, opts.m_exp);
  TObject* obj;
  IntervalOfValidity* iov;
  payload.connect();
  payload.read(&obj, &iov, ev);
  T* refobj = static_cast<T*>(obj);
  printPayloadInfo(refobj, iov);
  delete obj;
  delete iov;
}
// Add reference mark.
void addRef(const ECLDBTool& payload,
            const cmdOptions& opts)
{
  IntervalOfValidity iov(opts.m_exp, opts.m_lowrun,
                         opts.m_exp, opts.m_highrun);
  ECLLocalRunCalibRef refobj(opts.m_exp, opts.m_run);
  payload.connect();
  if (opts.m_changeprev) {
    int run_high = opts.m_lowrun - 1;
    EventMetaData ev(1, run_high, opts.m_exp);
    IntervalOfValidity* prevIoV;
    payload.read(&prevIoV, ev);
    int run_low = prevIoV->getRunLow();
    IntervalOfValidity ciov(opts.m_exp, run_low,
                            opts.m_exp, run_high);
    payload.changeIoV(ev, ciov);
    delete prevIoV;
  }
  payload.write(&refobj, iov);
}
// Set low run.
void setLowRun(const ECLDBTool& payload,
               cmdOptions* opts)
{
  payload.connect();
  EventMetaData ev(1, opts->m_run, opts->m_exp);
  IntervalOfValidity* iov;
  payload.read(&iov, ev);
  opts->m_lowrun = iov->getRunLow();
  delete iov;
}
// Set validity interval.
void setIoV(const ECLDBTool& payload,
            const cmdOptions& opts)
{
  payload.connect();
  EventMetaData ev(1, opts.m_run, opts.m_exp);
  IntervalOfValidity* iov;
  payload.read(&iov, ev);
  IntervalOfValidity ciov(opts.m_newexp, opts.m_lowrun,
                          opts.m_newexp, opts.m_highrun);
  payload.changeIoV(ev, ciov);
  delete iov;
}
int main(int argc, char* argv[])
{
  po::options_description desc("Allowed options:");
  cmdOptions opts;
  setOptions(&desc, &opts);
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);
  if (vm.count("help")) {
    help(desc);
    return 0;
  }
  if (vm.count("run") &&
      !vm.count("lowrun")) {
    opts.m_lowrun = opts.m_run;
  }
  if (vm.count("exp") &&
      !vm.count("newexp")) {
    opts.m_newexp = opts.m_exp;
  }
  if (vm.count("lowrun") &&
      vm.count("run") &&
      (opts.m_lowrun > opts.m_run)) {
    std::cout << "[!] lowrun must be "
              "less than run" << std::endl;
    return 0;
  }
  if (opts.m_calibtime &&
      opts.m_calibampl) {
    std::cout << "[!] calibtime and "
              "calibampl options can not be used "
              "at the same time." << std::endl;
    return 0;
  }
  ECLDBTool* payload;
  if (opts.m_calibtime) {
    payload = new ECLDBTool(!opts.m_centraldb,
                            opts.m_dbname.c_str(),
                            "ECLCalibTime");
  }
  if (opts.m_calibampl) {
    payload = new ECLDBTool(!opts.m_centraldb,
                            opts.m_dbname.c_str(),
                            "ECLCalibAmplitude");
  }
  if (opts.isRef()) {
    payload = new ECLDBTool(!opts.m_centraldb,
                            opts.m_dbname.c_str(),
                            "ECLCalibRef");
  }
  if (opts.isShowLast()) {
    if (opts.isRef()) {
      showLast<ECLLocalRunCalibRef>(*payload, opts);
    } else {
      showLast<ECLCrystalLocalRunCalib>(*payload, opts);
    }
    return 0;
  }
  if (opts.isShow()) {
    if (opts.isRef()) {
      show<ECLLocalRunCalibRef>(*payload, opts);
    } else {
      show<ECLCrystalLocalRunCalib>(*payload, opts);
    }
    return 0;
  }
  if (opts.isAddRef()) {
    addRef(*payload, opts);
    return 0;
  }
  if (opts.isSetIoV()) {
    setIoV(*payload, opts);
    return 0;
  }
  help(desc);
  return 0;
}
