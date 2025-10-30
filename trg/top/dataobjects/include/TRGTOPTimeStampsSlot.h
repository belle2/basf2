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
  class TRGTOPTimeStamp;


  //! TRGTOPTimeStampsSlot
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

    void setSlotId(int slotId);
    void setNumberOfTimeStamps(int numberOfTimeStamps);

    void addTimeStamp(TRGTOPTimeStamp timeStamp);

  private:

    //! slot Id (1 through 16)
    int m_slotId;

    //! number of timestamps
    int m_numberOfTimeStamps;


    /** the class title*/
    ClassDef(TRGTOPTimeStampsSlot, 15);

  };

} //end namespace Belle2
