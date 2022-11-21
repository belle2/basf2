/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRGTOPTIMESTAMPSSLOT_H
#define TRGTOPTIMESTAMPSSLOT_H

//#pragma once

//#include <TObject.h>
#include <framework/datastore/RelationsObject.h>

#include <vector>

#include <trg/top/dataobjects/TRGTOPTimeStamp.h>

namespace Belle2 {


  //! Example Detector
  //  class TRGTOPTimeStampsSlot : public TObject {
  class TRGTOPTimeStampsSlot : public RelationsObject {

  public:

    // Empty constructor
    // Recommended for ROOT IO
    TRGTOPTimeStampsSlot() :
      m_slotId(0)
    {}

    //! A Useful Constructor
    explicit TRGTOPTimeStampsSlot(
      int slotId
    ) :
      m_slotId(slotId)
    {}

    //! Another Useful Constructor
    TRGTOPTimeStampsSlot(
      int slotId,
      int numberOfTimeStamps
    ) :
      m_slotId(slotId),
      m_numberOfTimeStamps(numberOfTimeStamps)
    {}

    //! Destructor
    ~TRGTOPTimeStampsSlot() {}

    int getSlotId() const { return m_slotId;}
    int getNumberOfTimeStamps() const { return m_numberOfTimeStamps;}
    //int get() const { return m_;}

    //Need namespace qualifier because ROOT CINT has troubles otherwise
    //    const std::vector<Belle2::TRGTOPTimeStamp> getTimeStamps() { return m_timeStamps; }

    void setSlotId(int slotId);
    void setNumberOfTimeStamps(int numberOfTimeStamps);

    //void set(int );

    void addTimeStamp(TRGTOPTimeStamp timeStamp);

  private:

    //! slot Id (1 through 16)
    int m_slotId;

    //! number of timestamps
    int m_numberOfTimeStamps;

    //! vector of timestamps
    //    std::vector<TRGTOPTimeStamp> m_timeStamps; /**<  a vector of slot-level TOP timing decisions */

    /** the class title*/
    ClassDef(TRGTOPTimeStampsSlot, 15);

  };

} //end namespace Belle2

#endif

