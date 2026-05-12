/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  //! TRGTOPWaveFormTimeStamp
  class TRGTOPWaveFormTimeStamp : public RelationsObject {

  public:

    // Empty constructor
    // Recommended for ROOT IO
    TRGTOPWaveFormTimeStamp() :
      m_timeStamp(0)
    {}

    //! A Useful Constructor
    explicit TRGTOPWaveFormTimeStamp(
      int timeStamp
    ) :
      m_timeStamp(timeStamp)
    {}

    //! Another Useful Constructor
    TRGTOPWaveFormTimeStamp(
      int timeStamp,
      int slot
    ) :
      m_timeStamp(timeStamp),
      m_slot(slot)
    {}


    //! Destructor
    ~TRGTOPWaveFormTimeStamp() {}

    int getTimeStamp()      const { return m_timeStamp;     }
    int getSlot()      const { return m_slot;     }

    void setTimeStamp(int timeStamp);
    void setSlot(int slot);

    bool isEmptyClockCycle() const;
    bool isValid() const;

  private:

    //! timestamp value in 1/8 of FTSW clock cycle (conventional "2ns")
    int m_timeStamp;
    //! slot number (1 through 16)
    int m_slot;

    ClassDef(TRGTOPWaveFormTimeStamp, 15); /**< ClassDef */

  };

} //end namespace Belle2
