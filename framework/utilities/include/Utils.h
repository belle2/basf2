#pragma once

#include <TBranch.h>

#include <string>
#include <sstream>
#include <functional>

namespace Belle2 {
  /** General utility functions */
  namespace Utils {

    /**
     * Reduce a branch of a TTree
     * @return reduced branch
     */
    template<class T>
    T reduceTBranch(TBranch* branch, std::function<T(T, T)> f, T reduced = T())
    {
      T object;
      branch->SetAddress(&object);
      int nevents = branch->GetEntries();
      for (int i = 0; i < nevents; ++i) {
        branch->GetEvent(i);
        reduced = f(reduced, object);
      }
      return reduced;
    }


    /**
     * Return current value of the high performance clock.
     *
     * The returned value is meant to measure relative times and does not show
     * absolute time values;
     *
     * @return Clock value in default time unit (ns)
     */
    double getClock();

    /**
     * Returns currently used virtual memory in KB.
     *
     */
    unsigned long getMemoryKB();

    /** Small helper class that prints its lifetime when destroyed. */
    class Timer {
    public:
      /** Constructor, with some identifying text */
      explicit Timer(std::string text = "");
      ~Timer();
    private:
      double m_startTime; /**< time at start (in ns). */
      std::string m_text; /**< identifying text (printed at end). */
    };
  }

  /**
   * Print an INFO message txt followed by the time it took to execute the
   * statment given as second argument.
   *
   * Care should be taken not to define variables in this call since they will
   * be scoped and thus unavaiable after the macro
   */
#define B2INFO_MEASURE_TIME(txt, ...) {\
    std::stringstream __b2_timer_str__;\
    __b2_timer_str__ << txt;\
    ::Belle2::Utils::Timer __b2_timer__(__b2_timer_str__.str());\
    {__VA_ARGS__;}\
  }

} // Belle2 namespace
