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
    /** set prescale of i-th output trigger bit**/
    void setprescales(int i, int j)
    {
      m_prescales[i] = j;
    }
    /** set the number of output trigger bit**/
    void setnoutbit(int i)
    {
      m_noutbit = i;
    }
    /** get prescale of i-th output trigger bit**/
    int getprescales(int i) const
    {
      return m_prescales[i];
    }
    /** get the number of output trigger bit**/
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
