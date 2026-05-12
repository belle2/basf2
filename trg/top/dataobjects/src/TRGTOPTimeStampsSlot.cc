/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <trg/top/dataobjects/TRGTOPTimeStampsSlot.h>

using namespace Belle2;

void TRGTOPTimeStampsSlot::setSlotId(int slotId) { m_slotId = slotId; }
void TRGTOPTimeStampsSlot::setNumberOfTimeStamps(int numberOfTimeStamps) { m_numberOfTimeStamps = numberOfTimeStamps; }
