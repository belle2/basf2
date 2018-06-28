/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * ECLLocalRunCalibratorModule                                            *
 *                                                                        *
 * This module computes the mean values and standard deviations           *
 * for the fitted time and amplitude.                                     *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su) (BINP),        *
 * Vitaly Vorobyev (vvorob@inp.nsk.su) (BINP)                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef _ECLLocalRunCalibratorModule_H_
#define _ECLLocalRunCalibratorModule_H_
// STL
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>
#include <utility>
// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/database/IntervalOfValidity.h>
// ECL
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dbobjects/ECLCrystalLocalRunCalib.h>
#include <ecl/modules/eclLocalRunCalibration/ECLLocalRunCalibUnit.h>
// ROOT
#include <TObject.h>
#include <TTree.h>
#include <TH1F.h>
namespace Belle2 {
  /**
   * ECLLocalRunCalibratorModule
   * is the module developed to perform
   * ECL local run calibration.
   */
  class ECLLocalRunCalibratorModule : public Module {
  public:
    /**
     * Constructor.
     */
    ECLLocalRunCalibratorModule();
    /**
     * Destructor.
     */
    ~ECLLocalRunCalibratorModule();
    /**
     * Begin run.
     */
    void beginRun() override;
    /**
     * Event.
     */
    void event();
    /**
     * End run.
     */
    void endRun();
  private:
    /**
     * Calculate time shift.
     * @param digit is input time ecl digit.
     */
    int16_t getTimeShift(const ECLDigit& digit) const;
    /**
     * Decode time.
     * @param time is the raw time value.
     */
    inline uint32_t decodeTrigTime(uint32_t time) const;
    /**
     * Number of cell ids.
     */
    static const int c_ncellids;
    /**
     * If m_isFillHisto is false, than
     * the mode of local run calibration
     * is enabled. If m_isFillHisto is
     * true, than the mode of histogram filling
     * mode is enabled. In the second case, the
     * only output result of the module is the file
     * with the amplitude and time distributions for a
     * certain cell id (m_cellid) in a certain run.
     */
    bool m_isFillHisto;
    /**
     * cell id number.
     * This variable is used only in
     * the histogram filling mode.
     */
    int m_cellid;
    /**
     * The path of the .root
     * file with the histograms
     * obtained in the histogram
     * filling mode.
     */
    std::string m_histoFileName;
    /**
     * Minimum allowed time value.
     */
    float m_minTime;
    /**
     * Maximum allowed time value.
     */
    float m_maxTime;
    /**
     * Minimum allowed amplitude value.
     */
    float m_minAmpl;
    /**
     * Maximum allowed amplitude value.
     */
    float m_maxAmpl;
    /**
     * Number of standard
     * deviations used to
     * update value limits.
     */
    int m_devs;
    /**
     * Tag of central database
     * or path to a local database.
     */
    std::string m_dbName;
    /**
     * Enables local database
     * usage.
     */
    bool m_isLocal;
    /**
     * If m_changePrev is true,
     * the validity intervals of
     * the previous payloads stored
     * into the database will be changed.
     * The default validity interval for the
     * certain run is [certain run number, -1 == inf).
     * The validity interval for the previous run will
     * be changer form [previous run number, -1) to
     * [previous run number, current run number - 1)
     */
    bool m_changePrev;
    /**
     * If m_addref is true, then
     * the current calibration
     * run will be marked as reference
     * run immediately after saving to
     * the database.
     */
    bool m_addref;
    /**
     * m_histo_time is the name of
     * the histogram, which contains
     * time distribution for a
     * certain cell id. This histogram
     * will be filled only if the histogram
     * filling mode is enabled.
     */
    TH1F* m_histo_time;
    /**
     * m_histo_ampl is the name of
     * the histogram, which contains
     * amplitude distribution for a
     * certain cell id. This histogram
     * will be filled only if the histogram
     * filling mode is enabled.
     */
    TH1F* m_histo_ampl;
    /**
     * Name of the time payload.
     */
    static const std::string c_timePayloadName;
    /**
     * Name of the amplitude payload.
     */
    static const std::string c_amplPayloadName;
    /**
     * Low run of the validity interval.
     */
    int m_lowRun;
    /**
     * High run of the validity interval.
     */
    int m_highRun;
    /**
     * m_time contains time mean value and standard
     * deviation accumulators for all cell ids.
     */
    ECLLocalRunCalibUnit* m_time;
    /**
     * m_ampl contains amplitude mean value and standard
     * deviation accumulators for all cell ids.
     */
    ECLLocalRunCalibUnit* m_ampl;
    /**
     * Write histograms to file in
     * the case, if the histogram
     * filling mode is enabled.
     */
    void writeHistoToFile();
    /**
     * Write calibration results into
     * a database.
     */
    void writeCalibResultsToDB();
  };
}  // namespace Belle2

#endif
