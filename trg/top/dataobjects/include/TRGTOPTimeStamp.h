/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRGTOPTIMESTAMP_H
#define TRGTOPTIMESTAMP_H

//#pragma once

//#include <TObject.h>
#include <framework/datastore/RelationsObject.h>

#include <vector>
#include <limits>

namespace Belle2 {

  //! Example Detector
  //  class TRGTOPTimeStamp : public TObject {
  class TRGTOPTimeStamp : public RelationsObject {

  public:

    // Empty constructor
    // Recommended for ROOT IO
    TRGTOPTimeStamp() :
      m_timeStamp(0)
    {}

    //! A Useful Constructor
    explicit TRGTOPTimeStamp(
      int timeStamp
    ) :
      m_timeStamp(timeStamp)
    {}

    //! Another Useful Constructor
    TRGTOPTimeStamp(
      int timeStamp,
      int slot
    ) :
      m_timeStamp(timeStamp),
      m_slot(slot)
    {}


    //! Destructor
    ~TRGTOPTimeStamp() {}

    int getTimeStamp()      const { return m_timeStamp;     }
    int getSlot()      const { return m_slot;     }
    //int get() const { return m_;}

    void setTimeStamp(int timeStamp);
    void setSlot(int slot);
    //void set(int );

  private:

    //! timestamp value in 1/8 of FTSW clock cycle (conventional "2ns")
    int m_timeStamp;
    //! slot number (1 through 16)
    int m_slot;

    ClassDef(TRGTOPTimeStamp, 15); /**< ClassDef */

  };

} //end namespace Belle2

#endif
