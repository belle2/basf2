/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TBranch.h>

#include <string>
#include <sstream>
#include <functional>

namespace Belle2 {
  /** General utility functions */
  namespace Utils {

    /** Helper struct for the C++17 std::visit overload pattern to allow simple use of variants */
    // cppcheck-suppress syntaxError ; apparently this confuses cppcheck
    template<class... Ts> struct VisitOverload : Ts... { using Ts::operator()...; };
    /** Function for the C++17 std::visit overload pattern to allow simple use of variants */
    template<class... Ts> VisitOverload(Ts...) -> VisitOverload<Ts...>;

    /**
     * Reduce a branch of a TTree
     * @return reduced branch
     */
    template<class T>
    T reduceTBranch(TBranch* branch, const std::function<T(T, T)>& f, T reduced = T())
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
     * Return current value of the real-time clock.
     *
     * The returned value is meant to measure relative times and does not show
     * absolute time values.
     *
     * @note See getCPUClock() for a higher-resolution clock unaffected by wait times.
     * @return Clock value in default time unit (ns)
     */
    double getClock();

    /**
     * Return current value of the per-thread CPU clock.
     *
     * @return CPU clock value in default time unit (ns)
     */
    double getCPUClock();

    /**
     * Returns currently used virtual memory in KB, includes swapped and not occupied memory
     * pages and memory-mapped files.
     */
    unsigned long getVirtualMemoryKB();

    /**
     * Returns the amount of memory the process actually occupies in the physical RAM of
     * the machine.
     */
    unsigned long getRssMemoryKB();

    /** Small helper class that prints its lifetime when destroyed. */
    class Timer {
    public:
      /** Constructor, with some identifying text */
      explicit Timer(std::string  text = "");
      ~Timer();
    private:
      double m_startTime; /**< time at start (in ns). */
      std::string m_text; /**< identifying text (printed at end). */
    };

    /**
     * Execute a shell command and return its output
     */
    std::string getCommandOutput(const std::string& command, const std::vector<std::string>& arguments = {}, bool searchPath = true);
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

  /** \def branch_unlikely(x)
   * \brief A macro to tell the compiler that the argument x will be very
   * likely be false.
   *
   * This could result in optimized code as the compiler can reorder the code
   * to reduce conditional jumps during execution. Due to the high efficiency
   * of todays branch prediction systems the effect is probably small.
   * \warning Use of this macro should be accompanied by a comment
   *          documenting the (significant) performance gains that justify
   *          its use. (Please make sure you compile in 'opt' mode.)
   * \see \def branch_likely(x)
   *
   * Usage:
   *
   *     if (branch_unlikely(my_number > 10)) {
   *       // executed only rarely
   *     } else {
   *       // executed very often
   *     }
   */
#if defined(__GNUC__) || defined(__ICL) || defined(__clang__)
#define branch_likely(x)       __builtin_expect(!!(x), 1)
#define branch_unlikely(x)     __builtin_expect(!!(x), 0)
#else
#define branch_likely(x)       (x)
#define branch_unlikely(x)     (x)
#endif

} // Belle2 namespace
