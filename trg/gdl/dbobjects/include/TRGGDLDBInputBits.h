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


  /** The payload class for GDL input bit
   *
   *  The payload class stores the number of input bit and their names
   *  value of a missing name is ""
   */

  class TRGGDLDBInputBits: public TObject {
  public:

    /** Default constructor */
    TRGGDLDBInputBits(): m_ninbit{0}, m_inbitname{} {}
    /** copy constructor */
    TRGGDLDBInputBits(const TRGGDLDBInputBits& b)
    {
      m_ninbit = b.m_ninbit;
      for (int i = 0; i < 320; i++) {
        strcpy(m_inbitname[i], b.m_inbitname[i]);
      }
    }
    /** assignment operator */
    TRGGDLDBInputBits& operator=(const TRGGDLDBInputBits& b)
    {
      m_ninbit = b.m_ninbit;
      for (int i = 0; i < 320; i++) {
        strcpy(m_inbitname[i], b.m_inbitname[i]);
      }
      return *this;
    }

    void setinbitname(int i, const char* c)
    {
      strcpy(m_inbitname[i], c);
    }
    void setninbit(int i)
    {
      m_ninbit = i;
    }

    const char* getinbitname(int i) const
    {
      return m_inbitname[i];
    }
    int getninbit() const
    {
      return m_ninbit;
    }

  private:

    /** Number of bit */
    int m_ninbit;

    /** Number of bins per sensor along u side */
    char m_inbitname[320][100];

    ClassDef(TRGGDLDBInputBits, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2
