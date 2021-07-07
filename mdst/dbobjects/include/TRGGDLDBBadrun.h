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

  /** The payload class for GDL badrun
   *
   *  The payload class stores the flag of gdl status. 1 is fine and -1 is bad.
   *  value of a not confirmed status is "0"
   */


  class TRGGDLDBBadrun: public TObject {
  public:

    /** Default constructor */
    TRGGDLDBBadrun(): m_flag{0} {}
    /** copy constructor */
    TRGGDLDBBadrun(const TRGGDLDBBadrun& b): TObject(b)
    {
      m_flag = b.m_flag;
    }
    /** assignment operator */
    TRGGDLDBBadrun& operator=(const TRGGDLDBBadrun& b)
    {
      m_flag = b.m_flag;
      return *this;
    }
    /** set trigger GDL readout status 0:fine 1:problematic -1:not known**/
    void setflag(int i)
    {
      m_flag = i;
    }
    /** get trigger GDL readout status 0:fine 1:problematic -1:not known**/
    int getflag() const
    {
      return m_flag;
    }

  private:

    /** status flag */
    int m_flag;

    ClassDef(TRGGDLDBBadrun, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2
