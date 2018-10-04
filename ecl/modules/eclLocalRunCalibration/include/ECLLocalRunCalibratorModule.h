
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Local run Digit Calibration.                                           *
 *                                                                        *
 * This module computes the averages and standard deviation values for    *
 * the fitted time and amplitude                                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su),               *
 * Vitaly Vorobyev (vvorob@inp.nsk.su) (BINP)                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
// ECL
#include <ecl/modules/eclLocalRunCalibration/ECLLocalRunCalibrationData.h>
// ROOT
// STL
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>
#include <utility>

// Forward declarations (to reduce number of include directives in the header)
class TTree;
namespace Belle2 {
  class ECLDigit;
  class ECLCrystalCalib;
}

namespace Belle2 {
  /**
   * ECLLocalRunCalibratorModule
   * is the module developed to perfrom
   * ECL local run calibration
   */
  class ECLLocalRunCalibratorModule : public Module {
  public:
    /**
     * Constructor
     */
    ECLLocalRunCalibratorModule();
    /**
     * Destructor
     */
    ~ECLLocalRunCalibratorModule();
    /**
     * Begin run
     */
    void beginRun() override;
    /**
     * Event
     */
    void event() override;
    /**
     * End run
     */
    void endRun() override;
  private:
    /**
     * Debug tree map
     */
    std::unordered_map<std::string, TTree*> m_treeMap;
    /**
     * Data vector
     */
    using DataVect = std::vector<ECLLocalRunCalibrationData>;
    /**
     * Data map.
     * Keys sorespond
     * to amplitude and time.
     */
    using DataMap = std::unordered_map<std::string, DataVect>;
    /**
     * String map
     */
    using StringMap = std::unordered_map<std::string, std::string>;
    /**
     * Number of ECL crystals
     */
    static constexpr uint16_t c_nCrystals = 8736;
    /**
     * Time value corresponding
     * to unknown time (no pulse shape fit)
     */
    static constexpr int16_t c_wrongTime = -2048;
    /**
     * Input array
     */
    static const std::string c_eclDigitArrayName;
    /**
     * Time key
     */
    static const std::string c_timeKey;
    /**
     * Amplitude key
     */
    static const std::string c_amplKey;
    /**
     * Payload names for
     * reference run.
     * Map keys correspond to
     * amplitude and time.
     */
    static const StringMap c_refTags;
    /**
     * Payload names for
     * calibration run.
     * Map keys correspond to
     * amplitude and time.
     */
    static const StringMap c_calibTags;
    /**
     * Enables reference mode
     */
    bool m_refMode;
    /**
     * If m_verbose
     * is true, a short
     * summarry will be
     * printed into standard
     * output.
     */
    bool m_verbose;
    /**
     * Left time limit
     */
    float m_minTime;
    /**
     * Right time limit
     */
    float m_maxTime;
    /**
     * Left amplitude limit
     */
    float m_minAmpl;
    /**
     * Right amplitude limit
     */
    float m_maxAmpl;
    /**
     * Maximum time offset
     * relative to reference run
     */
    float m_maxTimeOffset;
    /**
     * Maximum amplitude offset
     * relative to reference run
     */
    float m_maxAmplOffset;
    /**
     * Number of events
     * to initialize left and
     * right time (amplitude) limits
     */
    int m_initNOfEvents;
    /**
     * Number of standard deviations
     * to initialize left and right
     * value limits
     */
    int m_nOfStdDevs;
    /**
     * Path to the file with debug
     * information. If m_debugFile==""
     * than the file will not
     * be generated
     */
    std::string m_debugFile;
    /**
     * m_data contains contains
     * ECLLocalRunCalibrationData object
     * for each crystal. ECLLocalRunCalibrationData
     * class provides methods to accamulate mean
     * values and standard deviations. m_data
     * has two keys, which correspond to time
     * and amplitude.
     */
    DataMap m_data;
    /**
     * Pointer to EventMetaData in Data Store.
     */
    StoreObjPtr<EventMetaData> m_evtPtr;
    /** Store Array of input events */
    StoreArray<ECLDigit> m_eclDigits;
    /**
     * Read collector's time to tune
     * the time starting point.
     * @param digit an ECLDigit object
     */
    int16_t getTimeShift(const ECLDigit& digit) const;
    /**
     * Decode time obtaned
     * from collector.
     * @param time encoded time value
     */
    inline uint32_t decodeTrigTime(uint32_t time) const;
    /**
     * Write mean values and
     * standard deviations
     * to database.
     * @param key the amplitude or time key
     * @param iov an iterval of validity
     */
    void writeObjToDB(const std::string& key,
                      const IntervalOfValidity& iov);
    /**
     * Normalize mean value
     * @param key the amplitude or time key
     * @param calibMean a mean value for
     calibration run
     * @param refMean a mean value for reference run
     */
    float normalizeMean(const std::string& key,
                        const float& calibMean,
                        const float& refMean) const;
    /**
     * Normalize standard deviation
     * @param calibStdDev a standard deviation
     for calibration run
     * @param refStdDev a standard deviation
     for reference run
    */
    float normalizeStdDev(const float& calibStdDev,
                          const float& refStdDev) const;
    /**
     * Fill debug trees for
     * calibration run.
     * @param key the amplitude or time key
     * @param refArray mean values and
     standard deviations for reference run
    */
    void fillDebugCalibTree(
      const std::string& key,
      const DBObjPtr<ECLCrystalCalib>& refArray);
    /**
     * Fill debug trees for
     * reference run.
     * @param key the amplitude or time key
     */
    void fillDebugRefTree(const std::string& key);
    /**
     * Write debug trees to file
     */
    void writeToFile() const;
    /**
     * Print summary.
     * @param key the amplitude or time key
     */
    void printSummary(const std::string& key) const;
  };
}  // namespace Belle2

