/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <bitset>
#include <TObject.h>

namespace Belle2 {
  /**
   * Storage element for information from the Trigger Timing Distribution (TTD).
   **/
  class TTDEventInfo : public TObject {

  public:
    /// Default constructor
    TTDEventInfo() {}
    /// Destructor
    ~TTDEventInfo() {}

    // Getters
    /// get if an injection happened recently (and the stored data is actually valid)
    bool getHasInjection() {return m_TTDflags[c_hasInjection];}
    /// get if injection in HER/LER
    bool getIsHER() {return m_TTDflags[c_isHER];}
    /// get lowest bit of revolution counter
    bool getRevo2() {return m_TTDflags[c_revo2];}
    /// get time since the last injection in clock ticks (FTSW clock)
    unsigned int getTimeSinceLastInjection() {return m_timeSinceLastInjection;}
    /// get number of injected bunch
    unsigned int getBunchNumber() {return m_bunchNumber;}

    // Setters
    /// set that injection happened recently
    void setHasInjection(bool isInjection) {m_TTDflags[c_hasInjection] = isInjection;}
    /// set if injection in HER/LER
    void setIsHER(bool isHER) {m_TTDflags[c_isHER] = isHER;}
    /// set lowest bit of revolution counter
    void setRevo2(bool revo2) {m_TTDflags[c_revo2] = revo2;}
    /// set time since the last injection in clock ticks (FTSW clock)
    void setTimeSinceLastInjection(unsigned int timeSinceLastInjection) {m_timeSinceLastInjection = timeSinceLastInjection;}
    /// set number of injected bunch
    void setBunchNumber(unsigned int bunchNumber) {m_bunchNumber = bunchNumber;}


  private:
    /// Use a bitset to save some space in file (compared to individual bools)
    std::bitset<4> m_TTDflags{"0000"};
    /// Enum to specify meaning of bits in m_flags bitset
    /// DOES THIS HAVE TO GO TO A .cc FILE SO IT IS NOT WRITTEN TO A FILE?
    enum E_TTDflags {
      c_hasInjection = 0, ///< the data stored in the TTDEventInfo is actually valid and an injection took place recently
      c_isHER        = 1, ///< injection in HER/LER
      c_revo2        = 2, ///< lowest bit of revolution counter (PXD needs ~2 revolutions to readout one frame)
      c_UNUSED       = 3, ///< UNUSED: maybe we could set a flag if it was a 2-bunch injection?
    };
    /// Time since the last injection in clock ticks (FTSW clock)
    unsigned int m_timeSinceLastInjection = 0;
    /// Number of injected bunch
    unsigned int m_bunchNumber = 0;

    ClassDef(TTDEventInfo, 1) ///< Storage element for TTD information
  };
}
