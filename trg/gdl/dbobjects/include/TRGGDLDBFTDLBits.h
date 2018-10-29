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

  class TRGGDLDBFTDLBits: public TObject {
  public:

    /** Default constructor */
    TRGGDLDBFTDLBits(): m_outbitname{0} {}
    /** copy constructor */
    TRGGDLDBFTDLBits(const TRGGDLDBFTDLBits& b)
    {
      for (int i = 0; i < 200; i++) {
        strcpy(m_outbitname[i], b.m_outbitname[i]);
      }
    }
    /** assignment operator */
    TRGGDLDBFTDLBits& operator=(const TRGGDLDBFTDLBits& b)
    {
      for (int i = 0; i < 200; i++) {
        strcpy(m_outbitname[i], b.m_outbitname[i]);
      }
      return *this;
    }

    void setoutbitname(int i, const char* c)
    {
      strcpy(m_outbitname[i], c);
    }

    const char* getoutbitname(int i) const
    {
      return m_outbitname[i];
    }

  private:

    /** Number of bins per sensor along u side */
    char m_outbitname[200][100];

    ClassDef(TRGGDLDBFTDLBits, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2
