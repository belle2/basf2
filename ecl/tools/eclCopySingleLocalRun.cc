/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 *                                                                        *
 * eclCopySingleLocalRun                                                  *
 *                                                                        *
 * This tool has been designed in order to copycalibration information    *
 * fro one single local run from one database to another one              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su) (BINP)         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// STL
#include <iostream>
#include <string>
// BOOST
#include <boost/program_options.hpp>
// ECL
#include <ecl/utility/ECLDBTool.h>
using namespace Belle2;
namespace po = boost::program_options;

struct copyOptions {
  bool m_srcDB_isLocal = false;
  bool m_destDB_isLocal = false;
  int m_run;
  int m_exp;
  std::string m_srcDB_name = "globaldb1";
  std::string m_destDB_name = "globaldb2";
};

void copyLR(const copyOptions& opts)
{
  ECLDBTool* src_timePayload =
    new ECLDBTool(opts.m_srcDB_isLocal,
                  opts.m_srcDB_name.c_str(),
                  "ECLCalibTime");
  ECLDBTool* src_amplPayload =
    new ECLDBTool(opts.m_srcDB_isLocal,
                  opts.m_srcDB_name.c_str(),
                  "ECLCalibAmplitude");
  ECLDBTool* dest_timePayload =
    new ECLDBTool(opts.m_destDB_isLocal,
                  opts.m_destDB_name.c_str(),
                  "ECLCalibTime");
  ECLDBTool* dest_amplPayload =
    new ECLDBTool(opts.m_destDB_isLocal,
                  opts.m_destDB_name.c_str(),
                  "ECLCalibAmplitude");
  TObject* obj;
  IntervalOfValidity* iov;
  EventMetaData ev(1, opts.m_run, opts.m_exp);
  src_timePayload->connect();
  src_timePayload->read(&obj, &iov, ev);
  dest_timePayload->connect();
  dest_timePayload->write(obj, *iov);
  delete obj;
  delete iov;
  src_amplPayload->connect();
  src_amplPayload->read(&obj, &iov, ev);
  dest_amplPayload->connect();
  dest_amplPayload->write(obj, *iov);
  delete obj;
  delete iov;
  delete src_timePayload;
  delete src_amplPayload;
  delete dest_timePayload;
  delete dest_amplPayload;
}

void setCopyOptions(po::options_description* desc,
                    copyOptions* opts)
{
  desc->add_options()
  ("help", "Produce a help message.")
  ("src-localdb",
   po::bool_switch(&(opts->m_srcDB_isLocal)),
   "Use local source DB.")
  ("dest-localdb",
   po::bool_switch(&(opts->m_destDB_isLocal)),
   "Use local destination DB.")
  ("src-dbname",
   po::value<std::string>(&(opts->m_srcDB_name)),
   "Name of source database.")
  ("dest-dbname",
   po::value<std::string>(&(opts->m_destDB_name)),
   "Name of destination database.")
  ("run",
   po::value<int>(&opts->m_run),
   "Run number.")
  ("exp",
   po::value<int>(&opts->m_exp),
   "Experiment number.");
}

int main(int argc, char* argv[])
{
  copyOptions opts;
  po::options_description desc("Allowed options:");
  setCopyOptions(&desc, &opts);
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);
  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 0;
  }
  if (vm.count("exp") && vm.count("run")) {
    copyLR(opts);
    return 0;
  }
  std::cout << desc << std::endl;
  return 0;
}
