/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: CDC group                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <TObject.h>

namespace Belle2 {
  /**
   * Database object for Fron-endt electronics params.
   */
  class CDCFEElectronics: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCFEElectronics() {}

    /**
     * Constructor
     */
    CDCFEElectronics(short width, short trgDelay, short aTh, short tThmV, short tTheV, short late) : m_widthOfTimeWindow(width),
      m_trgDelay(trgDelay), m_adcThresh(aTh), m_tdcThreshInmV(tThmV), m_tdcThreshIneV(tTheV), m_l1TrgLatency(late)
    {
    }

    /** Getter for width of time window */
    short getWidthOfTimeWindow() const
    {
      return m_widthOfTimeWindow;
    }

    /** Getter for trigger delay */
    short getTrgDelay() const
    {
      return m_trgDelay;
    }

    /** Getter for threshold (mV) for timing-signal */
    short getTDCThreshInmV() const
    {
      return m_tdcThreshInmV;
    }

    /** Getter for threshold for FADC */
    short getADCThresh() const
    {
      return m_adcThresh;
    }

    /** Getter for L1 trigger latency */
    short getL1TrgLatency() const
    {
      return m_l1TrgLatency;
    }

    /** Getter for threshold (eV) for timing-signal */
    short getTDCThreshIneV() const
    {
      return m_tdcThreshIneV;
    }

  private:
    short m_widthOfTimeWindow = 29; /**< Width of time window (in unit of 32*(TDC resol.)) */
    short m_trgDelay = 133;         /**< Trigger delay (in unit of 32*(TDC resol.)) */
    short m_adcThresh = 2;          /**< Threshold for FADC (count) */
    short m_tdcThreshInmV = 70;     /**< Threshold for timing signal (mV); may be used in waveform sim. */
    short m_tdcThreshIneV = 160;    /**< Threshold for timing signal (eV) */
    short m_l1TrgLatency = 4900;    /**< L1 trigger latency (in unit of TDC resol.) */

    ClassDef(CDCFEElectronics, 1); /**< ClassDef */
  };

} // end namespace Belle2
