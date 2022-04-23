/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRGTOPWAVEFORMTIMESTAMPSSLOT_H
#define TRGTOPWAVEFORMTIMESTAMPSSLOT_H

//#pragma once

//#include <TObject.h>
#include <framework/datastore/RelationsObject.h>

#include <vector>

#include <trg/top/dataobjects/TRGTOPWaveFormTimeStamp.h>

namespace Belle2 {


  //! Example Detector
  //  class TRGTOPWaveFormTimeStampsSlot : public TObject {
  class TRGTOPWaveFormTimeStampsSlot : public RelationsObject {

  public:

    // Empty constructor
    // Recommended for ROOT IO
    TRGTOPWaveFormTimeStampsSlot() :
      m_slotId(0)
    {}

    //! A Useful Constructor
    explicit TRGTOPWaveFormTimeStampsSlot(
      int slotId
    ) :
      m_slotId(slotId)
    {}

    //! Another Useful Constructor
    TRGTOPWaveFormTimeStampsSlot(
      int slotId,
      int numberOfTimeStamps
    ) :
      m_slotId(slotId),
      m_numberOfTimeStamps(numberOfTimeStamps)
    {}

    TRGTOPWaveFormTimeStampsSlot(
      int slotId,
      int numberOfTimeStamps,
      int numberOfActualTimeStamps
    ) :
      m_slotId(slotId),
      m_numberOfTimeStamps(numberOfTimeStamps),
      m_numberOfActualTimeStamps(numberOfActualTimeStamps)
    {}

    //! Destructor
    ~TRGTOPWaveFormTimeStampsSlot() {}

    int getSlotId() const { return m_slotId;}
    int getNumberOfTimeStamps() const { return m_numberOfTimeStamps;}
    int getNumberOfActualTimeStamps() const { return m_numberOfActualTimeStamps;}
    int getFirstActualTimeStampValue() const { return m_firstActualTimeStampValue;}
    int getFirstActualTimeStampClockCycle() const { return m_firstActualTimeStampClockCycle;}
    //int get() const { return m_;}

    //Need namespace qualifier because ROOT CINT has troubles otherwise
    //    const std::vector<Belle2::TRGTOPTimeStamp> getTimeStamps() { return m_timeStamps; }

    void setSlotId(int slotId);
    void setNumberOfTimeStamps(int numberOfTimeStamps);
    void setNumberOfActualTimeStamps(int numberOfActualTimeStamps);
    void setFirstActualTimeStampValue(int firstActualTimeStampValue);
    void setFirstActualTimeStampClockCycle(int firstActualTimeStampClockCycle);

    //void set(int );

    void addTimeStamp(TRGTOPWaveFormTimeStamp timeStamp);

  private:

    //! slot Id (1 through 16)
    int m_slotId;

    //! number of timestamps
    int m_numberOfTimeStamps;

    //! number of actual timestamps
    int m_numberOfActualTimeStamps;

    //! first actual timestamp's value
    int m_firstActualTimeStampValue;

    //! first actual timestamp's clock cycle
    int m_firstActualTimeStampClockCycle;

    //! vector of timestamps
    //    std::vector<TRGTOPTimeStamp> m_timeStamps; /**<  a vector of slot-level TOP timing decisions */

    /** the class title*/
    ClassDef(TRGTOPWaveFormTimeStampsSlot, 15);

  };

} //end namespace Belle2

#endif

