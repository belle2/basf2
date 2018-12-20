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

  /** The payload class for GDL psnm
   *
   *  The payload class stores the number of psnm bit and their prescale values
   *  value of a missing bit is "0"
   */


  class TRGGDLDBPrescales: public TObject {
  public:

    /** Default constructor */
    TRGGDLDBPrescales(): m_noutbit{0}, m_prescales{0} {}
    /** copy constructor */
    TRGGDLDBPrescales(const TRGGDLDBPrescales& b): TObject(b)
    {
      m_noutbit = b.m_noutbit;
      for (int i = 0; i < 320; i++) {
        m_prescales[i] = b.m_prescales[i];
      }
    }
    /** assignment operator */
    TRGGDLDBPrescales& operator=(const TRGGDLDBPrescales& b)
    {
      m_noutbit = b.m_noutbit;
      for (int i = 0; i < 320; i++) {
        m_prescales[i] = b.m_prescales[i];
      }
      return *this;
    }

    void setprescales(int i, int j)
    {
      m_prescales[i] = j;
    }
    void setnoutbit(int i)
    {
      m_noutbit = i;
    }

    int getprescales(int i) const
    {
      return m_prescales[i];
    }
    int getnoutbit() const
    {
      return m_noutbit;
    }

  private:

    /** Number of bit */
    int m_noutbit;

    /** Number of bins per sensor along u side */
    int m_prescales[320];

    ClassDef(TRGGDLDBPrescales, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2
