#include <framework/utilities/Utils.h>

#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <TSystem.h>

#include <sys/time.h>
#include <cstdlib>
#include <iomanip>

using namespace Belle2;

namespace Belle2 {
  namespace Utils {

    double getClock()
    {
      timespec ts;
      clock_gettime(CLOCK_REALTIME, &ts);
      return (ts.tv_sec * Unit::s) + (ts.tv_nsec * Unit::ns);
    }

    unsigned long getMemoryKB()
    {
      ProcInfo_t meminfo;
      gSystem->GetProcInfo(&meminfo);

      return meminfo.fMemVirtual;
    }

    Timer::Timer(std::string text):
      m_startTime(getClock()),
      m_text(text)
    { }

    Timer::~Timer()
    {
      double elapsed = (getClock() - m_startTime) / Unit::ms;
      B2INFO(m_text << std::fixed << std::setprecision(3) << elapsed << " ms");
    }
  }
}
