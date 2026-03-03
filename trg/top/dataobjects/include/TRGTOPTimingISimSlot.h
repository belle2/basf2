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
  class TRGTOPTimingISim;


  //! TRGTOPTimingISimSlot
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

    void setSlotId(int slotId);
    void setNumberOfTimeStamps(int numberOfTimeStamps);

    void addTimingISim(TRGTOPTimingISim timingISim);

  private:

    //! slot Id (1 through 16)
    int m_slotId;

    //! number of timestamps
    int m_numberOfTimeStamps;

    /** the class title*/
    ClassDef(TRGTOPTimingISimSlot, 1);

  };

} //end namespace Belle2
