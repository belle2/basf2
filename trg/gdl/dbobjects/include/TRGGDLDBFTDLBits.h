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

  /** The payload class for FTDL output bit
   *
   *  The payload class stores the number of output bit and their name
   *  value of a missing name is ""
   */

  class TRGGDLDBFTDLBits: public TObject {
  public:

    /** Default constructor */
    TRGGDLDBFTDLBits(): m_noutbit{0}, m_outbitname{} {}
    /** copy constructor */
    TRGGDLDBFTDLBits(const TRGGDLDBFTDLBits& b)
    {
      m_noutbit = b.m_noutbit;
      for (int i = 0; i < 320; i++) {
        strcpy(m_outbitname[i], b.m_outbitname[i]);
      }
    }
    /** assignment operator */
    TRGGDLDBFTDLBits& operator=(const TRGGDLDBFTDLBits& b)
    {
      m_noutbit = b.m_noutbit;
      for (int i = 0; i < 320; i++) {
        strcpy(m_outbitname[i], b.m_outbitname[i]);
      }
      return *this;
    }

    void setoutbitname(int i, const char* c)
    {
      strcpy(m_outbitname[i], c);
    }
    void setnoutbit(int i)
    {
      m_noutbit = i;
    }

    const char* getoutbitname(int i) const
    {
      return m_outbitname[i];
    }
    int getnoutbit() const
    {
      return m_noutbit;
    }

  private:

    /** Number of bit */
    int m_noutbit;

    /** Name of bit */
    char m_outbitname[320][100];

    ClassDef(TRGGDLDBFTDLBits, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2
