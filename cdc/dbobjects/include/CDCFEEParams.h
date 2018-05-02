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
   * Database object for FEE params.
   */
  class CDCFEEParams: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCFEEParams() {}

    /**
     * Constructor
     */
    CDCFEEParams(short width, short trgDelay, short tThmV, short aTh, short late, short tTheV) : m_widthOfTimeWindow(width),
      m_trgDelay(trgDelay), m_tdcThreshInmV(tThmV), m_adcThresh(aTh), m_l1TrgLatency(late), m_tdcThreshIneV(tTheV)
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
    short m_tdcThreshInmV = 70;     /**< Threshold for timing signal (mV) */
    short m_adcThresh = 1;          /**< Threshold for FADC (count) */
    short m_l1TrgLatency = 4900;    /**< L1 trigger latency (in unit of TDC resol.) */
    short m_tdcThreshIneV = 160;    /**< Threshold for timing signal (eV) */

    ClassDef(CDCFEEParams, 1); /**< ClassDef */
  };

} // end namespace Belle2
