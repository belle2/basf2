/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * ECLLocalRunCalibUnit                                                   *
 *                                                                        *
 * This class controls feature (mean value, standard deviation and        *
 * number of accepted events) accumulators for each cell id.              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su) (BINP)         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef _ECLLocalCalibUnit_H_
#define _ECLLocalCalibUnit_H_
#include <string>
#include <vector>
#include <ecl/modules/eclLocalRunCalibration/ECLLocalRunCalibAcc.h>
#include <framework/database/IntervalOfValidity.h>
#include <ecl/utility/ECLDBTool.h>

namespace Belle2 {
  /**
   * ECLLocalRunCalibUnit is
   * the class designed for the
   * control of mean value and
   * the standard deviation accumulators.
   */
  class ECLLocalRunCalibUnit {
  public:
    /**
     * Constructor.
     * @param ncellids is the number of cell ids.
     * @param min_value is the minimum allowed value.
     * @param max_value is the maximum allowed value.
     * @param ndevs is the number of standard deviations.
     */
    ECLLocalRunCalibUnit(const int& ncellids,
                         const float& min_value,
                         const float& max_value,
                         const int* const ndevs);
    /**
     * Destructor.
     */
    ~ECLLocalRunCalibUnit();
    /**
     * Add value to accumulate
     * mean value, standard deviation
     * and number of accepted events.
     * @param cellid is the cell id number.
     * @param value is the accumulated value.
     */
    void add(const int& cellid, const float& value);
    /**
     * Calculate accumulated values.
     */
    void calc();
    /**
     * Write calibration results into
     * a database.
     * @param isLocal enables usage of a local database.
     * @param dbName is the central database tag or path to a local database.
     * @param payloadName is the payload name.
     * @param iov is the validity interval.
     * @param run is the current run number.
     * @param changePrev enables previous validity interval changing.
     * @param addref is used to mark current run as reference.
     */
    void writeToDB(
      bool isLocal,
      const std::string& dbName,
      const std::string& payloadName,
      const IntervalOfValidity& iov,
      const int& run,
      const bool& changePrev,
      const bool& addref);
    /**
     * This function will be called
     * only in the case, if negative
     * amplitudes are observed in the
     * current run.
     */
    void enableNegAmpl();
    /**
     * Check presence
     * of negative amplitudes
     * in the current run.
     */
    bool isNegAmpl() const;
  private:
    /**
     * m_isNegAmpl is true
     * if there are negative
     * amplitudes in the current
     * run and false otherwise.
     */
    bool m_isNegAmpl;
    /**
     * Mark current run as reference.
     * @param isLocal is true for local database
     * @param dbNaem is central DB tag or path to a local DB.
     * @param run is the current run number.
     * @param iov is the current validity interval.
     */
    void markAsRefference(const bool& isLocal,
                          const std::string& dbName,
                          const int& run,
                          const IntervalOfValidity& iov);
    /**
     * Change previous validity interval.
     * @param payload is the connection to a certain payload.
     * @param iov is the current validity interval.
     */
    void changePreviousIoV(const ECLDBTool& payload,
                           const IntervalOfValidity& iov);
    /**
     * Mean value and standard deviation
     * accumulators for each cell id.
     */
    std::vector<ECLLocalRunCalibAcc> m_unitData;
    /**
     * Getter of accumulated values.
     * @param vec is the vector used to get vector of accumulated values.
     * @param getter is the name of the getter method.
     */
    template <typename T>
    void callAccGetter(
      std::vector<T>* vec,
      T(ECLLocalRunCalibAcc::*getter)() const);
  };
} // Belle2
#endif
