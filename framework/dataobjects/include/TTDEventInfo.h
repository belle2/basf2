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

#include <limits>

#include <TObject.h>

namespace Belle2 {
  /**
   * Storage element for information from the Trigger Timing Distribution (TTD).
   **/
  class TTDEventInfo : public TObject {

  public:
    /// Constructor
    explicit TTDEventInfo(bool isValid = false, bool isHER = false, bool revo2 = false,
                          unsigned int timeSinceLastInjection = std::numeric_limits<unsigned int>::max(),
                          unsigned int bunchNumber = std::numeric_limits<unsigned int>::max()) :
      m_isValid(isValid), m_isHER(isHER), m_revo2(revo2), m_timeSinceLastInjection(timeSinceLastInjection),
      m_bunchNumber(bunchNumber) {}
    /// Destructor
    ~TTDEventInfo() {}

    // Getters
    /// get if stored information is valid
    bool isValid() {return m_isValid;}
    /// get if injection in HER/LER
    bool isHER() {return m_isHER;}
    /// get lowest bit of revolution counter
    bool revo2() {return m_revo2;}
    /// get time since the last injection in clock ticks (FTSW clock)
    unsigned int getTimeSinceLastInjection() {return m_timeSinceLastInjection;}
    /// get number of injected bunch
    unsigned int getBunchNumber() {return m_bunchNumber;}
    /// get if an injection happened recently (and the corresponding stored data is actually valid)
    bool hasInjection() {return m_timeSinceLastInjection == std::numeric_limits<unsigned int>::max();}

    // Setters
    /// set that stored information is valid
    void setValid() {m_isValid = true;}
    /// set that stored information is valid
    void setInvalid() {m_isValid = false;}
    /// set if injection in HER/LER
    void setIsHER(bool isHER) {m_isHER = isHER;}
    /// set lowest bit of revolution counter
    void setRevo2(bool revo2) {m_revo2 = revo2;}
    /// set time since the last injection in clock ticks (FTSW clock)
    void setTimeSinceLastInjection(unsigned int timeSinceLastInjection) {m_timeSinceLastInjection = timeSinceLastInjection;}
    /// set number of injected bunch
    void setBunchNumber(unsigned int bunchNumber) {m_bunchNumber = bunchNumber;}
    /// set that no injection happened recently (and the corresponding stored data is actually invalid)
    void setNoInjection() {m_timeSinceLastInjection = std::numeric_limits<unsigned int>::max();}


  private:
    /// Data stored in the TTD info is actually valid
    bool m_isValid;
    /// Injection in HER/LER
    bool m_isHER;
    /// Lowest bit of revolution counter (PXD needs ~2 revolutions to readout one frame)
    bool m_revo2;
    /// Time since the last injection in clock ticks (FTSW clock)
    unsigned int m_timeSinceLastInjection;
    /// Number of injected bunch
    unsigned int m_bunchNumber;

    ClassDef(TTDEventInfo, 1) ///< Storage element for TTD information
  };
}
