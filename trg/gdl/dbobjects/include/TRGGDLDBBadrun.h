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
    TRGGDLDBBadrun(const TRGGDLDBBadrun& b)
    {
      m_flag = b.m_flag;
    }
    /** assignment operator */
    TRGGDLDBBadrun& operator=(const TRGGDLDBBadrun& b)
    {
      m_flag = b.m_flag;
      return *this;
    }

    void setflag(int i)
    {
      m_flag = i;
    }

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
