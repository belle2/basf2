/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <unordered_map>
#include <vector>

namespace Belle2 {

  /** The payload class for PXD cluster charge calibrations
   *
   *  The payload class stores the median cluster charge values
   *  for PXD sensors on a grid nBinsU x nBinsV. The values are
   *  stored in raw ADC units (called ADU).
   *
   *  The granularity of the grid is user adjustable. The default
   *  value of a missing calibraiton is -1.0.
   */

  class TRGGDLDBDelay: public TObject {
  public:

    /** Default constructor */
    TRGGDLDBDelay(): m_delay{0} {}
    /** copy constructor */
    TRGGDLDBDelay(const TRGGDLDBDelay& b)
    {
      for (int i = 0; i < 200; i++) {
        m_delay[i] = b.m_delay[i];
      }
    }
    /** assignment operator */
    TRGGDLDBDelay& operator=(const TRGGDLDBDelay& b)
    {
      for (int i = 0; i < 200; i++) {
        m_delay[i] = b.m_delay[i];
      }
      return *this;
    }

    void setdelay(int i, const int j)
    {
      m_delay[i] = j;
    }

    int getdelay(int i) const
    {
      return m_delay[i];
    }

  private:

    /** Number of bins per sensor along u side */
    int m_delay[200];

    ClassDef(TRGGDLDBDelay, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2
