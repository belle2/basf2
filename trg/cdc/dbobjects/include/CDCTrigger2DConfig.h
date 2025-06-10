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



  /** The payload class for delay of GDL input bit
   *
   *  The payload class stores the number of input bit and their delay
   *  value of a missing delay is "-1"
   */


  class CDCTrigger2DConfig: public TObject {
  public:

    /** Default constructor */
    CDCTrigger2DConfig(): m_nTS(10), m_fullhit(false), m_hitthreshold(4), m_ADC(false) {}
    /** copy constructor */
    CDCTrigger2DConfig(const CDCTrigger2DConfig& b): TObject(b)
    {
      m_nTS = b.m_nTS;
    }
    /** assignment operator */
    CDCTrigger2DConfig& operator=(const CDCTrigger2DConfig& b)
    {
      m_nTS = b.m_nTS;
      return *this;
    }

    /** Get the number of TS */
    int getnTS() const
    {
      return m_nTS;
    }
    /** Set the number of TS */
    void setnTS(int i)
    {
      m_nTS = i;
    }

    /** Get full wire hit enable flag */
    bool getfullhit() const
    {
      return m_fullhit;
    }
    /** Set full wire hit enable flag */
    void setfullhit(bool i)
    {
      m_fullhit = i;
    }

    /** Get the hit threshold */
    int gethitthreshold() const
    {
      return m_hitthreshold;
    }
    /** Set the hit threshold */
    void sethitthreshold(int i)
    {
      m_hitthreshold = i;
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

  private:

    /** Number of TS */
    int m_nTS;
    /** use full wirehit or not */
    bool m_fullhit;
    /** required number of TS or wire hits for 2D track finding*/
    int m_hitthreshold;
    /** use ADC or not */
    bool m_ADC;

    ClassDef(CDCTrigger2DConfig, 2);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2
