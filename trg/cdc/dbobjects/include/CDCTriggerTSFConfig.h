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
    CDCTriggerTSFConfig(): m_useTDCfilter(false), m_useADC(false), m_ADC_threshold(-1) {}

    /** Get TDCfilter enable flag */
    bool getuseTDCfilter() const
    {
      return m_useTDCfilter;
    }
    /** Set TDCfilter enable flag */
    void setuseTDCfilter(bool i)
    {
      m_useTDCfilter = i;
    }

    /** Get ADC enable flag */
    bool getuseADC() const
    {
      return m_useADC;
    }
    /** Set ADC enable flag */
    void setuseADC(bool i)
    {
      m_useADC = i;
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
    bool m_useTDCfilter;
    /** use ADC or not */
    bool m_useADC;
    /** ADC threshold required for each wire*/
    int m_ADC_threshold;

    ClassDef(CDCTriggerTSFConfig, 3);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2
