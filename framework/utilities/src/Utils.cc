/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
// weird bug in intel compiler and or boost::process: If we include boost
// process intel compile fails. It seems that including sys/wait.h after
// other c++ std:: headers triggers a weird behavior failing two static
// asserts. We work around that by including it right away
#include <sys/wait.h>

#include <framework/utilities/Utils.h>

#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <boost/process.hpp>

#include <sys/time.h>
#include <unistd.h>

#include <cstdio>
#include <iomanip>
#include <utility>

using namespace Belle2;

namespace {
  /** get the Virtual and Resident memory size in KB
   *
   * This is done by reading /proc/PID/statm (kept open between calls)
   *
   * this is significantly faster than using TSystem::GetProcInfo()
   *
   * @return pair with virtual memory size in the first and resident memory
   *         size in the second entry.
   */
  std::pair<unsigned long, unsigned long> getStatmSize()
  {
    /** page size of system */
    const static long pageSizeKb = sysconf(_SC_PAGESIZE) / 1024;
    static FILE* stream = nullptr;
    static int pid = 0;
    int currentPid = getpid();
    if (currentPid != pid) {
      pid = currentPid;
      std::string statm = "/proc/" + std::to_string(pid) + "/statm";
      stream = fopen(statm.c_str(), "r");
      // If we use buffering we might get the same value each time we read so
      // disable buffering
      setvbuf(stream, nullptr, _IONBF, 0);
    }
    unsigned long vmSizePages{0};
    unsigned long rssPages{0};
    rewind(stream);
    fscanf(stream, "%lu %lu", &vmSizePages, &rssPages);
    return std::make_pair(vmSizePages * pageSizeKb, rssPages * pageSizeKb);
  }
}

namespace Belle2::Utils {

  double getClock()
  {
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (ts.tv_sec * Unit::s) + (ts.tv_nsec * Unit::ns);
  }
  double getCPUClock()
  {
    timespec ts;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    return (ts.tv_sec * Unit::s) + (ts.tv_nsec * Unit::ns);
  }

  unsigned long getVirtualMemoryKB()
  {
    return getStatmSize().first;
  }

  unsigned long getRssMemoryKB()
  {
    return getStatmSize().second;
  }

  Timer::Timer(std::string  text):
    m_startTime(getClock()),
    m_text(std::move(text))
  { }

  Timer::~Timer()
  {
    double elapsed = (getClock() - m_startTime) / Unit::ms;
    B2INFO(m_text << " " << std::fixed << std::setprecision(3) << elapsed << " ms");
  }

  std::string getCommandOutput(const std::string& command, const std::vector<std::string>& arguments,
                               bool searchPath)
  {
    namespace bp = boost::process;
    auto cmd = searchPath ? bp::search_path(command) : boost::filesystem::path(command);
    bp::ipstream cmdOut;
    bp::child child(cmd, bp::args(arguments), bp::std_in.close(), bp::std_out > cmdOut);
    char buffer[4096];
    std::string result;
    while (child.running() && cmdOut.read(buffer, sizeof(buffer))) {
      result.append(buffer, sizeof(buffer));
    }
    if (cmdOut.gcount()) result.append(buffer, cmdOut.gcount());
    return result;
  }
}
