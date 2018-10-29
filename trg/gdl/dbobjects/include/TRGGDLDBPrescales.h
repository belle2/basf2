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

  class TRGGDLDBPrescales: public TObject {
  public:

    /** Default constructor */
    TRGGDLDBPrescales(): m_prescales{0} {}
    /** copy constructor */
    TRGGDLDBPrescales(const TRGGDLDBPrescales& b)
    {
      for (int i = 0; i < 200; i++) {
        m_prescales[i] = b.m_prescales[i];
      }
    }
    /** assignment operator */
    TRGGDLDBPrescales& operator=(const TRGGDLDBPrescales& b)
    {
      for (int i = 0; i < 200; i++) {
        m_prescales[i] = b.m_prescales[i];
      }
      return *this;
    }

    void setprescales(int i, int j)
    {
      m_prescales[i] = j;
    }

    int getprescales(int i) const
    {
      return m_prescales[i];
    }

  private:

    /** Number of bins per sensor along u side */
    int m_prescales[200];

    ClassDef(TRGGDLDBPrescales, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2
