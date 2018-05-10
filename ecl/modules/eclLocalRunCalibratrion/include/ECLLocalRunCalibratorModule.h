/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Local run Digit Calibration.                                           *
 *                                                                        *
 * This module computes the averages and RMS values for the fitted        *
 * time and amplitude                                                     *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Vitaly Vorobyev (vvorob@inp.nsk.su) (BINP),              *
 * Sergei Gribanov (S.S.Gribanov@inp.nsk.su) (BINP)                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLDIGITCALIBRATORMODULE_H_
#define ECLDIGITCALIBRATORMODULE_H_

// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBObjPtr.h>

// ECL
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>

// STL
#include <vector>
#include <string>
#include <unordered_map>
#include <cstdint>
//#include <memory>

namespace Belle2 {
  /** The ECLLocalRunCalibratorModule module serves for daily ECL
   *  calibrations. It accumulates amplitude and time from each
   *  crystal and calculates mean values and standard deviations.
   *  The mean and std. values are written in the database.
   *
   *  The module operates in the two switchable modes:
   *    1. The reference mode. Raw amplitude and time values are
   *       processed and stored.
   *    2. The normal mode. The amplitude and time normalized
   *       on some reference values are processd and stored.
   *
   *  Module parameters:
   *    refMode (bool)  : activate the reference mode if true
   *    minCounts (int) : number of counts to be collected without checking.
   *                      After *minCounts* counts are collected, a new input
   *                      is checked if it is within the acceptance range
   *    maxDef (int)    : defines the acceptance range in terms of numer of
   *                      RMS values from the mean value. Each value is accepted
   *                      if maxDef equals zero
   */
  class ECLLocalRunCalibratorModule : public Module {
    class Unit;  // forward declaration for type alias
    // Type aliases
    using UnitMap = std::map<std::string, std::vector<Unit>>;
    using FloatMap = std::map<std::string, std::vector<float>>;
    using StrMap = std::map<std::string, std::string>;
    // Static constants
    /** Number of ECL crystals */
    static constexpr uint16_t c_nCrystals = 8736;
    /** Min number of counts to activate input regection procedure */
    static constexpr uint16_t c_minCounts = 100;
    /** Max number ofi misfit counts */
    static constexpr uint16_t c_maxMisfits = 5;
    /** Time value corresponding to unknown time (no pulse shape fit) */
    static constexpr int16_t c_wrongTime = -2048;
    /** String key for time */
    static const std::string c_timeStr;
    /** String key for amplitude */
    static const std::string c_amplStr;
    /** Map for the reference database tags */
    static const StrMap c_tagMapRef;
    /** Map for calibration (normal) database tags */
    static const StrMap c_tagMap;
    /** Name of the ECLDigit.*/
    static const std::string c_eclDigitArrayName;
    /** Name of the ECLTrigs.*/
    static const std::string c_eclTrigsArrayName;
    /** The Unit class calculates mean and RMS values for a stream
     *  of integer values. It can reject input if the distance to
     *  the mean value exceeds the m_maxDev x RMS value */
    class Unit {
      /** Current mean value */
      float mean;
      /** Current mean of squares */
      float variance;
      /** Counter for accepted imputs */
      uint32_t counter;
      /** Counter for rejected inputs */
      uint32_t misfits;

    public:
      /** Constructor */
      Unit();
      /** Add value and update the unit */
      void add(float value);
      /** Returns true if the value is within the acceptance region
       *  and false otherwise */
      bool isMisfit(float value);
      /** Returns estimate for the mean value of accepted inputs */
      float getMean() const;
      /** Returns estimate for the standard deviation of accepted inputs */
      float getStddev() const;
      /** Returns the number of accepted inputs */
      uint32_t getCounter() const;
      /** Returns the number of rejectd inputs */
      uint32_t getMisfits() const;
      /** Number of standard deviations defining the acceptance range */
      static uint32_t m_maxDev;
      /** Number of counts required to apply the acceptance rule */
      static uint32_t m_minCounts;
    };
    /** Map to store vectors of Units for time and amplitude */
    UnitMap m_aveMap;
    /** Map of reference values of amplitude and time */
    FloatMap m_refMap;
    /** Map of reference uncertainties of amplitude and time */
    FloatMap m_refMapUnc;
    /** If true then the module makes a reference record.
     *  If false then the module makes a calibration record
     *  including comparizon with a reference record */
    bool m_refMode;
    /** Print all results in stdout **/
    bool m_verb;
    float m_max_time_offset;
    float m_max_ampl_offset;
    uint16_t m_max_misfit;
    std::map<std::string, float> m_max_offset;
    /** Analizes the collected numbers, looks for suspecious
     *  values and prints info in std output */
    void checkResults(const std::string& key) const;
    void checkOffset(const std::string& key) const;
    /** Fills and returns DB object. The same interface for time and amplitude */
    void saveObj(const std::string& key, const IntervalOfValidity& iov) const;
    /** Read collector's time to tune the time starting point */
    int16_t getTimeShift(const ECLDigit& eclg) const;
    /** Decode time obtaned from collector */
    uint32_t decodeTrigTime(uint32_t time) const;
    /** Read reference DB entry and fill the reference map */
    void fillReferenceMap(const std::string& key);

  public:
    /** Constructor. */
    ECLLocalRunCalibratorModule();
    /** Destructor. */
    ~ECLLocalRunCalibratorModule() {}
    /** Initialize variables. */
    void initialize() override;
    /** begin run.*/
    void beginRun() override;
    /** event per event. */
    void event() override;
    /** end run. */
    void endRun() override;
    /** terminate.*/
    void terminate() override {}
  };

}  // end Belle2 namespace

#endif

