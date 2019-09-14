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



  /** The payload class for delay of GDL input bit
   *
   *  The payload class stores the number of input bit and their delay
   *  value of a missing delay is "-1"
   */


  class CDCTrigger2DConfig: public TObject {
  public:

    /** Default constructor */
    CDCTrigger2DConfig(): m_nTS{0} {}
    /** copy constructor */
    CDCTrigger2DConfig(const CDCTrigger2DConfig& b): TObject(b)
    {
      m_nTS = b.m_nTS;
    }
    /** assignment operator */
    CDCTrigger2DConfig& operator=(const CDCTrigger2DConfig& b)
    {
      m_nTS = b.m_nTS;
      return *this;
    }

    /** Get the number of TS */
    int getnTS() const
    {
      return m_nTS;
    }
    /** Set the number of TS */
    void setnTS(int i)
    {
      m_nTS = i;
    }


  private:

    /** Number of TS */
    int m_nTS;

    ClassDef(CDCTrigger2DConfig, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2
