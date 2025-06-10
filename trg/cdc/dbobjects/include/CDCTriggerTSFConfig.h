/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TObject.h>

namespace Belle2 {



  /** The payload class for TSF run-dependent parameter **/


  class CDCTriggerTSFConfig: public TObject {
  public:

    /** Default constructor */
    CDCTriggerTSFConfig(): m_TDCfilter(false), m_ADC(false), m_ADC_threshold(-1) {}
    /** copy constructor */
    CDCTriggerTSFConfig(const CDCTriggerTSFConfig& b): TObject(b)
    {
      m_TDCfilter = b.m_TDCfilter;
      m_ADC = b.m_ADC;
      m_ADC_threshold = b.m_ADC_threshold;
    }
    /** assignment operator */
    CDCTriggerTSFConfig& operator=(const CDCTriggerTSFConfig& b)
    {
      m_TDCfilter = b.m_TDCfilter;
      m_ADC = b.m_ADC;
      m_ADC_threshold = b.m_ADC_threshold;
      return *this;
    }


    /** Get TDCfilter enable flag */
    bool getTDCfilter() const
    {
      return m_TDCfilter;
    }
    /** Set TDCfilter enable flag */
    void setTDCfilter(bool i)
    {
      m_TDCfilter = i;
    }

    /** Get ADC enable flag */
    bool getADC() const
    {
      return m_ADC;
    }
    /** Set ADC enable flag */
    void setADC(bool i)
    {
      m_ADC = i;
    }

    /** Get the ADC_ threshold */
    int getADC_threshold() const
    {
      return m_ADC_threshold;
    }
    /** Set the ADC_ threshold */
    void setADC_threshold(int i)
    {
      m_ADC_threshold = i;
    }


  private:

    /** use TDCfilter or not */
    bool m_TDCfilter;
    /** use ADC or not */
    bool m_ADC;
    /** ADC threshold required for each wire*/
    int m_ADC_threshold;

    ClassDef(CDCTriggerTSFConfig, 2);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2
