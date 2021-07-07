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
    TRGGDLDBInputBits(const TRGGDLDBInputBits& b): TObject(b)
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
    /** set name of i-th input trigger bit**/
    void setinbitname(int i, const char* c)
    {
      strcpy(m_inbitname[i], c);
    }
    /** set the number of input trigger bits**/
    void setninbit(int i)
    {
      m_ninbit = i;
    }
    /** get name of i-th input trigger bit**/
    const char* getinbitname(int i) const
    {
      return m_inbitname[i];
    }
    /** get the number of input trigger bits**/
    int getninbit() const
    {
      return m_ninbit;
    }
    /** get name of i-th input trigger bit**/
    int getinbitnum(const char* c) const
    {
      for (int i = 0; i < 320; i++) {
        if (strcmp(c, m_inbitname[i]) == 0)return i;
      }
      return -1;
    }

  private:

    /** Number of bit */
    int m_ninbit;

    /** Number of bins per sensor along u side */
    char m_inbitname[320][100];

    ClassDef(TRGGDLDBInputBits, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2
