/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

// ECL
#include <ecl/dbobjects/ECLCrystalCalib.h>

namespace Belle2 {
  /**
   * ECLCrystalLocalRunCalib is designed
   * to store results of the ECL local run
   * calibration to database.
   */
  class ECLCrystalLocalRunCalib: public Belle2::ECLCrystalCalib {
  public:
    /**
     * Constructor.
     * @param isNegAmpl is true, when there are
     negative amplitude values.
     */
    explicit ECLCrystalLocalRunCalib(bool isNegAmpl = false);
    /**
     * Destructor.
     */
    ~ECLCrystalLocalRunCalib();
    /**
     * Set total number of events.
     * @param numberOfEvents is the number of
     events.
     */
    void setNumberOfEvents(const int& numberOfEvents);
    /**
     * Set number of accepted events for each cell id.
     * @param numberOfAcceptedEvents is the numbers of
     accepted events.
     */
    void setNumbersOfAcceptedEvents(const std::vector<int>&
                                    numberOfAcceptedEvents);
    /**
     * Get number of events.
     */
    int getNumberOfEvents() const;
    /**
     * Set experiment number and run number.
     * @param exp is the experiment number.
     * @param run is the run number.
     */
    void setExpRun(const int& exp, const int& run);
    /**
     * Get experiment number.
     */
    int getExp() const;
    /**
     * Get run number.
     */
    int getRun() const;
    /**
     * Get numbers of accepted events for
     * each cell id.
     */
    const std::vector<int>& getNumbersOfAcceptedEvents() const;
    /**
     * In case if the amplitude
     * payload: check is there are
     * negative amplitudes.
     */
    bool isNegAmpl() const;
  private:
    /**
     * In the case of time payload
     * c_isNegAmpl is always false.
     * In the case of amplitude payload
     * c_isNegAmpl is true, if there are
     * negative amplitude values, and is
     * false otherwise.
     */
    bool c_isNegAmpl;
    /**
     * Experiment number.
     */
    int m_exp;
    /**
     * Run number.
     */
    int m_run;
    /**
     * Total number of events.
     */
    int m_numberOfEvents;
    /**
     * Numbers of accepted events
     * for each cell id.
     */
    std::vector<int> m_numberOfAcceptedEvs;
    ClassDef(ECLCrystalLocalRunCalib, 1); /**< ClassDef */
  };
}
