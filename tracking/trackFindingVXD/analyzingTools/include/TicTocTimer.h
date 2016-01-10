/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Madlener                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>

#include <chrono> // if inheritance from RelationObject is desired cannot use c++11 features
// #include <boost/chrono.hpp>
#include <ostream>
#include <sstream>
#include <string>

namespace Belle2 {

  /**
   * Timer class that somewhat mimics the tic-toc behavior of MATLAB.
   * Use tic() to start a time measurement and toc() to end it.
   * The base unit of the measurement is nanoseconds (ns) but the desired unit can be chosen by the user
   */
  class TicTocTimer { // : public RelationsObject {
  public:
    /**
     * constructor (also works as tic)
     * @param convFactor, factor by which the base unit is divided for the output (i.e. for us convFactor = 1000)
     * @param name, name of the timer
     */
    TicTocTimer(unsigned convFactor = 1, std::string name = "");

    void tic(); /**< reset the starting time */

    void toc(); /**< stop the time measurement */

    /**
     * print the name of the timer and the elapsed time
     * NOTE: calls time() -> clock is stopped if it has not already been stopped.
     */
    std::string print();

    /**
     * get the time measured by the timer.
     * NOTE: if the clock has only been started (call to tic()), but not stopped, it is stopped by a call to this function!)
     */
    double time();

    std::string getName() const { return m_name; } /**< get the name of the timer */

    std::string getUnit() const { return m_unit; } /**< get the unit of the timer */

  protected:

    /**
     * define the internally used clock.
     * Using steady_clock because it's easier to test and it should have the same resolution as the high_resolution_clock
    */
    using internal_clock = std::chrono::steady_clock;

    unsigned int m_convFactor; /**< conversion factor to be used to convert from nanoseconds to any other unit by division */

    std::string m_name; /**< name of the instance */

    bool m_tocked; /**< bool to indicate if both toc has been called after a call to tic */

    std::string m_unit; /**< the unit used for the time measurement */

    internal_clock::time_point m_start{}; /**< starting time point of the current time measurement */

    internal_clock::time_point m_end{}; /**< end time point of the current time measurement */

    friend class TicTocTimerTest; /**< test class is friend to have access to private members in tests
                                   */
    // ClassDef(TicTocTimer, 1)
  };

  inline TicTocTimer::TicTocTimer(unsigned convFactor, std::string name) :
    m_convFactor(convFactor),
    m_name(name),
    m_tocked(false),
    m_unit("ns")
  {
    switch (m_convFactor) {
      case 1: break;
      case 1000:
        m_unit = "us"; break;
      case 1000000:
        m_unit = "ms"; break;
      case 1000000000:
        m_unit = "s"; break;
      default:
        std::stringstream ss{};
        ss << " / " << m_convFactor << " ns";
        m_unit = ss.str();
    }
    tic();
  }

  inline void TicTocTimer::tic()
  {
    m_tocked = false;
    m_start = internal_clock::now();
  }

  inline void TicTocTimer::toc()
  {
    m_end = internal_clock::now();
    m_tocked = true;
  }

  inline double TicTocTimer::time()
  {
    if (!m_tocked) toc();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(m_end - m_start).count() / m_convFactor;
  }

  inline std::string TicTocTimer::print()
  {
    std::stringstream ss{};
    ss << m_name << " elapsed time: " << time() << " " << getUnit();
    return ss.str();
  }

  /** stream operator overload for TicTocTimer */
  inline std::ostream& operator<<(std::ostream& os, TicTocTimer& timer)
  {
    os << timer.print();
    return os;
  }

} // end namespace Belle2
