/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRGTOPTIMINGISIMSLOT_H
#define TRGTOPTIMINGISIMSLOT_H

//#pragma once

//#include <TObject.h>
#include <framework/datastore/RelationsObject.h>

#include <vector>

#include <trg/top/dataobjects/TRGTOPTimingISim.h>

namespace Belle2 {


  //! Example Detector
  //  class TRGTOPTimingISimSlot : public TObject {
  class TRGTOPTimingISimSlot : public RelationsObject {

  public:

    // Empty constructor
    // Recommended for ROOT IO
    TRGTOPTimingISimSlot() :
      m_slotId(0)
    {}

    //! A Useful Constructor
    explicit TRGTOPTimingISimSlot(
      int slotId
    ) :
      m_slotId(slotId)
    {}

    //! Another Useful Constructor
    TRGTOPTimingISimSlot(
      int slotId,
      int numberOfTimeStamps
    ) :
      m_slotId(slotId),
      m_numberOfTimeStamps(numberOfTimeStamps)
    {}

    //! Destructor
    ~TRGTOPTimingISimSlot() {}

    int getSlotId() const { return m_slotId;}
    int getNumberOfTimeStamps() const { return m_numberOfTimeStamps;}
    //int get() const { return m_;}

    //Need namespace qualifier because ROOT CINT has troubles otherwise
    //    const std::vector<Belle2::TRGTOPTimingISim> getTimeStamps() { return m_timeStamps; }

    void setSlotId(int slotId);
    void setNumberOfTimeStamps(int numberOfTimeStamps);

    //void set(int );

    void addTimingISim(TRGTOPTimingISim timingISim);

  private:

    //! slot Id (1 through 16)
    int m_slotId;

    //! number of timestamps
    int m_numberOfTimeStamps;

    //! vector of ISim timings
    //    std::vector<TRGTOPTimingISim> m_timingISims; /**<  a vector of slot-level TOP timing decisions */

    /** the class title*/
    ClassDef(TRGTOPTimingISimSlot, 1);

  };

} //end namespace Belle2

#endif

