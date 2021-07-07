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
    TRGGDLDBFTDLBits(const TRGGDLDBFTDLBits& b): TObject(b)
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
    /** set name of i-th output trigger bit**/
    void setoutbitname(int i, const char* c)
    {
      strcpy(m_outbitname[i], c);
    }
    /** set the number of output trigger bits**/
    void setnoutbit(int i)
    {
      m_noutbit = i;
    }
    /** get name of i-th output trigger bit**/
    const char* getoutbitname(int i) const
    {
      return m_outbitname[i];
    }
    /** get the number of output trigger bits**/
    int getnoutbit() const
    {
      return m_noutbit;
    }
    /** get name of i-th input trigger bit**/
    int getoutbitnum(const char* c) const
    {
      for (int i = 0; i < 320; i++) {
        if (strcmp(c, m_outbitname[i]) == 0)return i;
      }
      return -1;
    }

  private:

    /** Number of bit */
    int m_noutbit;

    /** Name of bit */
    char m_outbitname[320][100];

    ClassDef(TRGGDLDBFTDLBits, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2
