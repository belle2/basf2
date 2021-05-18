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

#include <TObject.h>

namespace Belle2 {
  /**
   * Storage element for information from the Trigger Timing Distribution (TTD).
   * */
  class TTDEventInfo : public TObject {

  public:

    /// Default constructor
    TTDEventInfo() {}
    /// Destructor
    ~TTDEventInfo() {}

    // Getters
    /// get if injection in HER/LER
    bool getIsHER() {return m_isHER;}
    /// get time since the last injection in clock ticks (FTSW clock)
    unsigned int getTimeSinceLastInjection() {return m_timeSinceLastInjection;}
    /// get number of injected bunch
    unsigned int getBunchNumber() {return m_bunchNumber;}
    /// get number of revolutions/2 (counting every other revolution, corresponding to readout of one PXD frame)
    unsigned int getRevo2() {return m_revo2;}

    // Setters
    /// set if injection in HER/LER
    void setIsHER(bool isHER) {m_isHER = isHER;}
    /// set time since the last injection in clock ticks (FTSW clock)
    void setTimeSinceLastInjection(unsigned int timeSinceLastInjection) {m_timeSinceLastInjection = timeSinceLastInjection;}
    /// set number of injected bunch
    void setBunchNumber(unsigned int bunchNumber) {m_bunchNumber = bunchNumber;}
    /// set number of revolutions/2 (counting every other revolution, corresponding to readout of one PXD frame)
    void setRevo2(unsigned int revo2) {m_revo2 = revo2;}


  private:
    /// Injection in HER/LER
    bool m_isHER = false;
    /// Time since the last injection in clock ticks (FTSW clock)
    unsigned int m_timeSinceLastInjection = 0;
    /// Number of injected bunch
    unsigned int m_bunchNumber = 0;
    /// Number of revolutions/2 (counting every other revolution, corresponding to readout of one PXD frame)
    unsigned int m_revo2 = 0;

    ClassDef(TTDEventInfo, 1) ///< Storage element for TTD information
  };
}
