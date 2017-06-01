#include <framework/utilities/Utils.h>

#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>
#include <cstdio>
#include <iomanip>

using namespace Belle2;

namespace {
  /** get stream of /proc/PID/statm (kept open between calls)
   *
   * this is significantly faster than using TSystem::GetProcInfo()
   */
  FILE* getStatm()
  {
    static FILE* stream = nullptr;
    static int pid = 0;
    int currentPid = getpid();
    if (currentPid != pid) {
      pid = currentPid;
      std::string statm = "/proc/" + std::to_string(pid) + "/statm";
      stream = fopen(statm.c_str(), "r");
    }
    rewind(stream);
    return stream;
  }
  //page size of system
  const static int pageSizeKb = sysconf(_SC_PAGESIZE) / 1024;
}

namespace Belle2 {
  namespace Utils {

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
      unsigned long int vmSizePages = 0;
      long rssPages = 0;
      fscanf(getStatm(), "%lu %ld", &vmSizePages, &rssPages);
      return vmSizePages * pageSizeKb;
    }

    unsigned long getRssMemoryKB()
    {
      unsigned long int vmSizePages = 0;
      long rssPages = 0;
      fscanf(getStatm(), "%lu %ld", &vmSizePages, &rssPages);
      return rssPages * pageSizeKb;
    }

    Timer::Timer(const std::string& text):
      m_startTime(getClock()),
      m_text(text)
    { }

    Timer::~Timer()
    {
      double elapsed = (getClock() - m_startTime) / Unit::ms;
      B2INFO(m_text << " " << std::fixed << std::setprecision(3) << elapsed << " ms");
    }
  }
}
